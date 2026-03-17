#include "ResumeRepository.h"

#include <QDir>
#include <QStandardPaths>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>
#include <QDateTime>
#include <QDebug>
#include <QDir>

ResumeRepository::ResumeRepository(QObject *parent)
    : QObject(parent)
{
}

bool ResumeRepository::initialize()
{
    const QString appDataDir =
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(appDataDir);

    const QString dbPath = appDataDir + "/playback.sqlite";

    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(dbPath);

    if (!m_db.open()) {
        qWarning() << "Could not open SQLite DB:" << m_db.lastError().text();
        return false;
    }

    QSqlQuery query(m_db);
    const bool created = query.exec(
        "CREATE TABLE IF NOT EXISTS playback_state ("
        "file_path TEXT PRIMARY KEY,"
        "last_position REAL NOT NULL DEFAULT 0,"
        "duration REAL NOT NULL DEFAULT 0,"
        "audio_delay REAL NOT NULL DEFAULT 0,"
        "updated_at TEXT NOT NULL)"
        );

    if (!created)
        return false;

    QSqlQuery columnsQuery(m_db);
    if (!columnsQuery.exec("PRAGMA table_info(playback_state)"))
        return false;

    bool hasAudioDelay = false;
    while (columnsQuery.next()) {
        if (columnsQuery.value(1).toString() == "audio_delay") {
            hasAudioDelay = true;
            break;
        }
    }

    if (hasAudioDelay)
        ;
    else if (!query.exec("ALTER TABLE playback_state ADD COLUMN audio_delay REAL NOT NULL DEFAULT 0"))
        return false;

    QSqlQuery skipQuery(m_db);
    return skipQuery.exec(
        "CREATE TABLE IF NOT EXISTS skip_ranges ("
        "file_path TEXT NOT NULL,"
        "start_pos REAL NOT NULL,"
        "end_pos REAL NOT NULL,"
        "updated_at TEXT NOT NULL,"
        "PRIMARY KEY(file_path, start_pos, end_pos))"
    );
}

double ResumeRepository::loadPosition(const QString &filePath) const
{
    QSqlQuery query(m_db);
    query.prepare("SELECT last_position FROM playback_state WHERE file_path = ?");
    query.addBindValue(filePath);

    if (query.exec() && query.next())
        return query.value(0).toDouble();

    return 0.0;
}

double ResumeRepository::loadDuration(const QString &filePath) const
{
    QSqlQuery query(m_db);
    query.prepare("SELECT duration FROM playback_state WHERE file_path = ?");
    query.addBindValue(filePath);

    if (query.exec() && query.next())
        return query.value(0).toDouble();

    return 0.0;
}

double ResumeRepository::loadAudioDelay(const QString &filePath) const
{
    QSqlQuery query(m_db);
    query.prepare("SELECT audio_delay FROM playback_state WHERE file_path = ?");
    query.addBindValue(filePath);

    if (query.exec() && query.next())
        return query.value(0).toDouble();

    return 0.0;
}

QVector<SkipRange> ResumeRepository::loadSkipRanges(const QString &filePath) const
{
    QVector<SkipRange> ranges;

    QSqlQuery query(m_db);
    query.prepare(
        "SELECT start_pos, end_pos FROM skip_ranges "
        "WHERE file_path = ? ORDER BY start_pos ASC"
    );
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

double ResumeRepository::loadTotalSkipDuration(const QString &filePath) const
{
    QSqlQuery query(m_db);
    query.prepare(
        "SELECT COALESCE(SUM(MAX(0, end_pos - start_pos)), 0) "
        "FROM skip_ranges WHERE file_path = ?"
    );
    query.addBindValue(filePath);

    if (query.exec() && query.next())
        return query.value(0).toDouble();

    return 0.0;
}

bool ResumeRepository::savePosition(const QString &filePath,
                                    double position,
                                    double duration,
                                    double audioDelay)
{
    QSqlQuery query(m_db);
    query.prepare(
        "INSERT INTO playback_state(file_path, last_position, duration, audio_delay, updated_at) "
        "VALUES(?, ?, ?, ?, ?) "
        "ON CONFLICT(file_path) DO UPDATE SET "
        "last_position = excluded.last_position, "
        "duration = excluded.duration, "
        "audio_delay = excluded.audio_delay, "
        "updated_at = excluded.updated_at"
        );

    query.addBindValue(filePath);
    query.addBindValue(position);
    query.addBindValue(duration);
    query.addBindValue(audioDelay);
    query.addBindValue(QDateTime::currentDateTimeUtc().toString(Qt::ISODate));

    return query.exec();
}

bool ResumeRepository::saveSkipRanges(const QString &filePath, const QVector<SkipRange> &ranges)
{
    if (!m_db.transaction())
        return false;

    QSqlQuery deleteQuery(m_db);
    deleteQuery.prepare("DELETE FROM skip_ranges WHERE file_path = ?");
    deleteQuery.addBindValue(filePath);
    if (!deleteQuery.exec()) {
        m_db.rollback();
        return false;
    }

    if (!ranges.isEmpty()) {
        QSqlQuery insertQuery(m_db);
        insertQuery.prepare(
            "INSERT INTO skip_ranges(file_path, start_pos, end_pos, updated_at) "
            "VALUES(?, ?, ?, ?)"
        );

        const QString updatedAt = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
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

bool ResumeRepository::deletePosition(const QString &filePath)
{
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM playback_state WHERE file_path = ?");
    query.addBindValue(filePath);
    return query.exec();
}

bool ResumeRepository::deleteSkipRanges(const QString &filePath)
{
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM skip_ranges WHERE file_path = ?");
    query.addBindValue(filePath);
    return query.exec();
}

bool ResumeRepository::clearAllPositions()
{
    QSqlQuery query(m_db);
    if (!query.exec("DELETE FROM playback_state"))
        return false;

    return query.exec("DELETE FROM skip_ranges");
}

bool ResumeRepository::clearFolderEntries(const QString &folderPath)
{
    if (folderPath.isEmpty())
        return false;

    const QString normalizedFolder = QDir::cleanPath(folderPath);
    QString prefix = normalizedFolder;
    if (!prefix.endsWith('/'))
        prefix += '/';
    const QString pattern = prefix + "%";
    const int prefixLength = prefix.size();

    if (!m_db.transaction())
        return false;

    QSqlQuery playbackQuery(m_db);
    playbackQuery.prepare(
        "DELETE FROM playback_state "
        "WHERE file_path LIKE ? "
        "AND INSTR(SUBSTR(file_path, ?), '/') = 0"
    );
    playbackQuery.addBindValue(pattern);
    playbackQuery.addBindValue(prefixLength + 1);
    if (!playbackQuery.exec()) {
        m_db.rollback();
        return false;
    }

    QSqlQuery skipQuery(m_db);
    skipQuery.prepare(
        "DELETE FROM skip_ranges "
        "WHERE file_path LIKE ? "
        "AND INSTR(SUBSTR(file_path, ?), '/') = 0"
    );
    skipQuery.addBindValue(pattern);
    skipQuery.addBindValue(prefixLength + 1);
    if (!skipQuery.exec()) {
        m_db.rollback();
        return false;
    }

    return m_db.commit();
}
