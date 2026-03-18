#include "AppController.h"

#include "../media/VideoLibraryModel.h"
#include "../media/LibraryScanner.h"
#include "../media/ThumbnailService.h"
#include "../player/PlayerController.h"
#include "../persistence/ResumeRepository.h"
#include <QFutureWatcher>
#include <QCursor>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMetaObject>
#include <QSaveFile>
#include <QStandardPaths>
#include <QtConcurrentRun>
#include <algorithm>
#include <cmath>

AppController::AppController(VideoLibraryModel *libraryModel,
                             LibraryScanner *scanner,
                             PlayerController *playerController,
                             ResumeRepository *resumeRepository,
                             QObject *parent)
    : QObject(parent),
    m_libraryModel(libraryModel),
    m_scanner(scanner),
    m_playerController(playerController),
    m_resumeRepository(resumeRepository),
    m_thumbnailService(scanner ? scanner->thumbnailService() : nullptr)
{
    m_thumbnailThreadPool.setMaxThreadCount(2);

    connect(m_playerController, &PlayerController::playbackFinished,
            this, &AppController::handlePlaybackFinished,
            Qt::QueuedConnection);

    connect(m_playerController, &PlayerController::skipRangesChanged, this, [this]() {
        if (m_currentFilePath.isEmpty() || m_fastMode)
            return;

        m_resumeRepository->saveSkipRanges(
            m_currentFilePath,
            m_playerController->skipRangesData());
    });
}

void AppController::setPlayerCursorHidden(bool hidden)
{
    if (hidden) {
        if (!m_playerCursorHidden) {
            QGuiApplication::setOverrideCursor(QCursor(Qt::BlankCursor));
            m_playerCursorHidden = true;
        }
        return;
    }

    if (m_playerCursorHidden) {
        QGuiApplication::restoreOverrideCursor();
        m_playerCursorHidden = false;
    }
}

bool AppController::playerVisible() const
{
    return m_playerVisible;
}

int AppController::currentIndex() const
{
    return m_currentIndex;
}

QString AppController::currentVideoName() const
{
    return m_currentVideoName;
}

bool AppController::resumePromptVisible() const
{
    return m_resumePromptVisible;
}

double AppController::pendingResumePosition() const
{
    return m_pendingResumePosition;
}

QString AppController::playerMessage() const
{
    return m_playerMessage;
}

bool AppController::fastMode() const
{
    return m_fastMode;
}

bool AppController::skipImportPromptVisible() const
{
    return m_skipImportPromptVisible;
}

bool AppController::canNavigateUp() const
{
    if (!m_libraryModel || m_libraryModel->rowCount() <= 0)
        return false;

    return m_libraryModel->itemAt(0).isParentFolder;
}

double AppController::browserDurationForFile(const QString &filePath, double storedDuration) const
{
    if (filePath.isEmpty())
        return storedDuration;

    const double safeDuration = std::max(0.0, storedDuration);
    if (m_fastMode || safeDuration <= 0.0)
        return safeDuration;

    const double skipDuration = std::max(0.0, m_resumeRepository->loadTotalSkipDuration(filePath));
    return std::max(0.0, safeDuration - skipDuration);
}

void AppController::refreshBrowserDurations()
{
    if (m_videoFolder.isEmpty())
        return;

    const int previousIndex = m_currentIndex;
    initialize(m_videoFolder);

    if (m_libraryModel->rowCount() > 0 && m_currentIndex != previousIndex) {
        m_currentIndex = std::clamp(previousIndex, 0, m_libraryModel->rowCount() - 1);
        emit currentIndexChanged();
    }
}

void AppController::startPlayback(double startPosition)
{
    if (m_resumePromptVisible) {
        m_resumePromptVisible = false;
        emit resumePromptVisibleChanged();
    }

    if (m_pendingResumePosition != 0.0) {
        m_pendingResumePosition = 0.0;
        emit pendingResumePositionChanged();
    }

    const QString fileToPlay = m_currentFilePath;
    const double audioDelay = m_currentAudioDelay;
    const bool shouldPromptSkipImport = shouldPromptForSkipImport(fileToPlay);
    QMetaObject::invokeMethod(this, [this, fileToPlay, startPosition, audioDelay, shouldPromptSkipImport]() {
        if (!m_playerVisible || fileToPlay.isEmpty() || m_currentFilePath != fileToPlay)
            return;
        m_playerController->playFile(fileToPlay, startPosition, audioDelay);
        if (shouldPromptSkipImport) {
            m_playerController->togglePause();
            setSkipImportPromptVisible(true);
        }
    }, Qt::QueuedConnection);
}

void AppController::initialize(const QString &videoFolder)
{
    if (m_rootVideoFolder.isEmpty()) {
        m_rootVideoFolder = videoFolder;
        if (m_scanner)
            m_scanner->setRootFolder(m_rootVideoFolder);
    }

    m_videoFolder = videoFolder;
    const quint64 generation = ++m_thumbnailGeneration;
    auto items = m_scanner->scan(videoFolder);

    for (auto &item : items) {
        if (!item.isFolder && !item.filePath.isEmpty()) {
            item.resumePosition = m_resumeRepository->loadPosition(item.filePath);
            const double storedDuration = m_resumeRepository->loadDuration(item.filePath);
            item.duration = browserDurationForFile(item.filePath, storedDuration);
        }
    }

    m_libraryModel->setItems(items);
    queueMissingDurations(items, generation);
    queueMissingThumbnails(items, generation);

    const int count = m_libraryModel->rowCount();
    const int previousIndex = m_currentIndex;
    if (count <= 0)
        m_currentIndex = 0;
    else
        m_currentIndex = std::clamp(m_currentIndex, 0, count - 1);

    if (m_currentIndex != previousIndex)
        emit currentIndexChanged();
}

void AppController::queueMissingThumbnails(const QVector<VideoItem> &items, quint64 generation)
{
    if (!m_thumbnailService)
        return;

    for (const auto &item : items) {
        if (item.isFolder || item.isDemo || item.filePath.isEmpty() || !item.coverPath.isEmpty())
            continue;

        auto *watcher = new QFutureWatcher<QString>(this);
        connect(watcher, &QFutureWatcher<QString>::finished, this, [this, watcher, filePath = item.filePath, generation]() {
            const QString coverPath = watcher->result();
            watcher->deleteLater();

            if (generation != m_thumbnailGeneration || coverPath.isEmpty())
                return;

            m_libraryModel->updateCoverPath(filePath, coverPath);
        });

        watcher->setFuture(QtConcurrent::run(&m_thumbnailThreadPool, [thumbnailService = m_thumbnailService, filePath = item.filePath]() -> QString {
            if (!thumbnailService)
                return {};
            return thumbnailService->ensureThumbnail(filePath);
        }));
    }
}

void AppController::queueMissingDurations(const QVector<VideoItem> &items, quint64 generation)
{
    if (!m_thumbnailService)
        return;

    for (const auto &item : items) {
        if (item.isFolder || item.isDemo || item.filePath.isEmpty())
            continue;

        if (m_resumeRepository->loadDuration(item.filePath) > 0.0)
            continue;

        const double resumePosition = item.resumePosition;
        const double audioDelay = m_resumeRepository->loadAudioDelay(item.filePath);

        auto *watcher = new QFutureWatcher<double>(this);
        connect(watcher, &QFutureWatcher<double>::finished, this, [this, watcher, filePath = item.filePath, resumePosition, audioDelay, generation]() {
            const double duration = watcher->result();
            watcher->deleteLater();

            if (generation != m_thumbnailGeneration || duration <= 0.0)
                return;

            m_resumeRepository->savePosition(filePath, resumePosition, duration, audioDelay);
            m_libraryModel->updateDuration(filePath, browserDurationForFile(filePath, duration));
        });

        watcher->setFuture(QtConcurrent::run(&m_thumbnailThreadPool, [thumbnailService = m_thumbnailService, filePath = item.filePath]() -> double {
            if (!thumbnailService)
                return 0.0;
            return thumbnailService->probeDuration(filePath);
        }));
    }
}

void AppController::playSelected(int index)
{
    const auto item = m_libraryModel->itemAt(index);
    if (item.filePath.isEmpty())
        return;

    if (item.isFolder) {
        initialize(item.filePath);
        if (m_currentIndex != 0) {
            m_currentIndex = 0;
            emit currentIndexChanged();
        }
        return;
    }

    m_currentIndex = index;
    emit currentIndexChanged();

    m_playerVisible = true;
    emit playerVisibleChanged();
    setPlayerCursorHidden(true);

    m_currentFilePath = item.filePath;
    m_currentVideoName = QFileInfo(item.filePath).completeBaseName();
    m_currentAudioDelay = m_resumeRepository->loadAudioDelay(item.filePath);
    m_playerController->setSkipHandlingEnabled(!m_fastMode);
    m_playerController->setSkipRanges(
        m_fastMode ? QVector<SkipRange>() : m_resumeRepository->loadSkipRanges(item.filePath));
    emit currentVideoNameChanged();

    const double loadedResume = m_resumeRepository->loadPosition(item.filePath);
    const double resumePosition = std::isfinite(loadedResume) ? std::max(0.0, loadedResume) : 0.0;

    if (resumePosition > 3.0) {
        m_pendingResumePosition = resumePosition;
        emit pendingResumePositionChanged();
        m_resumePromptVisible = true;
        emit resumePromptVisibleChanged();
    } else {
        if (m_pendingResumePosition != 0.0) {
            m_pendingResumePosition = 0.0;
            emit pendingResumePositionChanged();
        }
        startPlayback(0.0);
    }
}

bool AppController::deleteCurrentVideo()
{
    const auto item = m_libraryModel->itemAt(m_currentIndex);
    if (item.filePath.isEmpty())
        return false;

    if (item.isFolder) {
        if (item.isParentFolder)
            return false;

        QDir dir(item.filePath);
        const QFileInfoList entries = dir.entryInfoList(
            QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System);
        const bool canDeleteFolder =
            entries.isEmpty()
            || (entries.size() == 1
                && entries.first().isFile()
                && entries.first().fileName().compare("folder.jpg", Qt::CaseInsensitive) == 0);
        if (!canDeleteFolder)
            return false;

        if (entries.size() == 1 && !QFile::remove(entries.first().absoluteFilePath()))
            return false;

        if (!dir.rmdir(item.filePath))
            return false;

        const int previousIndex = m_currentIndex;
        initialize(m_videoFolder);

        const int count = m_libraryModel->rowCount();
        int nextIndex = 0;
        if (count > 0)
            nextIndex = std::clamp(previousIndex, 0, count - 1);

        if (m_currentIndex != nextIndex) {
            m_currentIndex = nextIndex;
            emit currentIndexChanged();
        }

        return true;
    }

    const QFileInfo fileInfo(item.filePath);
    const QString basePath = fileInfo.absolutePath() + "/" + fileInfo.completeBaseName();
    const QString coverPath = basePath + ".jpg";
    const QString skipFilePath = basePath + "_skip.json";

    m_resumeRepository->deletePosition(item.filePath);
    m_resumeRepository->deleteSkipRanges(item.filePath);

    if (!QFile::remove(item.filePath))
        return false;

    if (QFileInfo::exists(coverPath))
        QFile::remove(coverPath);

    if (QFileInfo::exists(skipFilePath))
        QFile::remove(skipFilePath);

    const int previousIndex = m_currentIndex;
    initialize(m_videoFolder);

    const int count = m_libraryModel->rowCount();
    int nextIndex = 0;
    if (count > 0)
        nextIndex = std::clamp(previousIndex, 0, count - 1);

    if (m_currentIndex != nextIndex) {
        m_currentIndex = nextIndex;
        emit currentIndexChanged();
    }

    return true;
}

bool AppController::canDeleteCurrentVideo() const
{
    const auto item = m_libraryModel->itemAt(m_currentIndex);
    if (item.filePath.isEmpty() || item.isDemo || item.isParentFolder)
        return false;

    if (!item.isFolder)
        return true;

    QDir dir(item.filePath);
    const QFileInfoList entries = dir.entryInfoList(
        QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System);
    return entries.isEmpty()
           || (entries.size() == 1
               && entries.first().isFile()
               && entries.first().fileName().compare("folder.jpg", Qt::CaseInsensitive) == 0);
}

QString AppController::deleteCurrentPromptText() const
{
    const auto item = m_libraryModel->itemAt(m_currentIndex);
    if (item.isFolder && !item.isParentFolder)
        return "Diesen Ordner wirklich loeschen?";

    return "Dieses Video wirklich loeschen?";
}

bool AppController::resetResumeDatabase()
{
    if (!m_resumeRepository->clearAllPositions())
        return false;

    initialize(m_videoFolder);
    return true;
}

bool AppController::resetCurrentFolderResumeDatabase()
{
    if (m_videoFolder.isEmpty())
        return false;

    if (!m_resumeRepository->clearFolderEntries(m_videoFolder))
        return false;

    initialize(m_videoFolder);
    return true;
}

bool AppController::exportCurrentSkipRanges()
{
    if (m_fastMode)
        return false;

    if (m_currentFilePath.isEmpty()) {
        setPlayerMessage("Kein aktuelles Video aktiv.");
        return false;
    }

    const QVector<SkipRange> ranges = m_playerController->skipRangesData();
    if (ranges.isEmpty()) {
        setPlayerMessage("Keine Skip-Bereiche zum Exportieren vorhanden.");
        return false;
    }

    const QFileInfo fileInfo(m_currentFilePath);
    const QString videoDir = fileInfo.absolutePath();
    QDir().mkpath(videoDir);

    const QString exportPath = videoDir + "/" + fileInfo.completeBaseName() + "_skip.json";

    QJsonArray jsonRanges;
    for (const SkipRange &range : ranges) {
        QJsonObject jsonRange;
        jsonRange.insert("start", range.start);
        jsonRange.insert("end", range.end);
        jsonRanges.append(jsonRange);
    }

    QJsonObject rootObject;
    rootObject.insert("video_name", fileInfo.completeBaseName());
    rootObject.insert("source_file", fileInfo.fileName());
    rootObject.insert("skip_ranges", jsonRanges);

    QSaveFile file(exportPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        setPlayerMessage("Export fehlgeschlagen.");
        return false;
    }

    const QJsonDocument document(rootObject);
    if (file.write(document.toJson(QJsonDocument::Indented)) < 0) {
        setPlayerMessage("Export fehlgeschlagen.");
        return false;
    }

    if (!file.commit()) {
        setPlayerMessage("Export fehlgeschlagen.");
        return false;
    }

    setPlayerMessage("Skip-Bereiche im Video-Ordner gespeichert.");
    return true;
}

bool AppController::importCurrentSkipRanges()
{
    if (m_fastMode)
        return false;

    if (m_currentFilePath.isEmpty()) {
        setPlayerMessage("Kein aktuelles Video aktiv.");
        return false;
    }

    const QFileInfo fileInfo(m_currentFilePath);
    const QString importFileName = fileInfo.completeBaseName() + "_skip.json";

    const QString videoDir = fileInfo.absolutePath();
    QString downloadDir = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    if (downloadDir.isEmpty())
        downloadDir = QDir::homePath() + "/Downloads";

    QString importPath = videoDir + "/" + importFileName;
    if (!QFileInfo::exists(importPath))
        importPath = downloadDir + "/" + importFileName;

    QFile file(importPath);
    if (!file.open(QIODevice::ReadOnly)) {
        setPlayerMessage("Keine passende Skip-Datei im Video-Ordner oder in Downloads gefunden.");
        return false;
    }

    const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
    if (!document.isObject()) {
        setPlayerMessage("Skip-Datei ist ungueltig.");
        return false;
    }

    const QJsonValue rangesValue = document.object().value("skip_ranges");
    if (!rangesValue.isArray()) {
        setPlayerMessage("Skip-Datei ist ungueltig.");
        return false;
    }

    QVector<SkipRange> ranges;
    const QJsonArray jsonRanges = rangesValue.toArray();
    ranges.reserve(jsonRanges.size());

    for (const QJsonValue &value : jsonRanges) {
        if (!value.isObject())
            continue;

        const QJsonObject rangeObject = value.toObject();
        const QJsonValue startValue = rangeObject.value("start");
        const QJsonValue endValue = rangeObject.value("end");
        if (!startValue.isDouble() || !endValue.isDouble())
            continue;

        ranges.append({ startValue.toDouble(), endValue.toDouble() });
    }

    m_playerController->clearPendingSkipRange();
    m_playerController->setSkipRanges(ranges);
    if (!m_resumeRepository->saveSkipRanges(
        m_currentFilePath,
        m_playerController->skipRangesData())) {
        setPlayerMessage("Import fehlgeschlagen.");
        return false;
    }

    return true;
}

bool AppController::clearCurrentSkipRanges()
{
    if (m_fastMode)
        return false;

    if (m_currentFilePath.isEmpty()) {
        setPlayerMessage("Kein aktuelles Video aktiv.");
        return false;
    }

    m_playerController->clearPendingSkipRange();
    m_playerController->setSkipRanges({});

    if (!m_resumeRepository->deleteSkipRanges(m_currentFilePath)) {
        setPlayerMessage("Skip-Bereiche konnten nicht geloescht werden.");
        return false;
    }

    return true;
}

void AppController::clearPlayerMessage()
{
    setPlayerMessage(QString());
}

void AppController::respondToSkipImportPrompt(bool shouldImport)
{
    if (!m_skipImportPromptVisible)
        return;

    setSkipImportPromptVisible(false);

    if (shouldImport) {
        importCurrentSkipRanges();
        return;
    }

    if (m_playerController->paused())
        m_playerController->togglePause();
}

void AppController::toggleFastMode()
{
    m_fastMode = !m_fastMode;
    m_playerController->setSkipHandlingEnabled(!m_fastMode);
    m_playerController->clearPendingSkipRange();
    m_playerController->setSkipRanges(
        m_fastMode || m_currentFilePath.isEmpty()
            ? QVector<SkipRange>()
            : m_resumeRepository->loadSkipRanges(m_currentFilePath));
    if (m_fastMode)
        setSkipImportPromptVisible(false);
    emit fastModeChanged();

    if (!m_playerVisible)
        refreshBrowserDurations();

    if (m_fastMode)
        setPlayerMessage("Fast-Modus aktiviert. Skip-Bereiche sind deaktiviert.");
    else
        setPlayerMessage("Normalmodus aktiviert. Skip-Bereiche werden wieder beruecksichtigt.");
}

void AppController::decideResumePlayback(bool continueFromSavedPosition)
{
    if (!m_resumePromptVisible)
        return;

    const double startPosition = continueFromSavedPosition ? m_pendingResumePosition : 0.0;
    startPlayback(startPosition);
}

void AppController::closePlayer(bool saveResumePosition)
{
    const QString currentFilePath = m_currentFilePath;

    if (saveResumePosition && !m_resumePromptVisible) {
        const double pos = m_playerController->position();
        const double dur = m_playerController->duration();
        double savePos = std::max(0.0, pos);
        if (dur > 0.0)
            savePos = std::min(savePos, dur);

        if (!currentFilePath.isEmpty()) {
            m_resumeRepository->savePosition(
                currentFilePath,
                savePos,
                dur,
                m_playerController->audioDelay());
            m_libraryModel->updatePlaybackState(
                currentFilePath,
                savePos,
                browserDurationForFile(currentFilePath, dur));
        }
    }

    m_currentFilePath.clear();
    m_playerController->stop();
    m_playerController->setSkipHandlingEnabled(!m_fastMode);
    m_playerController->setSkipRanges({});
    setPlayerCursorHidden(false);
    setSkipImportPromptVisible(false);

    if (m_resumePromptVisible) {
        m_resumePromptVisible = false;
        emit resumePromptVisibleChanged();
    }
    if (m_pendingResumePosition != 0.0) {
        m_pendingResumePosition = 0.0;
        emit pendingResumePositionChanged();
    }

    m_playerVisible = false;
    emit playerVisibleChanged();

    m_currentAudioDelay = 0.0;
    if (!m_currentVideoName.isEmpty()) {
        m_currentVideoName.clear();
        emit currentVideoNameChanged();
    }
    setPlayerMessage(QString());
}

void AppController::handlePlaybackFinished()
{
    if (!m_currentFilePath.isEmpty()) {
        m_resumeRepository->deletePosition(m_currentFilePath);
        m_libraryModel->updatePlaybackState(m_currentFilePath, 0.0, 0.0);
    }

    closePlayer(false);
}

void AppController::backToBrowser()
{
    closePlayer(true);
}

void AppController::setCurrentIndex(int index)
{
    if (m_currentIndex == index)
        return;
    m_currentIndex = index;
    emit currentIndexChanged();
}

void AppController::navigateUpOrQuit()
{
    if (canNavigateUp()) {
        initialize(m_libraryModel->itemAt(0).filePath);
        if (m_currentIndex != 0) {
            m_currentIndex = 0;
            emit currentIndexChanged();
        }
        return;
    }

    QCoreApplication::quit();
}

void AppController::setPlayerMessage(const QString &message)
{
    if (m_playerMessage == message)
        return;

    m_playerMessage = message;
    emit playerMessageChanged();
}

bool AppController::shouldPromptForSkipImport(const QString &filePath) const
{
    if (m_fastMode || filePath.isEmpty())
        return false;

    if (!m_resumeRepository->loadSkipRanges(filePath).isEmpty())
        return false;

    const QFileInfo fileInfo(filePath);
    const QString importPath = fileInfo.absolutePath() + "/" + fileInfo.completeBaseName() + "_skip.json";
    return QFileInfo::exists(importPath);
}

void AppController::setSkipImportPromptVisible(bool visible)
{
    if (m_skipImportPromptVisible == visible)
        return;

    m_skipImportPromptVisible = visible;
    emit skipImportPromptVisibleChanged();
}
