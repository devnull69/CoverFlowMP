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

void PlayerController::playFile(const QString &filePath, double startPosition)
{
    m_mpv->playFile(filePath, startPosition);
}

void PlayerController::togglePause()
{
    m_mpv->togglePause();
}

void PlayerController::stop()
{
    m_mpv->stop();
}

void PlayerController::attachToWindow(QObject *windowObject)
{
    auto *window = qobject_cast<QQuickWindow *>(windowObject);
    if (!window)
        return;

    m_mpv->setVideoWindow(static_cast<uintptr_t>(window->winId()));
}
