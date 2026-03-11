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
    bool savePosition(const QString &filePath, double position, double duration = 0.0);

private:
    QSqlDatabase m_db;
};
