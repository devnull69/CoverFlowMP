#include "PlayerController.h"
#include "MpvObject.h"

#include <QQuickWindow>
#include <QtGlobal>
#include <algorithm>

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
        if (m_skipJumpInProgress) {
            m_skipJumpInProgress = false;
            return;
        }

        maybeSkipCurrentPosition();
    });

    connect(m_mpv, &MpvObject::durationChanged, this, [this](double duration) {
        m_duration = duration;
        emit durationChanged();
    });

    connect(m_mpv, &MpvObject::audioDelayChanged, this, [this](double audioDelay) {
        m_audioDelay = audioDelay;
        emit audioDelayChanged();
    });

    connect(m_mpv, &MpvObject::playbackFinished, this, [this]() {
        m_playbackFinishedPending = false;
        emit playbackFinished();
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

QVariantList PlayerController::skipRanges() const
{
    QVariantList ranges;
    ranges.reserve(m_skipRanges.size());

    for (const SkipRange &range : m_skipRanges) {
        QVariantMap item;
        item.insert(QStringLiteral("start"), range.start);
        item.insert(QStringLiteral("end"), range.end);
        ranges.append(item);
    }

    return ranges;
}

bool PlayerController::skipRangePending() const
{
    return m_skipRangePending;
}

double PlayerController::pendingSkipStart() const
{
    return m_pendingSkipStart;
}

QVector<SkipRange> PlayerController::skipRangesData() const
{
    return m_skipRanges;
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
    if (!m_mpv)
        return;

    if (qFuzzyIsNull(seconds))
        return;

    const SeekDirection direction = seconds > 0.0
        ? SeekDirection::Forward
        : SeekDirection::Backward;

    const double target = resolveSeekTarget(m_position + seconds, direction);
    if (m_duration > 0.0 && target >= m_duration) {
        requestPlaybackFinished();
        return;
    }

    m_skipJumpInProgress = true;
    m_mpv->seekAbsolute(target);
}

void PlayerController::stepFrameForward()
{
    if (!m_paused)
        return;

    m_mpv->frameStep();
}

void PlayerController::stepFrameBackward()
{
    if (!m_paused)
        return;

    m_mpv->frameBackStep();
}

void PlayerController::markSkipBoundary()
{
    if (!m_paused)
        return;

    if (!m_skipRangePending) {
        m_skipRangePending = true;
        m_pendingSkipStart = qMax(0.0, m_position);
        emitSkipRangeState();
        return;
    }

    finalizeSkipRange(m_position);
}

void PlayerController::clearPendingSkipRange()
{
    if (!m_skipRangePending)
        return;

    m_skipRangePending = false;
    m_pendingSkipStart = 0.0;
    emitSkipRangeState();
}

void PlayerController::stop()
{
    m_skipJumpInProgress = false;
    m_playbackFinishedPending = false;
    clearPendingSkipRange();
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

void PlayerController::setSkipRanges(const QVector<SkipRange> &ranges)
{
    const QVector<SkipRange> normalized = normalizedSkipRanges(ranges);
    if (m_skipRanges == normalized)
        return;

    m_skipRanges = normalized;
    emit skipRangesChanged();
}

void PlayerController::maybeSkipCurrentPosition()
{
    if (m_paused || m_skipRanges.isEmpty())
        return;

    for (const SkipRange &range : m_skipRanges) {
        if (m_position >= range.start && m_position < range.end) {
            if (m_duration > 0.0 && range.end >= m_duration) {
                requestPlaybackFinished();
                return;
            }

            m_skipJumpInProgress = true;
            m_mpv->seekAbsolute(range.end);
            return;
        }
    }
}

void PlayerController::emitSkipRangeState()
{
    emit skipRangePendingChanged();
    emit pendingSkipStartChanged();
}

void PlayerController::finalizeSkipRange(double position)
{
    const double start = qMin(m_pendingSkipStart, position);
    const double end = qMax(m_pendingSkipStart, position);
    m_skipRangePending = false;
    m_pendingSkipStart = 0.0;
    emitSkipRangeState();

    if (end - start <= 0.001)
        return;

    QVector<SkipRange> updated = m_skipRanges;
    updated.append({ start, end });
    const QVector<SkipRange> normalized = normalizedSkipRanges(updated);
    if (m_skipRanges == normalized)
        return;

    m_skipRanges = normalized;
    emit skipRangesChanged();
}

void PlayerController::requestPlaybackFinished()
{
    if (m_playbackFinishedPending)
        return;

    m_playbackFinishedPending = true;
    emit playbackFinished();
}

QVector<SkipRange> PlayerController::normalizedSkipRanges(const QVector<SkipRange> &ranges) const
{
    QVector<SkipRange> normalized;
    normalized.reserve(ranges.size());

    for (const SkipRange &range : ranges) {
        const double start = qMin(range.start, range.end);
        const double end = qMax(range.start, range.end);
        if (end - start <= 0.001)
            continue;

        SkipRange clampedRange { qMax(0.0, start), end };
        if (m_duration > 0.0)
            clampedRange.end = qMin(clampedRange.end, m_duration);

        if (clampedRange.end - clampedRange.start <= 0.001)
            continue;

        normalized.append(clampedRange);
    }

    std::sort(normalized.begin(), normalized.end(), [](const SkipRange &lhs, const SkipRange &rhs) {
        if (lhs.start == rhs.start)
            return lhs.end < rhs.end;
        return lhs.start < rhs.start;
    });

    QVector<SkipRange> merged;
    for (const SkipRange &range : normalized) {
        if (merged.isEmpty()) {
            merged.append(range);
            continue;
        }

        SkipRange &last = merged.last();
        if (range.start <= last.end + 0.001) {
            last.end = qMax(last.end, range.end);
            continue;
        }

        merged.append(range);
    }

    return merged;
}

double PlayerController::resolveSeekTarget(double target, SeekDirection direction) const
{
    double resolved = qMax(0.0, target);
    if (m_duration > 0.0)
        resolved = qMin(resolved, m_duration);

    bool changed = true;
    while (changed) {
        changed = false;

        for (const SkipRange &range : m_skipRanges) {
            if (direction == SeekDirection::Forward) {
                if (resolved >= range.start && resolved < range.end) {
                    resolved = range.end;
                    changed = true;
                    break;
                }
            } else {
                if (resolved > range.start && resolved <= range.end) {
                    resolved = qMax(0.0, range.start - 0.01);
                    changed = true;
                    break;
                }
            }
        }
    }

    return resolved;
}
