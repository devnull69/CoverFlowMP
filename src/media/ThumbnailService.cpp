#include "ThumbnailService.h"

#include <QCryptographicHash>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QStandardPaths>

ThumbnailService::ThumbnailService(QObject *parent)
    : QObject(parent)
{
}

QString ThumbnailService::cacheFilePathFor(const QString &videoPath) const
{
    const QString cacheDir =
        QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/thumbnails";
    QDir().mkpath(cacheDir);

    const QByteArray hash =
        QCryptographicHash::hash(videoPath.toUtf8(), QCryptographicHash::Md5).toHex();

    return cacheDir + "/" + hash + ".jpg";
}

QString ThumbnailService::ensureThumbnail(const QString &videoPath) const
{
    const QString outFile = cacheFilePathFor(videoPath);

    if (QFileInfo::exists(outFile))
        return outFile;

    QProcess proc;
    proc.start("ffmpegthumbnailer", {
                                        "-i", videoPath,
                                        "-o", outFile,
                                        "-s", "512",
                                        "-t", "5%"
                                    });
    proc.waitForFinished(10000);

    if (QFileInfo::exists(outFile))
        return outFile;

    QProcess ffmpegProc;
    ffmpegProc.start("ffmpeg", {
                                   "-y",
                                   "-ss", "00:00:05",
                                   "-i", videoPath,
                                   "-frames:v", "1",
                                   outFile
                               });
    ffmpegProc.waitForFinished(10000);

    if (QFileInfo::exists(outFile))
        return outFile;

    return "";
}
