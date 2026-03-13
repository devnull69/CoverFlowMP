#pragma once

#include <QObject>
#include <QString>
#include <QThreadPool>
#include <QVector>
#include "../media/VideoItem.h"

class VideoLibraryModel;
class LibraryScanner;
class PlayerController;
class ResumeRepository;
class ThumbnailService;

class AppController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool playerVisible READ playerVisible NOTIFY playerVisibleChanged)
    Q_PROPERTY(int currentIndex READ currentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(QString currentVideoName READ currentVideoName NOTIFY currentVideoNameChanged)
    Q_PROPERTY(bool resumePromptVisible READ resumePromptVisible NOTIFY resumePromptVisibleChanged)
    Q_PROPERTY(double pendingResumePosition READ pendingResumePosition NOTIFY pendingResumePositionChanged)
    Q_PROPERTY(QString playerMessage READ playerMessage NOTIFY playerMessageChanged)
    Q_PROPERTY(bool fastMode READ fastMode NOTIFY fastModeChanged)

public:
    explicit AppController(VideoLibraryModel *libraryModel,
                           LibraryScanner *scanner,
                           PlayerController *playerController,
                           ResumeRepository *resumeRepository,
                           QObject *parent = nullptr);

    bool playerVisible() const;
    int currentIndex() const;
    QString currentVideoName() const;
    bool resumePromptVisible() const;
    double pendingResumePosition() const;
    QString playerMessage() const;
    bool fastMode() const;

    Q_INVOKABLE void initialize(const QString &videoFolder);
    Q_INVOKABLE void playSelected(int index);
    Q_INVOKABLE void decideResumePlayback(bool continueFromSavedPosition);
    Q_INVOKABLE bool canDeleteCurrentVideo() const;
    Q_INVOKABLE bool deleteCurrentVideo();
    Q_INVOKABLE bool resetResumeDatabase();
    Q_INVOKABLE bool exportCurrentSkipRanges();
    Q_INVOKABLE bool importCurrentSkipRanges();
    Q_INVOKABLE void clearPlayerMessage();
    Q_INVOKABLE void toggleFastMode();
    Q_INVOKABLE void backToBrowser();
    Q_INVOKABLE void setCurrentIndex(int index);

signals:
    void playerVisibleChanged();
    void currentIndexChanged();
    void currentVideoNameChanged();
    void resumePromptVisibleChanged();
    void pendingResumePositionChanged();
    void playerMessageChanged();
    void fastModeChanged();

private:
    void closePlayer(bool saveResumePosition);
    void handlePlaybackFinished();
    void setPlayerMessage(const QString &message);
    void setPlayerCursorHidden(bool hidden);
    void startPlayback(double startPosition);
    void queueMissingThumbnails(const QVector<VideoItem> &items, quint64 generation);

    VideoLibraryModel *m_libraryModel;
    LibraryScanner *m_scanner;
    PlayerController *m_playerController;
    ResumeRepository *m_resumeRepository;
    ThumbnailService *m_thumbnailService;

    bool m_playerVisible = false;
    bool m_playerCursorHidden = false;
    bool m_resumePromptVisible = false;
    int m_currentIndex = 0;
    QString m_rootVideoFolder;
    QString m_videoFolder;
    QString m_currentFilePath;
    QString m_currentVideoName;
    QString m_playerMessage;
    bool m_fastMode = false;
    double m_pendingResumePosition = 0.0;
    double m_currentAudioDelay = 0.0;
    quint64 m_thumbnailGeneration = 0;
    QThreadPool m_thumbnailThreadPool;
};
