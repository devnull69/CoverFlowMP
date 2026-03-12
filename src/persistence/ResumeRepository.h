#pragma once

#include <QObject>
#include <QSqlDatabase>

class ResumeRepository : public QObject
{
    Q_OBJECT

public:
    explicit ResumeRepository(QObject *parent = nullptr);

    bool initialize();
    double loadPosition(const QString &filePath) const;
    double loadAudioDelay(const QString &filePath) const;
    bool savePosition(const QString &filePath,
                      double position,
                      double duration = 0.0,
                      double audioDelay = 0.0);
    bool deletePosition(const QString &filePath);
    bool clearAllPositions();

private:
    QSqlDatabase m_db;
};
