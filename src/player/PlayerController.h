#pragma once

#include <QObject>
#include <QString>

class MpvObject;

class PlayerController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool paused READ paused NOTIFY pausedChanged)
    Q_PROPERTY(double position READ position NOTIFY positionChanged)
    Q_PROPERTY(double duration READ duration NOTIFY durationChanged)

public:
    explicit PlayerController(QObject *parent = nullptr);

    bool paused() const;
    double position() const;
    double duration() const;

    Q_INVOKABLE void playFile(const QString &filePath, double startPosition = 0.0);
    Q_INVOKABLE void togglePause();
    Q_INVOKABLE void stop();

signals:
    void pausedChanged();
    void positionChanged();
    void durationChanged();

private:
    MpvObject *m_mpv;
    bool m_paused = false;
    double m_position = 0.0;
    double m_duration = 0.0;
};
