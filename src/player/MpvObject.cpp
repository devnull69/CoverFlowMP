#include "MpvObject.h"

MpvObject::MpvObject(QObject *parent)
    : QObject(parent)
{
}

void MpvObject::playFile(const QString &filePath, double startPosition)
{
    Q_UNUSED(filePath)
    m_position = startPosition;
    m_duration = 0.0;
    m_paused = false;

    emit positionChanged(m_position);
    emit durationChanged(m_duration);
    emit pausedChanged(m_paused);
}

void MpvObject::togglePause()
{
    m_paused = !m_paused;
    emit pausedChanged(m_paused);
}

void MpvObject::stop()
{
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
