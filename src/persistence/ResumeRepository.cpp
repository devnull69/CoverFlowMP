#include "ResumeRepository.h"

#include <QDir>
#include <QStandardPaths>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>
#include <QDateTime>
#include <QDebug>

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
    return query.exec(
        "CREATE TABLE IF NOT EXISTS playback_state ("
        "file_path TEXT PRIMARY KEY,"
        "last_position REAL NOT NULL DEFAULT 0,"
        "duration REAL NOT NULL DEFAULT 0,"
        "updated_at TEXT NOT NULL)"
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

bool ResumeRepository::savePosition(const QString &filePath, double position, double duration)
{
    QSqlQuery query(m_db);
    query.prepare(
        "INSERT INTO playback_state(file_path, last_position, duration, updated_at) "
        "VALUES(?, ?, ?, ?) "
        "ON CONFLICT(file_path) DO UPDATE SET "
        "last_position = excluded.last_position, "
        "duration = excluded.duration, "
        "updated_at = excluded.updated_at"
        );

    query.addBindValue(filePath);
    query.addBindValue(position);
    query.addBindValue(duration);
    query.addBindValue(QDateTime::currentDateTimeUtc().toString(Qt::ISODate));

    return query.exec();
}

bool ResumeRepository::deletePosition(const QString &filePath)
{
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM playback_state WHERE file_path = ?");
    query.addBindValue(filePath);
    return query.exec();
}

bool ResumeRepository::clearAllPositions()
{
    QSqlQuery query(m_db);
    return query.exec("DELETE FROM playback_state");
}
