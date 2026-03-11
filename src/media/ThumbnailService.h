#pragma once

#include <QObject>
#include <QString>

class ThumbnailService : public QObject
{
    Q_OBJECT

public:
    explicit ThumbnailService(QObject *parent = nullptr);
    QString ensureThumbnail(const QString &videoPath) const;

private:
    QString cacheFilePathFor(const QString &videoPath) const;
};
