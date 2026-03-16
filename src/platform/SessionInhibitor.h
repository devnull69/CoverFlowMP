#pragma once

#include <QObject>

class QDBusUnixFileDescriptor;

class SessionInhibitor : public QObject
{
    Q_OBJECT

public:
    explicit SessionInhibitor(QObject *parent = nullptr);
    ~SessionInhibitor() override;

    void setVideoPlaybackActive(bool active);
    bool isVideoPlaybackActive() const;

private:
    void acquireInhibitions();
    void releaseInhibitions();
    void acquireScreenSaverInhibit();
    void releaseScreenSaverInhibit();
    void acquireLogin1Inhibit();
    void releaseLogin1Inhibit();

    bool m_videoPlaybackActive = false;
    uint m_screenSaverCookie = 0;
    bool m_hasScreenSaverCookie = false;
    QDBusUnixFileDescriptor *m_login1Fd = nullptr;
};
