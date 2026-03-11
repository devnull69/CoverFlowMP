#include "LibraryScanner.h"
#include "ThumbnailService.h"

#include <QDir>
#include <QFileInfo>
#include <QFileInfoList>

LibraryScanner::LibraryScanner(ThumbnailService *thumbnailService, QObject *parent)
    : QObject(parent),
    m_thumbnailService(thumbnailService)
{
}

bool LibraryScanner::isVideoFile(const QString &suffix) const
{
    const QString s = suffix.toLower();
    return s == "mp4" || s == "mkv" || s == "avi";
}

QVector<VideoItem> LibraryScanner::createDemoItems() const
{
    QVector<VideoItem> items;

    for (int i = 1; i <= 10; ++i) {
        VideoItem item;
        item.title = QString("Beispielcover %1").arg(i);
        item.filePath = "";
        item.coverPath = "";
        item.isDemo = true;
        items.push_back(item);
    }

    return items;
}

QVector<VideoItem> LibraryScanner::scan(const QString &folderPath) const
{
    QVector<VideoItem> items;

    QDir dir(folderPath);
    if (!dir.exists())
        return createDemoItems();

    const QFileInfoList files = dir.entryInfoList(
        QDir::Files | QDir::NoDotAndDotDot,
        QDir::Name
        );

    for (const QFileInfo &fi : files) {
        if (!isVideoFile(fi.suffix()))
            continue;

        VideoItem item;
        item.filePath = fi.absoluteFilePath();
        item.title = fi.completeBaseName();

        const QString coverPath = fi.absolutePath() + "/" + fi.completeBaseName() + ".jpg";
        if (QFileInfo::exists(coverPath)) {
            item.coverPath = coverPath;
        } else {
            item.coverPath = m_thumbnailService->ensureThumbnail(fi.absoluteFilePath());
        }

        item.isDemo = false;
        items.push_back(item);
    }

    if (items.isEmpty())
        return createDemoItems();

    return items;
}
