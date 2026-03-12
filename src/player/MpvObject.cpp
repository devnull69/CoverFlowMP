#include "MpvObject.h"

#include <QFileInfo>
#include <QtGlobal>

#include <mpv/client.h>

MpvObject::MpvObject(QObject *parent)
    : QObject(parent)
{
    m_eventTimer.setInterval(16);
    connect(&m_eventTimer, &QTimer::timeout, this, &MpvObject::processMpvEvents);
}

MpvObject::~MpvObject()
{
    m_eventTimer.stop();
    if (m_mpv) {
        mpv_terminate_destroy(m_mpv);
        m_mpv = nullptr;
    }
}

bool MpvObject::ensureInitialized()
{
    if (m_mpv)
        return true;

    m_mpv = mpv_create();
    if (!m_mpv)
        return false;

    mpv_set_option_string(m_mpv, "terminal", "no");
    mpv_set_option_string(m_mpv, "vo", "x11");
    mpv_set_option_string(m_mpv, "fullscreen", "no");
    mpv_set_option_string(m_mpv, "force-window", "no");
    mpv_set_option_string(m_mpv, "keep-open", "no");
    mpv_set_option_string(m_mpv, "input-default-bindings", "no");
    mpv_set_option_string(m_mpv, "input-vo-keyboard", "no");

    if (m_videoWindowId != 0) {
        qint64 wid = static_cast<qint64>(m_videoWindowId);
        mpv_set_option(m_mpv, "wid", MPV_FORMAT_INT64, &wid);
    }

    if (mpv_initialize(m_mpv) < 0) {
        mpv_terminate_destroy(m_mpv);
        m_mpv = nullptr;
        return false;
    }

    mpv_observe_property(m_mpv, 0, "pause", MPV_FORMAT_FLAG);
    mpv_observe_property(m_mpv, 0, "time-pos", MPV_FORMAT_DOUBLE);
    mpv_observe_property(m_mpv, 0, "duration", MPV_FORMAT_DOUBLE);
    mpv_observe_property(m_mpv, 0, "audio-delay", MPV_FORMAT_DOUBLE);

    m_eventTimer.start();
    return true;
}

void MpvObject::processMpvEvents()
{
    if (!m_mpv)
        return;

    while (true) {
        mpv_event *event = mpv_wait_event(m_mpv, 0.0);
        if (!event || event->event_id == MPV_EVENT_NONE)
            break;

        if (event->event_id == MPV_EVENT_PROPERTY_CHANGE) {
            auto *prop = static_cast<mpv_event_property *>(event->data);
            if (!prop || !prop->name)
                continue;

            if (qstrcmp(prop->name, "pause") == 0 && prop->format == MPV_FORMAT_FLAG && prop->data) {
                const bool paused = *static_cast<int *>(prop->data) != 0;
                if (m_paused != paused) {
                    m_paused = paused;
                    emit pausedChanged(m_paused);
                }
            } else if (qstrcmp(prop->name, "time-pos") == 0 && prop->format == MPV_FORMAT_DOUBLE && prop->data) {
                const double position = *static_cast<double *>(prop->data);
                if (!qFuzzyCompare(m_position + 1.0, position + 1.0)) {
                    m_position = position;
                    emit positionChanged(m_position);
                }
            } else if (qstrcmp(prop->name, "duration") == 0 && prop->format == MPV_FORMAT_DOUBLE && prop->data) {
                const double duration = *static_cast<double *>(prop->data);
                if (!qFuzzyCompare(m_duration + 1.0, duration + 1.0)) {
                    m_duration = duration;
                    emit durationChanged(m_duration);
                }
            } else if (qstrcmp(prop->name, "audio-delay") == 0 &&
                       prop->format == MPV_FORMAT_DOUBLE && prop->data) {
                const double audioDelay = *static_cast<double *>(prop->data);
                if (!qFuzzyCompare(m_audioDelay + 1.0, audioDelay + 1.0)) {
                    m_audioDelay = audioDelay;
                    emit audioDelayChanged(m_audioDelay);
                }
            }
        } else if (event->event_id == MPV_EVENT_FILE_LOADED) {
            if (m_hasDeferredSeekAfterLoad && m_deferredSeekAfterLoad > 0.0) {
                const QByteArray seekPos = QByteArray::number(m_deferredSeekAfterLoad, 'f', 3);
                const char *seekArgs[] = { "seek", seekPos.constData(), "absolute+exact", nullptr };
                if (mpv_command(m_mpv, seekArgs) >= 0) {
                    if (!qFuzzyCompare(m_position + 1.0, m_deferredSeekAfterLoad + 1.0)) {
                        m_position = m_deferredSeekAfterLoad;
                        emit positionChanged(m_position);
                    }
                }
            }
            m_hasDeferredSeekAfterLoad = false;
            m_deferredSeekAfterLoad = 0.0;
        } else if (event->event_id == MPV_EVENT_END_FILE) {
            if (m_position != 0.0) {
                m_position = 0.0;
                emit positionChanged(m_position);
            }
            if (m_paused) {
                m_paused = false;
                emit pausedChanged(m_paused);
            }
        }
    }
}

void MpvObject::emitStateSnapshot()
{
    emit positionChanged(m_position);
    emit durationChanged(m_duration);
    emit pausedChanged(m_paused);
    emit audioDelayChanged(m_audioDelay);
}

void MpvObject::setVideoWindow(uintptr_t wid)
{
    m_videoWindowId = wid;

    if (m_mpv && m_videoWindowId != 0) {
        qint64 windowId = static_cast<qint64>(m_videoWindowId);
        mpv_set_property(m_mpv, "wid", MPV_FORMAT_INT64, &windowId);
    }

    if (m_hasPendingPlayback) {
        playFile(m_pendingFilePath, m_pendingStartPosition, m_audioDelay);
        m_hasPendingPlayback = false;
        m_pendingFilePath.clear();
        m_pendingStartPosition = 0.0;
    }
}

void MpvObject::playFile(const QString &filePath, double startPosition, double audioDelay)
{
    if (filePath.isEmpty())
        return;

    if (m_videoWindowId == 0) {
        m_pendingFilePath = filePath;
        m_pendingStartPosition = startPosition;
        m_audioDelay = audioDelay;
        m_hasPendingPlayback = true;
        return;
    }

    if (!ensureInitialized())
        return;

    setAudioDelay(audioDelay);

    const QByteArray utf8File = QFileInfo(filePath).absoluteFilePath().toUtf8();
    const char *loadArgs[] = { "loadfile", utf8File.constData(), "replace", nullptr };
    if (mpv_command(m_mpv, loadArgs) < 0)
        return;

    m_hasDeferredSeekAfterLoad = startPosition > 0.0;
    m_deferredSeekAfterLoad = startPosition > 0.0 ? startPosition : 0.0;

    int unpause = 0;
    mpv_set_property(m_mpv, "pause", MPV_FORMAT_FLAG, &unpause);

    m_position = 0.0;
    m_duration = 0.0;
    m_paused = false;
    emitStateSnapshot();
}

void MpvObject::togglePause()
{
    if (!m_mpv)
        return;

    m_paused = !m_paused;
    int pausedFlag = m_paused ? 1 : 0;
    mpv_set_property(m_mpv, "pause", MPV_FORMAT_FLAG, &pausedFlag);
    emit pausedChanged(m_paused);
}

void MpvObject::setAudioDelay(double seconds)
{
    const double clampedSeconds = qBound(-2.0, seconds, 2.0);

    if (!ensureInitialized())
        return;

    if (!qFuzzyCompare(m_audioDelay + 1.0, clampedSeconds + 1.0)) {
        m_audioDelay = clampedSeconds;
        emit audioDelayChanged(m_audioDelay);
    }

    mpv_set_property(m_mpv, "audio-delay", MPV_FORMAT_DOUBLE, &m_audioDelay);
}

void MpvObject::seekRelative(double seconds)
{
    if (!m_mpv)
        return;

    double target = m_position + seconds;
    if (target < 0.0)
        target = 0.0;
    if (m_duration > 0.0 && target > m_duration)
        target = m_duration;

    const QByteArray seekPos = QByteArray::number(target, 'f', 3);
    const char *seekArgs[] = { "seek", seekPos.constData(), "absolute+exact", nullptr };
    if (mpv_command(m_mpv, seekArgs) < 0)
        return;

    if (!qFuzzyCompare(m_position + 1.0, target + 1.0)) {
        m_position = target;
        emit positionChanged(m_position);
    }
}

void MpvObject::stop()
{
    m_eventTimer.stop();

    if (m_mpv) {
        const char *cmd[] = { "stop", nullptr };
        mpv_command(m_mpv, cmd);
        mpv_terminate_destroy(m_mpv);
        m_mpv = nullptr;
    }

    m_hasPendingPlayback = false;
    m_pendingFilePath.clear();
    m_pendingStartPosition = 0.0;
    m_hasDeferredSeekAfterLoad = false;
    m_deferredSeekAfterLoad = 0.0;

    m_position = 0.0;
    m_duration = 0.0;
    m_paused = false;
    m_audioDelay = 0.0;
    emitStateSnapshot();
}

bool MpvObject::paused() const
{
    return m_paused;
}

double MpvObject::position() const
{
    return m_position;
}

double MpvObject::duration() const
{
    return m_duration;
}

double MpvObject::audioDelay() const
{
    return m_audioDelay;
}
