#pragma once

#include <QObject>
#include <QString>

class MpvObject : public QObject
{
    Q_OBJECT

public:
    explicit MpvObject(QObject *parent = nullptr);

    void playFile(const QString &filePath, double startPosition);
    void togglePause();
    void stop();

    bool paused() const;
    double position() const;
    double duration() const;

signals:
    void pausedChanged(bool paused);
    void positionChanged(double position);
    void durationChanged(double duration);

private:
    bool m_paused = false;
    double m_position = 0.0;
    double m_duration = 0.0;
};
