#pragma once

#include <QObject>
#include <QString>
#include <QTimer>
#include <cstdint>
struct mpv_handle;

class MpvObject : public QObject
{
    Q_OBJECT

public:
    explicit MpvObject(QObject *parent = nullptr);
    ~MpvObject() override;

    void playFile(const QString &filePath, double startPosition);
    void togglePause();
    void seekRelative(double seconds);
    void stop();
    void setVideoWindow(uintptr_t wid);

    bool paused() const;
    double position() const;
    double duration() const;

signals:
    void pausedChanged(bool paused);
    void positionChanged(double position);
    void durationChanged(double duration);

private:
    bool ensureInitialized();
    void processMpvEvents();
    void emitStateSnapshot();

    mpv_handle *m_mpv = nullptr;
    QTimer m_eventTimer;
    uintptr_t m_videoWindowId = 0;
    QString m_pendingFilePath;
    double m_pendingStartPosition = 0.0;
    bool m_hasPendingPlayback = false;
    bool m_hasDeferredSeekAfterLoad = false;
    double m_deferredSeekAfterLoad = 0.0;
    bool m_paused = false;
    double m_position = 0.0;
    double m_duration = 0.0;
};
