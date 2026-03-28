#pragma once

#include <QObject>
#include <QJsonObject>
#include <QStringList>
#include <QSqlDatabase>
#include <QVector>

#include "../player/SkipRange.h"
#include "AppConfig.h"

class AppDatabase : public QObject
{
    Q_OBJECT

public:
    struct LegacyMergeResult {
        enum class Status {
            AlreadyChecked,
            NoLegacyDatabase,
            Merged,
            Failed
        };

        Status status = Status::AlreadyChecked;
        int mergedPlaybackRows = 0;
        int mergedSkipFiles = 0;
        QString legacyDatabasePath;
        QString errorMessage;
    };

    explicit AppDatabase(QObject *parent = nullptr);
    ~AppDatabase();

    bool initialize();
    LegacyMergeResult mergeLegacyDatabaseOnce();

    double loadPosition(const QString &filePath) const;
    double loadDuration(const QString &filePath) const;
    double loadAudioDelay(const QString &filePath) const;
    QVector<SkipRange> loadSkipRanges(const QString &filePath) const;
    double loadTotalSkipDuration(const QString &filePath) const;

    QString loadConfigString(const QString &key,
                             const QString &fallback = QString()) const;
    QJsonObject loadConfigObject(const QString &key,
                                 const QJsonObject &fallback = QJsonObject()) const;
    QStringList loadConfigStringList(const QString &key,
                                     const QStringList &fallback = QStringList()) const;

    bool savePosition(const QString &filePath,
                      double position,
                      double duration = 0.0,
                      double audioDelay = 0.0);
    bool saveSkipRanges(const QString &filePath, const QVector<SkipRange> &ranges);
    bool saveConfigObject(const QString &key, const QJsonObject &value);
    bool saveConfigString(const QString &key, const QString &value);
    bool saveConfigStringList(const QString &key, const QStringList &values);

    bool deletePosition(const QString &filePath);
    bool deleteSkipRanges(const QString &filePath);
    bool clearAllPlaybackState();
    bool clearFolderPlaybackState(const QString &folderPath);

private:
    struct ConfigRecord {
        QString value;
        QString valueType;
        bool found = false;
    };

    bool ensureDefaultConfiguration();
    bool migrateLegacyConfiguration();
    ConfigRecord loadConfigRecord(const QString &key) const;
    bool saveConfigValue(const QString &key,
                         const QString &value,
                         AppConfig::ValueType valueType);
    bool savePositionRecord(const QString &filePath,
                            double position,
                            double duration,
                            double audioDelay,
                            const QString &updatedAt);
    bool replaceSkipRangesRecords(const QString &filePath,
                                  const QVector<SkipRange> &ranges,
                                  const QVector<QString> &updatedAtValues,
                                  bool manageTransaction = true);
    bool markLegacyMergeChecked();
    static QString serializeJsonObject(const QJsonObject &values);
    static QString serializeStringList(const QStringList &values);

    QSqlDatabase m_db;
    QString m_connectionName;
};
