#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>
#include <QVector>

#include "SkipRange.h"

class MpvObject;

class PlayerController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool paused READ paused NOTIFY pausedChanged)
    Q_PROPERTY(double position READ position NOTIFY positionChanged)
    Q_PROPERTY(double duration READ duration NOTIFY durationChanged)
    Q_PROPERTY(double audioDelay READ audioDelay NOTIFY audioDelayChanged)
    Q_PROPERTY(QVariantList skipRanges READ skipRanges NOTIFY skipRangesChanged)
    Q_PROPERTY(bool skipRangePending READ skipRangePending NOTIFY skipRangePendingChanged)
    Q_PROPERTY(double pendingSkipStart READ pendingSkipStart NOTIFY pendingSkipStartChanged)
    Q_PROPERTY(bool skipHandlingEnabled READ skipHandlingEnabled NOTIFY skipHandlingEnabledChanged)

public:
    explicit PlayerController(QObject *parent = nullptr);

    bool paused() const;
    double position() const;
    double duration() const;
    double audioDelay() const;
    QVariantList skipRanges() const;
    bool skipRangePending() const;
    double pendingSkipStart() const;
    bool skipHandlingEnabled() const;
    QVector<SkipRange> skipRangesData() const;

    Q_INVOKABLE void playFile(const QString &filePath, double startPosition = 0.0, double audioDelay = 0.0);
    Q_INVOKABLE void togglePause();
    Q_INVOKABLE void seekRelative(double seconds);
    Q_INVOKABLE void seekRelativeClamped(double seconds);
    Q_INVOKABLE void stepFrameForward();
    Q_INVOKABLE void stepFrameBackward();
    Q_INVOKABLE void jumpToLastFrame();
    Q_INVOKABLE void markSkipBoundary();
    Q_INVOKABLE void clearPendingSkipRange();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void attachToWindow(QObject *windowObject);
    Q_INVOKABLE void setAudioDelay(double seconds);
    void setSkipRanges(const QVector<SkipRange> &ranges);
    void setSkipHandlingEnabled(bool enabled);

signals:
    void pausedChanged();
    void positionChanged();
    void durationChanged();
    void audioDelayChanged();
    void playbackFinished();
    void skipRangesChanged();
    void skipRangePendingChanged();
    void pendingSkipStartChanged();
    void skipHandlingEnabledChanged();

private:
    enum class SeekDirection {
        Backward,
        Forward
    };

    void maybeSkipCurrentPosition();
    void emitSkipRangeState();
    void finalizeSkipRange(double position);
    void requestPlaybackFinished();
    void seekRelativeInternal(double seconds, bool finishAtEnd);
    QVector<SkipRange> normalizedSkipRanges(const QVector<SkipRange> &ranges) const;
    double resolveSeekTarget(double target, SeekDirection direction) const;

    MpvObject *m_mpv;
    bool m_paused = false;
    double m_position = 0.0;
    double m_duration = 0.0;
    double m_audioDelay = 0.0;
    QVector<SkipRange> m_skipRanges;
    bool m_skipRangePending = false;
    double m_pendingSkipStart = 0.0;
    bool m_skipHandlingEnabled = true;
    bool m_skipJumpInProgress = false;
    bool m_resumeSkipActivationPending = false;
    double m_resumeSkipActivationTarget = 0.0;
    bool m_playbackFinishedPending = false;
};
