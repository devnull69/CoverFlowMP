#include "AppDatabase.h"

#include "AppPaths.h"

#include <QDateTime>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonValue>
#include <QSqlError>
#include <QSqlQuery>
#include <QUuid>

namespace {

QString currentUtcTimestamp()
{
    return QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
}

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
    query.addBindValue(currentUtcTimestamp());
    return query.exec();
}

bool AppDatabase::saveSkipRanges(const QString &filePath, const QVector<SkipRange> &ranges)
{
    if (!m_db.transaction())
        return false;

    QSqlQuery deleteQuery(m_db);
    deleteQuery.prepare(QStringLiteral("DELETE FROM skip_ranges WHERE file_path = ?"));
    deleteQuery.addBindValue(filePath);
    if (!deleteQuery.exec()) {
        m_db.rollback();
        return false;
    }

    if (!ranges.isEmpty()) {
        QSqlQuery insertQuery(m_db);
        insertQuery.prepare(
            QStringLiteral("INSERT INTO skip_ranges(file_path, start_pos, end_pos, updated_at) "
                           "VALUES(?, ?, ?, ?)"));

        const QString updatedAt = currentUtcTimestamp();
        for (const SkipRange &range : ranges) {
            insertQuery.addBindValue(filePath);
            insertQuery.addBindValue(range.start);
            insertQuery.addBindValue(range.end);
            insertQuery.addBindValue(updatedAt);
            if (!insertQuery.exec()) {
                m_db.rollback();
                return false;
            }
        }
    }

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
