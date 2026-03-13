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

void LibraryScanner::setRootFolder(const QString &rootFolder)
{
    m_rootFolder = QDir(rootFolder).absolutePath();
}

ThumbnailService *LibraryScanner::thumbnailService() const
{
    return m_thumbnailService;
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

    const QString currentPath = dir.absolutePath();
    const QString parentPath = QFileInfo(currentPath).dir().absolutePath();
    const bool atRootLevel = !m_rootFolder.isEmpty() && currentPath == m_rootFolder;
    if (parentPath != currentPath && !atRootLevel) {
        VideoItem parentFolder;
        parentFolder.title = "..";
        parentFolder.filePath = parentPath;
        parentFolder.coverPath = "";
        parentFolder.isFolder = true;
        parentFolder.isParentFolder = true;
        items.push_back(parentFolder);
    }

    const QFileInfoList folders = dir.entryInfoList(
        QDir::Dirs | QDir::NoDotAndDotDot,
        QDir::Name
        );

    for (const QFileInfo &fi : folders) {
        VideoItem item;
        item.title = fi.fileName();
        item.filePath = fi.absoluteFilePath();
        const QString folderCoverPath = fi.absoluteFilePath() + "/folder.jpg";
        item.coverPath = QFileInfo::exists(folderCoverPath) ? folderCoverPath : "";
        item.isFolder = true;
        item.isParentFolder = false;
        item.isDemo = false;
        items.push_back(item);
    }

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
            item.coverPath = m_thumbnailService->existingThumbnail(fi.absoluteFilePath());
        }

        item.isDemo = false;
        item.isFolder = false;
        item.isParentFolder = false;
        items.push_back(item);
    }

    if (items.isEmpty())
        return createDemoItems();

    return items;
}
