#include "ThumbnailService.h"

#include <QCryptographicHash>
#include <QDir>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QProcess>
#include <QStandardPaths>

#include <mpv/client.h>

namespace {

double probeDurationWithMpv(const QString &videoPath)
{
    mpv_handle *mpv = mpv_create();
    if (!mpv)
        return 0.0;

    mpv_set_option_string(mpv, "terminal", "no");
    mpv_set_option_string(mpv, "msg-level", "all=no");
    mpv_set_option_string(mpv, "vo", "null");
    mpv_set_option_string(mpv, "ao", "null");
    mpv_set_option_string(mpv, "pause", "yes");
    mpv_set_option_string(mpv, "keep-open", "no");
    mpv_set_option_string(mpv, "idle", "no");

    if (mpv_initialize(mpv) < 0) {
        mpv_terminate_destroy(mpv);
        return 0.0;
    }

    mpv_observe_property(mpv, 0, "duration", MPV_FORMAT_DOUBLE);

    const QByteArray utf8File = QFileInfo(videoPath).absoluteFilePath().toUtf8();
    const char *loadArgs[] = { "loadfile", utf8File.constData(), "replace", nullptr };
    if (mpv_command(mpv, loadArgs) < 0) {
        mpv_terminate_destroy(mpv);
        return 0.0;
    }

    QElapsedTimer timer;
    timer.start();

    while (timer.elapsed() < 10000) {
        mpv_event *event = mpv_wait_event(mpv, 0.1);
        if (!event || event->event_id == MPV_EVENT_NONE)
            continue;

        if (event->event_id == MPV_EVENT_PROPERTY_CHANGE) {
            auto *prop = static_cast<mpv_event_property *>(event->data);
            if (!prop || !prop->name || qstrcmp(prop->name, "duration") != 0)
                continue;

            if (prop->format == MPV_FORMAT_DOUBLE && prop->data) {
                const double duration = *static_cast<double *>(prop->data);
                if (duration > 0.0) {
                    mpv_terminate_destroy(mpv);
                    return duration;
                }
            }
        } else if (event->event_id == MPV_EVENT_FILE_LOADED) {
            double duration = 0.0;
            if (mpv_get_property(mpv, "duration", MPV_FORMAT_DOUBLE, &duration) >= 0
                && duration > 0.0) {
                mpv_terminate_destroy(mpv);
                return duration;
            }
        } else if (event->event_id == MPV_EVENT_END_FILE) {
            break;
        }
    }

    mpv_terminate_destroy(mpv);
    return 0.0;
}

}

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

QString ThumbnailService::existingThumbnail(const QString &videoPath) const
{
    const QString outFile = cacheFilePathFor(videoPath);
    return QFileInfo::exists(outFile) ? outFile : "";
}

QString ThumbnailService::ensureThumbnail(const QString &videoPath) const
{
    const QString outFile = existingThumbnail(videoPath);
    if (!outFile.isEmpty())
        return outFile;

    const QString generatedOutFile = cacheFilePathFor(videoPath);

    QProcess proc;
    proc.start("ffmpegthumbnailer", {
                                        "-i", videoPath,
                                        "-o", generatedOutFile,
                                        "-s", "512",
                                        "-t", "5%"
                                    });
    proc.waitForFinished(10000);

    if (QFileInfo::exists(generatedOutFile))
        return generatedOutFile;

    QProcess ffmpegProc;
    ffmpegProc.start("ffmpeg", {
                                   "-y",
                                   "-ss", "00:00:05",
                                   "-i", videoPath,
                                   "-frames:v", "1",
                                   generatedOutFile
                               });
    ffmpegProc.waitForFinished(10000);

    if (QFileInfo::exists(generatedOutFile))
        return generatedOutFile;

    return "";
}

double ThumbnailService::probeDuration(const QString &videoPath) const
{
    const QString ffprobePath = QStandardPaths::findExecutable("ffprobe");
    if (!ffprobePath.isEmpty()) {
        QProcess proc;
        proc.start(ffprobePath, {
                                 "-v", "error",
                                 "-show_entries", "format=duration",
                                 "-of", "default=noprint_wrappers=1:nokey=1",
                                 videoPath
                             });

        if (proc.waitForStarted(2000) && proc.waitForFinished(10000)) {
            const QString output = QString::fromUtf8(proc.readAllStandardOutput()).trimmed();
            bool ok = false;
            const double duration = output.toDouble(&ok);
            if (ok && duration > 0.0)
                return duration;
        }
    }

    return probeDurationWithMpv(videoPath);
}
