#pragma once

#include <QObject>
#include <QJsonArray>
#include <QNetworkAccessManager>
#include <QString>
#include <QUrl>
#include <QThreadPool>
#include <QVector>
#include "../media/VideoItem.h"

class VideoLibraryModel;
class LibraryScanner;
class PlayerController;
class AppDatabase;
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
    Q_PROPERTY(QString appVersion READ appVersion CONSTANT)
    Q_PROPERTY(QUrl browserBackgroundSource READ browserBackgroundSource CONSTANT)
    Q_PROPERTY(QUrl defaultBrowserBackgroundSource READ defaultBrowserBackgroundSource CONSTANT)
    Q_PROPERTY(bool fastMode READ fastMode NOTIFY fastModeChanged)
    Q_PROPERTY(bool skipImportPromptVisible READ skipImportPromptVisible NOTIFY skipImportPromptVisibleChanged)
    Q_PROPERTY(bool canNavigateUp READ canNavigateUp NOTIFY currentIndexChanged)
    Q_PROPERTY(bool playerNextEpisodeAvailable READ playerNextEpisodeAvailable NOTIFY playerNextEpisodeChanged)
    Q_PROPERTY(QString browserEpisodeInfoTitle READ browserEpisodeInfoTitle NOTIFY browserEpisodeInfoChanged)
    Q_PROPERTY(QString browserEpisodeInfoDescription READ browserEpisodeInfoDescription NOTIFY browserEpisodeInfoChanged)
    Q_PROPERTY(QString browserEpisodeInfoSeriesTitle READ browserEpisodeInfoSeriesTitle NOTIFY browserEpisodeInfoChanged)
    Q_PROPERTY(QString browserEpisodeInfoSeasonEpisode READ browserEpisodeInfoSeasonEpisode NOTIFY browserEpisodeInfoChanged)
    Q_PROPERTY(QUrl browserEpisodeInfoCoverSource READ browserEpisodeInfoCoverSource NOTIFY browserEpisodeInfoChanged)
    Q_PROPERTY(bool browserEpisodeInfoLoading READ browserEpisodeInfoLoading NOTIFY browserEpisodeInfoChanged)

public:
    explicit AppController(VideoLibraryModel *libraryModel,
                           LibraryScanner *scanner,
                           PlayerController *playerController,
                           AppDatabase *database,
                           QObject *parent = nullptr);

    bool playerVisible() const;
    int currentIndex() const;
    QString currentVideoName() const;
    bool resumePromptVisible() const;
    double pendingResumePosition() const;
    QString playerMessage() const;
    QString appVersion() const;
    QUrl browserBackgroundSource() const;
    QUrl defaultBrowserBackgroundSource() const;
    bool fastMode() const;
    bool skipImportPromptVisible() const;
    bool canNavigateUp() const;
    bool playerNextEpisodeAvailable() const;
    QString browserEpisodeInfoTitle() const;
    QString browserEpisodeInfoDescription() const;
    QString browserEpisodeInfoSeriesTitle() const;
    QString browserEpisodeInfoSeasonEpisode() const;
    QUrl browserEpisodeInfoCoverSource() const;
    bool browserEpisodeInfoLoading() const;

    void setConfiguredLibraryFolders(const QJsonArray &folders,
                                     bool enableVirtualRootNavigation = true);
    Q_INVOKABLE void initialize(const QString &videoFolder);
    Q_INVOKABLE void playSelected(int index);
    Q_INVOKABLE void decideResumePlayback(bool continueFromSavedPosition);
    Q_INVOKABLE bool canDeleteCurrentVideo() const;
    Q_INVOKABLE bool deleteCurrentVideo();
    Q_INVOKABLE QString deleteCurrentPromptText() const;
    Q_INVOKABLE bool resetResumeDatabase();
    Q_INVOKABLE bool resetCurrentFolderResumeDatabase();
    Q_INVOKABLE bool exportCurrentSkipRanges();
    Q_INVOKABLE bool importCurrentSkipRanges();
    Q_INVOKABLE bool clearCurrentSkipRanges();
    Q_INVOKABLE void clearPlayerMessage();
    Q_INVOKABLE void respondToSkipImportPrompt(bool shouldImport);
    Q_INVOKABLE void toggleFastMode();
    Q_INVOKABLE void backToBrowser();
    Q_INVOKABLE void setCurrentIndex(int index);
    Q_INVOKABLE void navigateUpOrQuit();
    Q_INVOKABLE bool canOpenBrowserActionDialog() const;
    Q_INVOKABLE void quitApplication();
    Q_INVOKABLE void playNextEpisode();
    Q_INVOKABLE bool requestCurrentBrowserEpisodeInfo();
    Q_INVOKABLE void clearBrowserEpisodeInfo();

signals:
    void playerVisibleChanged();
    void currentIndexChanged();
    void currentVideoNameChanged();
    void resumePromptVisibleChanged();
    void pendingResumePositionChanged();
    void playerMessageChanged();
    void fastModeChanged();
    void skipImportPromptVisibleChanged();
    void playerNextEpisodeChanged();
    void browserEpisodeInfoChanged();

private:
    struct ConfiguredFolderEntry {
        QString name;
        QString path;
    };

    struct SeriesEpisodeRequest {
        QString seriesName;
        QString seriesFileName;
        QString episodeInfoHost;
        QUrl coverSource;
        int season = 0;
        int episode = 0;
    };

    double browserDurationForFile(const QString &filePath, double storedDuration) const;
    void refreshBrowserDurations();
    void queueMissingDurations(const QVector<VideoItem> &items, quint64 generation);
    void closePlayer(bool saveResumePosition);
    void handlePlaybackFinished();
    void setPlayerMessage(const QString &message);
    void setPlayerCursorHidden(bool hidden);
    void startPlayback(double startPosition);
    QString findNextEpisodeFilePath(const QString &filePath) const;
    int indexOfVideoFilePath(const QString &filePath) const;
    void refreshPlayerNextEpisode();
    bool shouldPromptForSkipImport(const QString &filePath) const;
    void setSkipImportPromptVisible(bool visible);
    void queueMissingThumbnails(const QVector<VideoItem> &items, quint64 generation);
    QString resolveRootFolderForPath(const QString &folderPath) const;
    bool canShowConfiguredFoldersRoot() const;
    bool isAtConfiguredFolderRoot() const;
    int indexOfConfiguredFolder(const QString &folderPath) const;
    QVector<VideoItem> configuredFolderRootItems() const;
    void showConfiguredFoldersRoot();
    QString currentBrowserFolderPath() const;
    bool currentSeriesEpisodeRequest(SeriesEpisodeRequest *request) const;
    QString configuredEpisodeInfoHost() const;
    QString configuredEpisodeLookupKeyForSeriesFileName(const QString &seriesFileName) const;
    void requestShowSuggestion(int requestId, const SeriesEpisodeRequest &episodeRequest);
    void requestEpisodeInfoPage(int requestId, const SeriesEpisodeRequest &episodeRequest, const QString &showUrl);
    void setBrowserEpisodeInfoState(const QString &seriesTitle,
                                    const QString &seasonEpisode,
                                    const QString &title,
                                    const QString &description,
                                    const QUrl &coverSource,
                                    bool loading);
    void setBrowserEpisodeInfoFailure();

    VideoLibraryModel *m_libraryModel;
    LibraryScanner *m_scanner;
    PlayerController *m_playerController;
    AppDatabase *m_database;
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
    bool m_skipImportPromptVisible = false;
    double m_pendingResumePosition = 0.0;
    double m_currentAudioDelay = 0.0;
    quint64 m_thumbnailGeneration = 0;
    QThreadPool m_thumbnailThreadPool;
    QNetworkAccessManager m_episodeInfoNetworkManager;
    QVector<ConfiguredFolderEntry> m_configuredFolders;
    QString m_playerNextEpisodeFilePath;
    QString m_browserEpisodeInfoSeriesTitle;
    QString m_browserEpisodeInfoSeasonEpisode;
    QString m_browserEpisodeInfoTitle;
    QString m_browserEpisodeInfoDescription;
    QUrl m_browserEpisodeInfoCoverSource;
    bool m_virtualRootNavigationEnabled = true;
    bool m_showingConfiguredFoldersRoot = false;
    bool m_currentRootFromConfiguration = false;
    bool m_browserEpisodeInfoLoading = false;
    int m_browserEpisodeInfoRequestId = 0;
};
