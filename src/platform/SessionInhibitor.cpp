#include "SessionInhibitor.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusUnixFileDescriptor>

SessionInhibitor::SessionInhibitor(QObject *parent)
    : QObject(parent),
      m_login1Fd(new QDBusUnixFileDescriptor())
{
}

SessionInhibitor::~SessionInhibitor()
{
    releaseInhibitions();
    delete m_login1Fd;
    m_login1Fd = nullptr;
}

void SessionInhibitor::setVideoPlaybackActive(bool active)
{
    if (m_videoPlaybackActive == active)
        return;

    m_videoPlaybackActive = active;
    if (m_videoPlaybackActive)
        acquireInhibitions();
    else
        releaseInhibitions();
}

bool SessionInhibitor::isVideoPlaybackActive() const
{
    return m_videoPlaybackActive;
}

void SessionInhibitor::acquireInhibitions()
{
    acquireScreenSaverInhibit();
    acquireLogin1Inhibit();
}

void SessionInhibitor::releaseInhibitions()
{
    releaseScreenSaverInhibit();
    releaseLogin1Inhibit();
}

void SessionInhibitor::acquireScreenSaverInhibit()
{
    if (m_hasScreenSaverCookie)
        return;

    auto bus = QDBusConnection::sessionBus();
    if (!bus.isConnected())
        return;

    QDBusInterface interface(QStringLiteral("org.freedesktop.ScreenSaver"),
                             QStringLiteral("/ScreenSaver"),
                             QStringLiteral("org.freedesktop.ScreenSaver"),
                             bus);
    if (!interface.isValid())
        return;

    QDBusReply<uint> reply = interface.call(QStringLiteral("Inhibit"),
                                            QStringLiteral("CoverFlowMP"),
                                            QStringLiteral("Video playback active"));
    if (!reply.isValid())
        return;

    m_screenSaverCookie = reply.value();
    m_hasScreenSaverCookie = true;
}

void SessionInhibitor::releaseScreenSaverInhibit()
{
    if (!m_hasScreenSaverCookie)
        return;

    auto bus = QDBusConnection::sessionBus();
    if (bus.isConnected()) {
        QDBusInterface interface(QStringLiteral("org.freedesktop.ScreenSaver"),
                                 QStringLiteral("/ScreenSaver"),
                                 QStringLiteral("org.freedesktop.ScreenSaver"),
                                 bus);
        if (interface.isValid())
            interface.call(QStringLiteral("UnInhibit"), m_screenSaverCookie);
    }

    m_screenSaverCookie = 0;
    m_hasScreenSaverCookie = false;
}

void SessionInhibitor::acquireLogin1Inhibit()
{
    if (m_login1Fd && m_login1Fd->isValid())
        return;

    auto bus = QDBusConnection::systemBus();
    if (!bus.isConnected())
        return;

    QDBusInterface interface(QStringLiteral("org.freedesktop.login1"),
                             QStringLiteral("/org/freedesktop/login1"),
                             QStringLiteral("org.freedesktop.login1.Manager"),
                             bus);
    if (!interface.isValid())
        return;

    QDBusReply<QDBusUnixFileDescriptor> reply = interface.call(
        QStringLiteral("Inhibit"),
        QStringLiteral("idle"),
        QStringLiteral("CoverFlowMP"),
        QStringLiteral("Video playback active"),
        QStringLiteral("block"));
    if (!reply.isValid())
        return;

    *m_login1Fd = reply.value();
}

void SessionInhibitor::releaseLogin1Inhibit()
{
    if (!m_login1Fd || !m_login1Fd->isValid())
        return;

    *m_login1Fd = QDBusUnixFileDescriptor();
}
