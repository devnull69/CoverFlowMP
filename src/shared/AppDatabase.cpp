#include "AppDatabase.h"

#include "AppPaths.h"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonValue>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlDriver>
#include <QStandardPaths>
#include <QUuid>

namespace {

QString currentUtcTimestamp()
{
    return QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
}

QString normalizedTimestampForComparison(const QString &timestamp)
{
    const QDateTime parsed = QDateTime::fromString(timestamp, Qt::ISODate);
    if (parsed.isValid())
        return parsed.toUTC().toString(Qt::ISODate);

    return timestamp;
}

bool shouldImportLegacyTimestamp(const QString &existingTimestamp,
                                 const QString &legacyTimestamp)
{
    if (existingTimestamp.isEmpty())
        return true;
    if (legacyTimestamp.isEmpty())
        return false;

    return normalizedTimestampForComparison(legacyTimestamp)
        >= normalizedTimestampForComparison(existingTimestamp);
}

QString legacyDatabasePath()
{
    const QString legacyDirectory =
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (legacyDirectory.isEmpty())
        return {};

    return legacyDirectory + QStringLiteral("/playback.sqlite");
}

struct LegacySkipRecord {
    SkipRange range;
    QString updatedAt;
};

struct LegacySkipFileData {
    QVector<LegacySkipRecord> records;
    QString latestUpdatedAt;
};

}

AppDatabase::AppDatabase(QObject *parent)
    : QObject(parent)
{
}

AppDatabase::~AppDatabase()
{
    if (m_db.isValid()) {
        const QString connectionName = m_connectionName;
        m_db.close();
        m_db = QSqlDatabase();
        QSqlDatabase::removeDatabase(connectionName);
    }
}

bool AppDatabase::initialize()
{
    if (m_db.isOpen())
        return true;

    AppPaths::sharedDataDirectory();

    m_connectionName = QUuid::createUuid().toString(QUuid::WithoutBraces);
    m_db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), m_connectionName);
    m_db.setDatabaseName(AppPaths::databasePath());

    if (!m_db.open())
        return false;

    QSqlQuery query(m_db);
    if (!query.exec(
            "CREATE TABLE IF NOT EXISTS playback_state ("
            "file_path TEXT PRIMARY KEY,"
            "last_position REAL NOT NULL DEFAULT 0,"
            "duration REAL NOT NULL DEFAULT 0,"
            "audio_delay REAL NOT NULL DEFAULT 0,"
            "updated_at TEXT NOT NULL)")) {
        return false;
    }

    QSqlQuery columnsQuery(m_db);
    if (!columnsQuery.exec(QStringLiteral("PRAGMA table_info(playback_state)")))
        return false;

    bool hasAudioDelay = false;
    while (columnsQuery.next()) {
        if (columnsQuery.value(1).toString() == QStringLiteral("audio_delay")) {
            hasAudioDelay = true;
            break;
        }
    }

    if (!hasAudioDelay
        && !query.exec(QStringLiteral("ALTER TABLE playback_state ADD COLUMN audio_delay REAL NOT NULL DEFAULT 0"))) {
        return false;
    }

    if (!query.exec(
            "CREATE TABLE IF NOT EXISTS skip_ranges ("
            "file_path TEXT NOT NULL,"
            "start_pos REAL NOT NULL,"
            "end_pos REAL NOT NULL,"
            "updated_at TEXT NOT NULL,"
            "PRIMARY KEY(file_path, start_pos, end_pos))")) {
        return false;
    }

    if (!query.exec(
            "CREATE TABLE IF NOT EXISTS app_config ("
            "config_key TEXT PRIMARY KEY,"
            "config_value TEXT NOT NULL,"
            "value_type TEXT NOT NULL,"
            "updated_at TEXT NOT NULL)")) {
        return false;
    }

    return ensureDefaultConfiguration();
}

AppDatabase::LegacyMergeResult AppDatabase::mergeLegacyDatabaseOnce()
{
    LegacyMergeResult result;

    if (loadConfigString(AppConfig::legacyPlaybackMergeCheckedKey()) == QStringLiteral("true"))
        return result;

    result.legacyDatabasePath = legacyDatabasePath();
    if (result.legacyDatabasePath.isEmpty()
        || QDir::cleanPath(result.legacyDatabasePath) == QDir::cleanPath(AppPaths::databasePath())
        || !QFileInfo::exists(result.legacyDatabasePath)) {
        if (!markLegacyMergeChecked()) {
            result.status = LegacyMergeResult::Status::Failed;
            result.errorMessage =
                QStringLiteral("Der Migrationsstatus konnte nicht gespeichert werden.");
            return result;
        }

        result.status = LegacyMergeResult::Status::NoLegacyDatabase;
        return result;
    }

    const QString legacyConnectionName = QUuid::createUuid().toString(QUuid::WithoutBraces);
    QSqlDatabase legacyDb = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), legacyConnectionName);
    legacyDb.setConnectOptions(QStringLiteral("QSQLITE_OPEN_READONLY"));
    legacyDb.setDatabaseName(result.legacyDatabasePath);

    auto finalizeFailure = [this, &result](const QString &message) {
        result.status = LegacyMergeResult::Status::Failed;
        result.errorMessage = message;
        markLegacyMergeChecked();
    };

    if (!legacyDb.open()) {
        finalizeFailure(legacyDb.lastError().text());
        legacyDb = QSqlDatabase();
        QSqlDatabase::removeDatabase(legacyConnectionName);
        return result;
    }

    if (!m_db.transaction()) {
        finalizeFailure(m_db.lastError().text());
        legacyDb.close();
        legacyDb = QSqlDatabase();
        QSqlDatabase::removeDatabase(legacyConnectionName);
        return result;
    }

    QSqlQuery legacyPlaybackQuery(legacyDb);
    if (!legacyPlaybackQuery.exec(
            QStringLiteral("SELECT file_path, last_position, duration, audio_delay, updated_at "
                           "FROM playback_state"))) {
        m_db.rollback();
        finalizeFailure(legacyPlaybackQuery.lastError().text());
        legacyDb.close();
        legacyDb = QSqlDatabase();
        QSqlDatabase::removeDatabase(legacyConnectionName);
        return result;
    }

    QSqlQuery existingPlaybackQuery(m_db);
    existingPlaybackQuery.prepare(
        QStringLiteral("SELECT updated_at FROM playback_state WHERE file_path = ?"));

    while (legacyPlaybackQuery.next()) {
        const QString filePath = legacyPlaybackQuery.value(0).toString();
        const double position = legacyPlaybackQuery.value(1).toDouble();
        const double duration = legacyPlaybackQuery.value(2).toDouble();
        const double audioDelay = legacyPlaybackQuery.value(3).toDouble();
        const QString updatedAt = legacyPlaybackQuery.value(4).toString();

        existingPlaybackQuery.bindValue(0, filePath);
        QString existingUpdatedAt;
        if (!existingPlaybackQuery.exec()) {
            m_db.rollback();
            finalizeFailure(existingPlaybackQuery.lastError().text());
            legacyDb.close();
            legacyDb = QSqlDatabase();
            QSqlDatabase::removeDatabase(legacyConnectionName);
            return result;
        }

        if (existingPlaybackQuery.next())
            existingUpdatedAt = existingPlaybackQuery.value(0).toString();
        existingPlaybackQuery.finish();

        if (!shouldImportLegacyTimestamp(existingUpdatedAt, updatedAt))
            continue;

        if (!savePositionRecord(filePath,
                                position,
                                duration,
                                audioDelay,
                                updatedAt.isEmpty() ? currentUtcTimestamp() : updatedAt)) {
            m_db.rollback();
            finalizeFailure(m_db.lastError().text());
            legacyDb.close();
            legacyDb = QSqlDatabase();
            QSqlDatabase::removeDatabase(legacyConnectionName);
            return result;
        }

        ++result.mergedPlaybackRows;
    }

    QSqlQuery legacySkipQuery(legacyDb);
    if (!legacySkipQuery.exec(
            QStringLiteral("SELECT file_path, start_pos, end_pos, updated_at "
                           "FROM skip_ranges ORDER BY file_path ASC, start_pos ASC"))) {
        m_db.rollback();
        finalizeFailure(legacySkipQuery.lastError().text());
        legacyDb.close();
        legacyDb = QSqlDatabase();
        QSqlDatabase::removeDatabase(legacyConnectionName);
        return result;
    }

    QHash<QString, LegacySkipFileData> legacySkipFiles;
    while (legacySkipQuery.next()) {
        const QString filePath = legacySkipQuery.value(0).toString();
        const double start = legacySkipQuery.value(1).toDouble();
        const double end = legacySkipQuery.value(2).toDouble();
        const QString updatedAt = legacySkipQuery.value(3).toString();

        if (filePath.isEmpty() || end <= start)
            continue;

        LegacySkipFileData &fileData = legacySkipFiles[filePath];
        fileData.records.append({ { start, end }, updatedAt });
        if (fileData.latestUpdatedAt.isEmpty()
            || shouldImportLegacyTimestamp(fileData.latestUpdatedAt, updatedAt)) {
            fileData.latestUpdatedAt = updatedAt;
        }
    }

    QSqlQuery existingSkipQuery(m_db);
    existingSkipQuery.prepare(
        QStringLiteral("SELECT MAX(updated_at) FROM skip_ranges WHERE file_path = ?"));

    for (auto it = legacySkipFiles.cbegin(); it != legacySkipFiles.cend(); ++it) {
        const QString &filePath = it.key();
        const LegacySkipFileData &fileData = it.value();

        existingSkipQuery.bindValue(0, filePath);
        QString existingUpdatedAt;
        if (!existingSkipQuery.exec()) {
            m_db.rollback();
            finalizeFailure(existingSkipQuery.lastError().text());
            legacyDb.close();
            legacyDb = QSqlDatabase();
            QSqlDatabase::removeDatabase(legacyConnectionName);
            return result;
        }

        if (existingSkipQuery.next())
            existingUpdatedAt = existingSkipQuery.value(0).toString();
        existingSkipQuery.finish();

        if (!shouldImportLegacyTimestamp(existingUpdatedAt, fileData.latestUpdatedAt))
            continue;

        QVector<SkipRange> ranges;
        QVector<QString> updatedAtValues;
        ranges.reserve(fileData.records.size());
        updatedAtValues.reserve(fileData.records.size());
        for (const LegacySkipRecord &record : fileData.records) {
            ranges.append(record.range);
            updatedAtValues.append(record.updatedAt.isEmpty()
                                       ? currentUtcTimestamp()
                                       : record.updatedAt);
        }

        if (!replaceSkipRangesRecords(filePath, ranges, updatedAtValues, false)) {
            m_db.rollback();
            finalizeFailure(m_db.lastError().text());
            legacyDb.close();
            legacyDb = QSqlDatabase();
            QSqlDatabase::removeDatabase(legacyConnectionName);
            return result;
        }

        ++result.mergedSkipFiles;
    }

    if (!markLegacyMergeChecked()) {
        m_db.rollback();
        finalizeFailure(QStringLiteral("Der Migrationsstatus konnte nicht gespeichert werden."));
        legacyDb.close();
        legacyDb = QSqlDatabase();
        QSqlDatabase::removeDatabase(legacyConnectionName);
        return result;
    }

    if (!m_db.commit()) {
        m_db.rollback();
        finalizeFailure(m_db.lastError().text());
        legacyDb.close();
        legacyDb = QSqlDatabase();
        QSqlDatabase::removeDatabase(legacyConnectionName);
        return result;
    }

    legacyDb.close();
    legacyDb = QSqlDatabase();
    QSqlDatabase::removeDatabase(legacyConnectionName);

    result.status = LegacyMergeResult::Status::Merged;
    return result;
}

bool AppDatabase::ensureDefaultConfiguration()
{
    if (!loadConfigRecord(AppConfig::browserBackgroundKey()).found
        && !saveConfigString(AppConfig::browserBackgroundKey(),
                             AppConfig::defaultBrowserBackground())) {
        return false;
    }

    if (!loadConfigRecord(AppConfig::libraryFoldersKey()).found
        && !saveConfigStringList(AppConfig::libraryFoldersKey(),
                                 AppConfig::defaultLibraryFolders())) {
        return false;
    }

    return true;
}

double AppDatabase::loadPosition(const QString &filePath) const
{
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral("SELECT last_position FROM playback_state WHERE file_path = ?"));
    query.addBindValue(filePath);

    if (query.exec() && query.next())
        return query.value(0).toDouble();

    return 0.0;
}

double AppDatabase::loadDuration(const QString &filePath) const
{
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral("SELECT duration FROM playback_state WHERE file_path = ?"));
    query.addBindValue(filePath);

    if (query.exec() && query.next())
        return query.value(0).toDouble();

    return 0.0;
}

double AppDatabase::loadAudioDelay(const QString &filePath) const
{
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral("SELECT audio_delay FROM playback_state WHERE file_path = ?"));
    query.addBindValue(filePath);

    if (query.exec() && query.next())
        return query.value(0).toDouble();

    return 0.0;
}

QVector<SkipRange> AppDatabase::loadSkipRanges(const QString &filePath) const
{
    QVector<SkipRange> ranges;

    QSqlQuery query(m_db);
    query.prepare(
        QStringLiteral("SELECT start_pos, end_pos FROM skip_ranges "
                       "WHERE file_path = ? ORDER BY start_pos ASC"));
    query.addBindValue(filePath);

    if (!query.exec())
        return ranges;

    while (query.next()) {
        const double start = query.value(0).toDouble();
        const double end = query.value(1).toDouble();
        if (end > start)
            ranges.append({ start, end });
    }

    return ranges;
}

double AppDatabase::loadTotalSkipDuration(const QString &filePath) const
{
    QSqlQuery query(m_db);
    query.prepare(
        QStringLiteral("SELECT COALESCE(SUM(MAX(0, end_pos - start_pos)), 0) "
                       "FROM skip_ranges WHERE file_path = ?"));
    query.addBindValue(filePath);

    if (query.exec() && query.next())
        return query.value(0).toDouble();

    return 0.0;
}

AppDatabase::ConfigRecord AppDatabase::loadConfigRecord(const QString &key) const
{
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral("SELECT config_value, value_type FROM app_config WHERE config_key = ?"));
    query.addBindValue(key);

    if (query.exec() && query.next()) {
        return {
            query.value(0).toString(),
            query.value(1).toString(),
            true
        };
    }

    return {};
}

QString AppDatabase::loadConfigString(const QString &key, const QString &fallback) const
{
    const ConfigRecord record = loadConfigRecord(key);
    if (!record.found)
        return fallback;

    if (record.valueType != AppConfig::valueTypeName(AppConfig::ValueType::String))
        return fallback;

    return record.value;
}

QStringList AppDatabase::loadConfigStringList(const QString &key, const QStringList &fallback) const
{
    const ConfigRecord record = loadConfigRecord(key);
    if (!record.found)
        return fallback;

    if (record.valueType != AppConfig::valueTypeName(AppConfig::ValueType::Json))
        return fallback;

    const QJsonDocument document = QJsonDocument::fromJson(record.value.toUtf8());
    if (!document.isArray())
        return fallback;

    QStringList values;
    const QJsonArray array = document.array();
    values.reserve(array.size());
    for (const QJsonValue &entry : array) {
        if (entry.isString())
            values.append(entry.toString());
    }

    return values.isEmpty() ? fallback : values;
}

bool AppDatabase::savePosition(const QString &filePath,
                               double position,
                               double duration,
                               double audioDelay)
{
    return savePositionRecord(filePath,
                              position,
                              duration,
                              audioDelay,
                              currentUtcTimestamp());
}

bool AppDatabase::savePositionRecord(const QString &filePath,
                                     double position,
                                     double duration,
                                     double audioDelay,
                                     const QString &updatedAt)
{
    QSqlQuery query(m_db);
    query.prepare(
        QStringLiteral("INSERT INTO playback_state(file_path, last_position, duration, audio_delay, updated_at) "
                       "VALUES(?, ?, ?, ?, ?) "
                       "ON CONFLICT(file_path) DO UPDATE SET "
                       "last_position = excluded.last_position, "
                       "duration = excluded.duration, "
                       "audio_delay = excluded.audio_delay, "
                       "updated_at = excluded.updated_at"));
    query.addBindValue(filePath);
    query.addBindValue(position);
    query.addBindValue(duration);
    query.addBindValue(audioDelay);
    query.addBindValue(updatedAt);
    return query.exec();
}

bool AppDatabase::saveSkipRanges(const QString &filePath, const QVector<SkipRange> &ranges)
{
    QVector<QString> updatedAtValues;
    updatedAtValues.reserve(ranges.size());
    const QString updatedAt = currentUtcTimestamp();
    for (int i = 0; i < ranges.size(); ++i)
        updatedAtValues.append(updatedAt);

    return replaceSkipRangesRecords(filePath, ranges, updatedAtValues);
}

bool AppDatabase::replaceSkipRangesRecords(const QString &filePath,
                                           const QVector<SkipRange> &ranges,
                                           const QVector<QString> &updatedAtValues,
                                           bool manageTransaction)
{
    if (ranges.size() != updatedAtValues.size())
        return false;

    if (manageTransaction && !m_db.transaction())
        return false;

    QSqlQuery deleteQuery(m_db);
    deleteQuery.prepare(QStringLiteral("DELETE FROM skip_ranges WHERE file_path = ?"));
    deleteQuery.addBindValue(filePath);
    if (!deleteQuery.exec()) {
        if (manageTransaction)
            m_db.rollback();
        return false;
    }

    if (!ranges.isEmpty()) {
        QSqlQuery insertQuery(m_db);
        insertQuery.prepare(
            QStringLiteral("INSERT INTO skip_ranges(file_path, start_pos, end_pos, updated_at) "
                           "VALUES(?, ?, ?, ?)"));

        for (int i = 0; i < ranges.size(); ++i) {
            const SkipRange &range = ranges.at(i);
            insertQuery.addBindValue(filePath);
            insertQuery.addBindValue(range.start);
            insertQuery.addBindValue(range.end);
            insertQuery.addBindValue(updatedAtValues.at(i));
            if (!insertQuery.exec()) {
                if (manageTransaction)
                    m_db.rollback();
                return false;
            }
        }
    }

    if (!manageTransaction)
        return true;

    return m_db.commit();
}

bool AppDatabase::saveConfigString(const QString &key, const QString &value)
{
    return saveConfigValue(key, value, AppConfig::ValueType::String);
}

bool AppDatabase::saveConfigStringList(const QString &key, const QStringList &values)
{
    return saveConfigValue(key, serializeStringList(values), AppConfig::ValueType::Json);
}

bool AppDatabase::saveConfigValue(const QString &key,
                                  const QString &value,
                                  AppConfig::ValueType valueType)
{
    QSqlQuery query(m_db);
    query.prepare(
        QStringLiteral("INSERT INTO app_config(config_key, config_value, value_type, updated_at) "
                       "VALUES(?, ?, ?, ?) "
                       "ON CONFLICT(config_key) DO UPDATE SET "
                       "config_value = excluded.config_value, "
                       "value_type = excluded.value_type, "
                       "updated_at = excluded.updated_at"));
    query.addBindValue(key);
    query.addBindValue(value);
    query.addBindValue(AppConfig::valueTypeName(valueType));
    query.addBindValue(currentUtcTimestamp());
    return query.exec();
}

bool AppDatabase::markLegacyMergeChecked()
{
    return saveConfigString(AppConfig::legacyPlaybackMergeCheckedKey(),
                            QStringLiteral("true"));
}

QString AppDatabase::serializeStringList(const QStringList &values)
{
    QJsonArray array;
    for (const QString &value : values)
        array.append(value);

    return QString::fromUtf8(QJsonDocument(array).toJson(QJsonDocument::Compact));
}

bool AppDatabase::deletePosition(const QString &filePath)
{
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral("DELETE FROM playback_state WHERE file_path = ?"));
    query.addBindValue(filePath);
    return query.exec();
}

bool AppDatabase::deleteSkipRanges(const QString &filePath)
{
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral("DELETE FROM skip_ranges WHERE file_path = ?"));
    query.addBindValue(filePath);
    return query.exec();
}

bool AppDatabase::clearAllPlaybackState()
{
    QSqlQuery query(m_db);
    if (!query.exec(QStringLiteral("DELETE FROM playback_state")))
        return false;

    return query.exec(QStringLiteral("DELETE FROM skip_ranges"));
}

bool AppDatabase::clearFolderPlaybackState(const QString &folderPath)
{
    if (folderPath.isEmpty())
        return false;

    const QString normalizedFolder = QDir::cleanPath(folderPath);
    QString prefix = normalizedFolder;
    if (!prefix.endsWith(QLatin1Char('/')))
        prefix += QLatin1Char('/');
    const QString pattern = prefix + QStringLiteral("%");
    const int prefixLength = prefix.size();

    if (!m_db.transaction())
        return false;

    QSqlQuery playbackQuery(m_db);
    playbackQuery.prepare(
        QStringLiteral("DELETE FROM playback_state "
                       "WHERE file_path LIKE ? "
                       "AND INSTR(SUBSTR(file_path, ?), '/') = 0"));
    playbackQuery.addBindValue(pattern);
    playbackQuery.addBindValue(prefixLength + 1);
    if (!playbackQuery.exec()) {
        m_db.rollback();
        return false;
    }

    QSqlQuery skipQuery(m_db);
    skipQuery.prepare(
        QStringLiteral("DELETE FROM skip_ranges "
                       "WHERE file_path LIKE ? "
                       "AND INSTR(SUBSTR(file_path, ?), '/') = 0"));
    skipQuery.addBindValue(pattern);
    skipQuery.addBindValue(prefixLength + 1);
    if (!skipQuery.exec()) {
        m_db.rollback();
        return false;
    }

    return m_db.commit();
}
