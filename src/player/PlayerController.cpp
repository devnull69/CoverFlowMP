#include "PlayerController.h"
#include "MpvObject.h"

#include <QQuickWindow>

PlayerController::PlayerController(QObject *parent)
    : QObject(parent),
    m_mpv(new MpvObject(this))
{
    connect(m_mpv, &MpvObject::pausedChanged, this, [this](bool paused) {
        m_paused = paused;
        emit pausedChanged();
    });

    connect(m_mpv, &MpvObject::positionChanged, this, [this](double position) {
        m_position = position;
        emit positionChanged();
    });

    connect(m_mpv, &MpvObject::durationChanged, this, [this](double duration) {
        m_duration = duration;
        emit durationChanged();
    });

    connect(m_mpv, &MpvObject::audioDelayChanged, this, [this](double audioDelay) {
        m_audioDelay = audioDelay;
        emit audioDelayChanged();
    });
}

bool PlayerController::paused() const
{
    return m_paused;
}

double PlayerController::position() const
{
    return m_position;
}

double PlayerController::duration() const
{
    return m_duration;
}

double PlayerController::audioDelay() const
{
    return m_audioDelay;
}

void PlayerController::playFile(const QString &filePath, double startPosition, double audioDelay)
{
    m_mpv->playFile(filePath, startPosition, audioDelay);
}

void PlayerController::togglePause()
{
    m_mpv->togglePause();
}

void PlayerController::seekRelative(double seconds)
{
    m_mpv->seekRelative(seconds);
}

void PlayerController::stop()
{
    m_mpv->stop();
}

void PlayerController::setAudioDelay(double seconds)
{
    m_mpv->setAudioDelay(seconds);
}

void PlayerController::attachToWindow(QObject *windowObject)
{
    auto *window = qobject_cast<QQuickWindow *>(windowObject);
    if (!window)
        return;

    m_mpv->setVideoWindow(static_cast<uintptr_t>(window->winId()));
}
