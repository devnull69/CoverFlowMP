#pragma once

#include <QObject>
#include <QVector>
#include "VideoItem.h"

class ThumbnailService;

class LibraryScanner : public QObject
{
    Q_OBJECT

public:
    explicit LibraryScanner(ThumbnailService *thumbnailService, QObject *parent = nullptr);
    void setRootFolder(const QString &rootFolder);
    QVector<VideoItem> scan(const QString &folderPath) const;
    ThumbnailService *thumbnailService() const;

private:
    bool isVideoFile(const QString &suffix) const;
    QVector<VideoItem> createDemoItems() const;

    ThumbnailService *m_thumbnailService;
    QString m_rootFolder;
};
