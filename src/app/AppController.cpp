#include "AppController.h"

#include "../media/VideoLibraryModel.h"
#include "../media/LibraryScanner.h"
#include "../media/ThumbnailService.h"
#include "../player/PlayerController.h"
#include "../shared/AppConfig.h"
#include "../shared/AppDatabase.h"
#include <QFutureWatcher>
#include <QCursor>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMetaObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QSaveFile>
#include <QSslError>
#include <QStandardPaths>
#include <QTextDocumentFragment>
#include <QUrlQuery>
#include <QtConcurrentRun>
#include <algorithm>
#include <cmath>

#ifndef COVERFLOWMP_APP_VERSION
#define COVERFLOWMP_APP_VERSION "unbekannt"
#endif

namespace {

QString expandUserPath(const QString &path)
{
    if (path == QStringLiteral("~"))
        return QDir::homePath();
    if (path.startsWith(QStringLiteral("~/")))
        return QDir::homePath() + path.mid(1);
    return path;
}

QString normalizedFolderPath(const QString &path)
{
    return QDir(expandUserPath(path)).absolutePath();
}

bool isSameOrChildPath(const QString &rootPath, const QString &candidatePath)
{
    if (rootPath.isEmpty() || candidatePath.isEmpty())
        return false;

    if (candidatePath == rootPath)
        return true;

    return candidatePath.startsWith(rootPath + QDir::separator());
}

QString seriesSearchNameFromTitle(const QString &title, int episodeMarkerStart)
{
    QString searchName = title.left(episodeMarkerStart).trimmed();
    searchName.replace(QRegularExpression(QStringLiteral("[._-]+")), QStringLiteral(" "));
    searchName.replace(QRegularExpression(QStringLiteral("([A-Z]+)([A-Z][a-z])")), QStringLiteral("\\1 \\2"));
    searchName.replace(QRegularExpression(QStringLiteral("([a-z0-9])([A-Z])")), QStringLiteral("\\1 \\2"));
    return searchName.simplified();
}

bool parseSeriesEpisodeTitle(const QString &title, QString *seriesName, int *season, int *episode)
{
    static const QRegularExpression episodePattern(
        QStringLiteral("(\\d{1,2})x(\\d{2})"),
        QRegularExpression::CaseInsensitiveOption);

    const QRegularExpressionMatch match = episodePattern.match(title);
    if (!match.hasMatch())
        return false;

    const QString searchName = seriesSearchNameFromTitle(title, match.capturedStart());
    if (searchName.isEmpty())
        return false;

    bool seasonOk = false;
    bool episodeOk = false;
    const int parsedSeason = match.captured(1).toInt(&seasonOk);
    const int parsedEpisode = match.captured(2).toInt(&episodeOk);
    if (!seasonOk || !episodeOk || parsedSeason <= 0 || parsedEpisode <= 0)
        return false;

    if (seriesName)
        *seriesName = searchName;
    if (season)
        *season = parsedSeason;
    if (episode)
        *episode = parsedEpisode;

    return true;
}

QString decodedHtmlText(const QString &html)
{
    return QTextDocumentFragment::fromHtml(html).toPlainText().simplified();
}

QString htmlAttributeValue(const QString &tag, const QString &attributeName)
{
    const QRegularExpression attributeRegex(
        QStringLiteral("\\b%1\\s*=\\s*([\"'])(.*?)\\1")
            .arg(QRegularExpression::escape(attributeName)),
        QRegularExpression::CaseInsensitiveOption | QRegularExpression::DotMatchesEverythingOption);
    const QRegularExpressionMatch match = attributeRegex.match(tag);
    return match.hasMatch() ? match.captured(2) : QString();
}

QString debugPreview(const QByteArray &data, int maxLength = 500)
{
    QString preview = QString::fromUtf8(data.left(maxLength)).simplified();
    if (data.size() > maxLength)
        preview += QStringLiteral(" ...");
    return preview;
}

QString seasonEpisodeCode(int season, int episode)
{
    return QStringLiteral("S%1E%2")
        .arg(season, 2, 10, QLatin1Char('0'))
        .arg(episode, 2, 10, QLatin1Char('0'));
}

QString extractEpisodeTitleFromHtml(const QString &html, int season, int episode)
{
    static const QRegularExpression headingRegex(
        QStringLiteral("<h2\\b[^>]*>(.*?)</h2>"),
        QRegularExpression::CaseInsensitiveOption | QRegularExpression::DotMatchesEverythingOption);

    const QString episodeCode = seasonEpisodeCode(season, episode);
    QRegularExpressionMatchIterator iterator = headingRegex.globalMatch(html);
    while (iterator.hasNext()) {
        const QString headingText = decodedHtmlText(iterator.next().captured(1));
        const QRegularExpression titleRegex(
            QStringLiteral("^\\s*%1\\s*:\\s*(.+)$")
                .arg(QRegularExpression::escape(episodeCode)),
            QRegularExpression::CaseInsensitiveOption);
        const QRegularExpressionMatch titleMatch = titleRegex.match(headingText);
        if (titleMatch.hasMatch())
            return titleMatch.captured(1).simplified();
    }

    return {};
}

QString extractEpisodeDescriptionFromHtml(const QString &html)
{
    static const QRegularExpression descriptionLinkRegex(
        QStringLiteral("<a\\b(?=[^>]*>\\s*Beschreibung\\s+anzeigen\\s*</a>)(?=[^>]*\\bhref\\s*=\\s*([\"'])#([^\"']+)\\1)[^>]*>"),
        QRegularExpression::CaseInsensitiveOption | QRegularExpression::DotMatchesEverythingOption);
    static const QRegularExpression classDescriptionRegex(
        QStringLiteral("<div\\b(?=[^>]*\\bclass\\s*=\\s*([\"'])(?=[^\"']*\\bsmall\\b)(?=[^\"']*\\btext-body\\b)(?=[^\"']*\\blh-lg\\b)(?=[^\"']*\\bmb-3\\b)[^\"']*\\1)[^>]*>(.*?)</div>"),
        QRegularExpression::CaseInsensitiveOption | QRegularExpression::DotMatchesEverythingOption);
    static const QRegularExpression divRegex(
        QStringLiteral("<div\\b[^>]*\\bclass\\s*=\\s*([\"'])([^\"']*)\\1[^>]*>(.*?)</div>"),
        QRegularExpression::CaseInsensitiveOption | QRegularExpression::DotMatchesEverythingOption);

    const QRegularExpressionMatch linkMatch = descriptionLinkRegex.match(html);
    if (linkMatch.hasMatch()) {
        const QString targetId = linkMatch.captured(2);
        qDebug() << "[EpisodeInfo] Beschreibung-Link gefunden, targetId=" << targetId;

        const QRegularExpression targetRegex(
            QStringLiteral("<div\\b(?=[^>]*\\bid\\s*=\\s*([\"'])%1\\1)[^>]*>(.*?)<section\\b")
                .arg(QRegularExpression::escape(targetId)),
            QRegularExpression::CaseInsensitiveOption | QRegularExpression::DotMatchesEverythingOption);
        const QRegularExpressionMatch targetMatch = targetRegex.match(html, linkMatch.capturedEnd());
        if (targetMatch.hasMatch()) {
            const QRegularExpressionMatch descriptionMatch = classDescriptionRegex.match(targetMatch.captured(2));
            if (descriptionMatch.hasMatch()) {
                const QString description = decodedHtmlText(descriptionMatch.captured(2));
                if (!description.isEmpty())
                    return description;
            }
        } else {
            qDebug() << "[EpisodeInfo] Beschreibung-Zielcontainer nicht gefunden fuer" << targetId;
        }
    } else {
        qDebug() << "[EpisodeInfo] Beschreibung-Link nicht gefunden, nutze Klassen-Fallback.";
    }

    const QRegularExpressionMatch directClassMatch = classDescriptionRegex.match(html);
    if (directClassMatch.hasMatch()) {
        const QString description = decodedHtmlText(directClassMatch.captured(2));
        if (!description.isEmpty())
            return description;
    }

    QRegularExpressionMatchIterator iterator = divRegex.globalMatch(html);
    while (iterator.hasNext()) {
        const QRegularExpressionMatch match = iterator.next();
        const QStringList classes = match.captured(2).split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
        if (classes.contains(QStringLiteral("small"))
            && classes.contains(QStringLiteral("text-body"))
            && classes.contains(QStringLiteral("lh-lg"))
            && classes.contains(QStringLiteral("mb-3"))) {
            return decodedHtmlText(match.captured(3));
        }
    }

    return {};
}

QUrl extractEpisodeCoverSourceFromHtml(const QString &html, const QUrl &baseUrl)
{
    static const QRegularExpression imageRegex(
        QStringLiteral("<img\\b[^>]*\\bclass\\s*=\\s*([\"'])(?=[^\"']*\\bimg-fluid\\b)[^\"']*\\1[^>]*>"),
        QRegularExpression::CaseInsensitiveOption | QRegularExpression::DotMatchesEverythingOption);

    QRegularExpressionMatchIterator iterator = imageRegex.globalMatch(html);
    while (iterator.hasNext()) {
        const QString imageTag = iterator.next().captured(0);
        QString source = htmlAttributeValue(imageTag, QStringLiteral("data-src")).trimmed();
        if (source.isEmpty())
            source = htmlAttributeValue(imageTag, QStringLiteral("src")).trimmed();
        if (source.isEmpty() || source.startsWith(QStringLiteral("data:"), Qt::CaseInsensitive))
            continue;

        const QUrl resolvedSource = baseUrl.resolved(QUrl(source));
        qDebug() << "[EpisodeInfo] HTML-Cover gefunden:" << resolvedSource.toString();
        return resolvedSource;
    }

    qDebug() << "[EpisodeInfo] Kein img.img-fluid Cover in HTML gefunden.";
    return {};
}

int regexMatchCount(const QString &text, const QRegularExpression &regex)
{
    int count = 0;
    QRegularExpressionMatchIterator iterator = regex.globalMatch(text);
    while (iterator.hasNext()) {
        iterator.next();
        ++count;
    }
    return count;
}

int headingCount(const QString &html)
{
    static const QRegularExpression headingRegex(
        QStringLiteral("<h2\\b[^>]*>"),
        QRegularExpression::CaseInsensitiveOption);
    return regexMatchCount(html, headingRegex);
}

}

AppController::AppController(VideoLibraryModel *libraryModel,
                             LibraryScanner *scanner,
                             PlayerController *playerController,
                             AppDatabase *database,
                             QObject *parent)
    : QObject(parent),
    m_libraryModel(libraryModel),
    m_scanner(scanner),
    m_playerController(playerController),
    m_database(database),
    m_thumbnailService(scanner ? scanner->thumbnailService() : nullptr)
{
    m_thumbnailThreadPool.setMaxThreadCount(2);

    connect(&m_episodeInfoNetworkManager,
            &QNetworkAccessManager::sslErrors,
            this,
            [](QNetworkReply *reply, const QList<QSslError> &errors) {
                qDebug() << "[EpisodeInfo] TLS-Zertifikatsfehler fuer"
                         << (reply ? reply->url().toString() : QStringLiteral("<kein Reply>"))
                         << "- werden ignoriert.";
                for (const QSslError &error : errors)
                    qDebug() << "[EpisodeInfo] TLS error:" << error.errorString();

                if (reply)
                    reply->ignoreSslErrors();
            });

    connect(m_playerController, &PlayerController::playbackFinished,
            this, &AppController::handlePlaybackFinished,
            Qt::QueuedConnection);

    connect(m_playerController, &PlayerController::skipRangesChanged, this, [this]() {
        if (m_currentFilePath.isEmpty() || m_fastMode)
            return;

        m_database->saveSkipRanges(
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

QString AppController::appVersion() const
{
    return QStringLiteral(COVERFLOWMP_APP_VERSION);
}

QUrl AppController::browserBackgroundSource() const
{
    const QString configuredBackground = m_database
        ? m_database->loadConfigString(AppConfig::browserBackgroundKey(),
                                       AppConfig::defaultBrowserBackground())
        : AppConfig::defaultBrowserBackground();

    if (configuredBackground.isEmpty()
        || configuredBackground == AppConfig::defaultBrowserBackground()) {
        return defaultBrowserBackgroundSource();
    }

    const QFileInfo backgroundFile(expandUserPath(configuredBackground));
    if (backgroundFile.exists() && backgroundFile.isFile())
        return QUrl::fromLocalFile(backgroundFile.absoluteFilePath());

    return defaultBrowserBackgroundSource();
}

QUrl AppController::defaultBrowserBackgroundSource() const
{
    return QUrl(QStringLiteral("qrc:/assets/wallpaper.jpg"));
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
    if (m_showingConfiguredFoldersRoot)
        return false;

    if (!m_libraryModel || m_libraryModel->rowCount() <= 0)
        return false;

    return m_libraryModel->itemAt(0).isParentFolder;
}

QString AppController::browserEpisodeInfoTitle() const
{
    return m_browserEpisodeInfoTitle;
}

QString AppController::browserEpisodeInfoDescription() const
{
    return m_browserEpisodeInfoDescription;
}

QString AppController::browserEpisodeInfoSeriesTitle() const
{
    return m_browserEpisodeInfoSeriesTitle;
}

QString AppController::browserEpisodeInfoSeasonEpisode() const
{
    return m_browserEpisodeInfoSeasonEpisode;
}

QUrl AppController::browserEpisodeInfoCoverSource() const
{
    return m_browserEpisodeInfoCoverSource;
}

bool AppController::browserEpisodeInfoLoading() const
{
    return m_browserEpisodeInfoLoading;
}

void AppController::setConfiguredLibraryFolders(const QJsonArray &folders,
                                                bool enableVirtualRootNavigation)
{
    m_virtualRootNavigationEnabled = enableVirtualRootNavigation;
    m_configuredFolders.clear();

    for (const QJsonValue &entry : folders) {
        if (!entry.isObject())
            continue;

        const QJsonObject folderObject = entry.toObject();
        const QString name = folderObject.value(QStringLiteral("name")).toString().trimmed();
        const QString path = normalizedFolderPath(
            folderObject.value(QStringLiteral("path")).toString().trimmed());
        if (name.isEmpty() || path.isEmpty())
            continue;

        m_configuredFolders.push_back({ name, path });
    }
}

double AppController::browserDurationForFile(const QString &filePath, double storedDuration) const
{
    if (filePath.isEmpty())
        return storedDuration;

    const double safeDuration = std::max(0.0, storedDuration);
    if (m_fastMode || safeDuration <= 0.0)
        return safeDuration;

    const double skipDuration = std::max(0.0, m_database->loadTotalSkipDuration(filePath));
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
    const QString normalizedFolder = normalizedFolderPath(videoFolder);
    const QString resolvedRootFolder = resolveRootFolderForPath(normalizedFolder);
    const QString activeRootFolder = resolvedRootFolder.isEmpty()
        ? normalizedFolder
        : resolvedRootFolder;
    m_currentRootFromConfiguration = !resolvedRootFolder.isEmpty();

    if (m_rootVideoFolder != activeRootFolder) {
        m_rootVideoFolder = activeRootFolder;
        if (m_scanner)
            m_scanner->setRootFolder(m_rootVideoFolder);
    }

    m_showingConfiguredFoldersRoot = false;
    m_videoFolder = normalizedFolder;
    const quint64 generation = ++m_thumbnailGeneration;
    auto items = m_scanner->scan(m_videoFolder);

    for (auto &item : items) {
        if (!item.isFolder && !item.filePath.isEmpty()) {
            item.resumePosition = m_database->loadPosition(item.filePath);
            const double storedDuration = m_database->loadDuration(item.filePath);
            item.totalDuration = storedDuration;
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

        if (m_database->loadDuration(item.filePath) > 0.0)
            continue;

        const double resumePosition = item.resumePosition;
        const double audioDelay = m_database->loadAudioDelay(item.filePath);

        auto *watcher = new QFutureWatcher<double>(this);
        connect(watcher, &QFutureWatcher<double>::finished, this, [this, watcher, filePath = item.filePath, resumePosition, audioDelay, generation]() {
            const double duration = watcher->result();
            watcher->deleteLater();

            if (generation != m_thumbnailGeneration || duration <= 0.0)
                return;

            m_database->savePosition(filePath, resumePosition, duration, audioDelay);
            m_libraryModel->updateDuration(
                filePath,
                browserDurationForFile(filePath, duration),
                duration);
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
    m_currentAudioDelay = m_database->loadAudioDelay(item.filePath);
    m_playerController->setSkipHandlingEnabled(!m_fastMode);
    m_playerController->setSkipRanges(
        m_fastMode ? QVector<SkipRange>() : m_database->loadSkipRanges(item.filePath));
    emit currentVideoNameChanged();

    const double loadedResume = m_database->loadPosition(item.filePath);
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
    if (item.filePath.isEmpty() || item.isVirtualRootEntry)
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

    m_database->deletePosition(item.filePath);
    m_database->deleteSkipRanges(item.filePath);

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
    if (item.filePath.isEmpty() || item.isDemo || item.isParentFolder || item.isVirtualRootEntry)
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
    if (!m_database->clearAllPlaybackState())
        return false;

    initialize(m_videoFolder);
    return true;
}

bool AppController::resetCurrentFolderResumeDatabase()
{
    const QString folderPath = currentBrowserFolderPath();
    if (folderPath.isEmpty())
        return false;

    if (!m_database->clearFolderPlaybackState(folderPath))
        return false;

    if (m_showingConfiguredFoldersRoot)
        showConfiguredFoldersRoot();
    else
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
    if (!m_database->saveSkipRanges(
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

    if (!m_database->deleteSkipRanges(m_currentFilePath)) {
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
            : m_database->loadSkipRanges(m_currentFilePath));
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
            m_database->savePosition(
                currentFilePath,
                savePos,
                dur,
                m_playerController->audioDelay());
            m_libraryModel->updatePlaybackState(
                currentFilePath,
                savePos,
                browserDurationForFile(currentFilePath, dur),
                dur);
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
        m_database->deletePosition(m_currentFilePath);
        m_libraryModel->updatePlaybackState(m_currentFilePath, 0.0, 0.0, 0.0);
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
    if (m_database) {
        setConfiguredLibraryFolders(
            m_database->loadConfigArray(
                AppConfig::libraryFoldersKey(),
                AppConfig::defaultLibraryFoldersArray()),
            m_virtualRootNavigationEnabled);
    }

    if (canNavigateUp()) {
        initialize(m_libraryModel->itemAt(0).filePath);
        if (m_currentIndex != 0) {
            m_currentIndex = 0;
            emit currentIndexChanged();
        }
        return;
    }

    if (canShowConfiguredFoldersRoot()) {
        showConfiguredFoldersRoot();
        return;
    }
}

bool AppController::canOpenBrowserActionDialog() const
{
    return m_showingConfiguredFoldersRoot || isAtConfiguredFolderRoot();
}

void AppController::quitApplication()
{
    QCoreApplication::quit();
}

bool AppController::requestCurrentBrowserEpisodeInfo()
{
    SeriesEpisodeRequest episodeRequest;
    if (!currentSeriesEpisodeRequest(&episodeRequest)) {
        qDebug() << "[EpisodeInfo] Kein abrufbares Serienvideo ausgewaehlt.";
        return false;
    }

    const int requestId = ++m_browserEpisodeInfoRequestId;
    qDebug() << "[EpisodeInfo] Start request" << requestId
             << "seriesName=" << episodeRequest.seriesName
             << "season=" << episodeRequest.season
             << "episode=" << episodeRequest.episode;
    setBrowserEpisodeInfoState(
        episodeRequest.seriesName,
        QStringLiteral("S.%1 E.%2").arg(episodeRequest.season).arg(episodeRequest.episode),
        QStringLiteral("Episodeninfo wird geladen..."),
        QString(),
        episodeRequest.coverSource,
        true);
    requestShowSuggestion(requestId, episodeRequest);
    return true;
}

void AppController::clearBrowserEpisodeInfo()
{
    ++m_browserEpisodeInfoRequestId;
    qDebug() << "[EpisodeInfo] Clear current sidebar state. Next request id is" << m_browserEpisodeInfoRequestId;
    setBrowserEpisodeInfoState(QString(), QString(), QString(), QString(), QUrl(), false);
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

    if (!m_database->loadSkipRanges(filePath).isEmpty())
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

QString AppController::resolveRootFolderForPath(const QString &folderPath) const
{
    QString bestMatch;

    for (const ConfiguredFolderEntry &entry : m_configuredFolders) {
        if (!isSameOrChildPath(entry.path, folderPath))
            continue;

        if (entry.path.size() > bestMatch.size())
            bestMatch = entry.path;
    }

    return bestMatch;
}

bool AppController::canShowConfiguredFoldersRoot() const
{
    return m_virtualRootNavigationEnabled && m_configuredFolders.size() > 1;
}

bool AppController::isAtConfiguredFolderRoot() const
{
    if (m_showingConfiguredFoldersRoot || m_videoFolder.isEmpty())
        return false;

    return m_videoFolder == m_rootVideoFolder && m_currentRootFromConfiguration;
}

int AppController::indexOfConfiguredFolder(const QString &folderPath) const
{
    for (int i = 0; i < m_configuredFolders.size(); ++i) {
        if (m_configuredFolders.at(i).path == folderPath)
            return i;
    }

    return -1;
}

QVector<VideoItem> AppController::configuredFolderRootItems() const
{
    QVector<VideoItem> items;
    items.reserve(m_configuredFolders.size());

    for (const ConfiguredFolderEntry &entry : m_configuredFolders) {
        VideoItem item;
        item.title = entry.name;
        item.filePath = entry.path;
        const QString folderCoverPath = entry.path + "/folder.jpg";
        item.coverPath = QFileInfo::exists(folderCoverPath) ? folderCoverPath : "";
        item.isFolder = true;
        item.isVirtualRootEntry = true;
        items.push_back(item);
    }

    return items;
}

void AppController::showConfiguredFoldersRoot()
{
    if (!canShowConfiguredFoldersRoot())
        return;

    const QVector<VideoItem> items = configuredFolderRootItems();
    const int previousIndex = m_currentIndex;
    const int configuredIndex = indexOfConfiguredFolder(m_rootVideoFolder);

    ++m_thumbnailGeneration;
    m_libraryModel->setItems(items);
    m_showingConfiguredFoldersRoot = true;

    const int count = m_libraryModel->rowCount();
    if (count <= 0) {
        m_currentIndex = 0;
    } else if (configuredIndex >= 0 && configuredIndex < count) {
        m_currentIndex = configuredIndex;
    } else {
        m_currentIndex = std::clamp(previousIndex, 0, count - 1);
    }

    if (m_currentIndex != previousIndex)
        emit currentIndexChanged();
}

QString AppController::currentBrowserFolderPath() const
{
    if (m_showingConfiguredFoldersRoot) {
        const auto item = m_libraryModel->itemAt(m_currentIndex);
        return item.isFolder ? item.filePath : QString();
    }

    return m_videoFolder;
}

bool AppController::currentSeriesEpisodeRequest(SeriesEpisodeRequest *request) const
{
    if (!m_libraryModel)
        return false;

    const auto item = m_libraryModel->itemAt(m_currentIndex);
    qDebug() << "[EpisodeInfo] Browser item"
             << "index=" << m_currentIndex
             << "title=" << item.title
             << "filePath=" << item.filePath
             << "isFolder=" << item.isFolder
             << "isDemo=" << item.isDemo
             << "isParentFolder=" << item.isParentFolder
             << "isVirtualRootEntry=" << item.isVirtualRootEntry;

    if (item.filePath.isEmpty() || item.isFolder || item.isDemo || item.isParentFolder || item.isVirtualRootEntry)
        return false;

    const QString title = item.title.isEmpty()
        ? QFileInfo(item.filePath).completeBaseName()
        : item.title;

    SeriesEpisodeRequest parsedRequest;
    if (!parseSeriesEpisodeTitle(
            title,
            &parsedRequest.seriesName,
            &parsedRequest.season,
            &parsedRequest.episode)) {
        qDebug() << "[EpisodeInfo] Kein Serienfolgen-Muster im Titel gefunden:" << title;
        return false;
    }

    const QString localFolderCoverPath = QFileInfo(item.filePath).absolutePath() + QStringLiteral("/folder.jpg");
    if (QFileInfo::exists(localFolderCoverPath)) {
        parsedRequest.coverSource = QUrl::fromLocalFile(localFolderCoverPath);
        qDebug() << "[EpisodeInfo] Lokales Ordnercover gefunden:" << localFolderCoverPath;
    } else {
        qDebug() << "[EpisodeInfo] Kein lokales Ordnercover gefunden:" << localFolderCoverPath;
    }

    qDebug() << "[EpisodeInfo] Titel geparst"
             << "sourceTitle=" << title
             << "seriesName=" << parsedRequest.seriesName
             << "season=" << parsedRequest.season
             << "episode=" << parsedRequest.episode
             << "episodeCode=" << seasonEpisodeCode(parsedRequest.season, parsedRequest.episode);

    if (request)
        *request = parsedRequest;

    return true;
}

void AppController::requestShowSuggestion(int requestId, const SeriesEpisodeRequest &episodeRequest)
{
    QUrl url;
    url.setScheme(QStringLiteral("https"));
    // url.setHost(QStringLiteral("websocket.bplaced.net"));
    url.setHost(QStringLiteral("s.to"));
    url.setPath(QStringLiteral("/api/search/suggest"));

    QUrlQuery query;
    query.addQueryItem(QStringLiteral("term"), episodeRequest.seriesName);
    url.setQuery(query);

    QNetworkRequest request(url);
    request.setRawHeader("Accept", "application/json");
    request.setRawHeader("User-Agent", "CoverFlowMP");
    request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);

    qDebug() << "[EpisodeInfo] GET suggest" << url.toString(QUrl::FullyEncoded);

    QNetworkReply *reply = m_episodeInfoNetworkManager.get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply, requestId, episodeRequest, episodePageUrl = url]() {
        reply->deleteLater();

        const QVariant httpStatusAttribute = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        const int httpStatus = httpStatusAttribute.isValid() ? httpStatusAttribute.toInt() : 0;
        const QByteArray responseBody = reply->readAll();

        qDebug() << "[EpisodeInfo] Suggest finished"
                 << "requestId=" << requestId
                 << "httpStatus=" << httpStatus
                 << "networkError=" << reply->error()
                 << "errorString=" << reply->errorString()
                 << "bytes=" << responseBody.size();
        qDebug() << "[EpisodeInfo] Suggest response preview:" << debugPreview(responseBody);

        if (requestId != m_browserEpisodeInfoRequestId) {
            qDebug() << "[EpisodeInfo] Veraltete Suggest-Antwort ignoriert" << requestId;
            return;
        }

        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << "[EpisodeInfo] Suggest-Netzwerkfehler:" << reply->errorString();
            setBrowserEpisodeInfoFailure();
            return;
        }
        if (httpStatus < 200 || httpStatus >= 300) {
            qDebug() << "[EpisodeInfo] Suggest-HTTP-Fehler:" << httpStatus;
            setBrowserEpisodeInfoFailure();
            return;
        }

        QJsonParseError parseError;
        const QJsonDocument document = QJsonDocument::fromJson(responseBody, &parseError);
        if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
            qDebug() << "[EpisodeInfo] Suggest-JSON ungueltig:" << parseError.errorString();
            setBrowserEpisodeInfoFailure();
            return;
        }

        const QJsonArray shows = document.object().value(QStringLiteral("shows")).toArray();
        qDebug() << "[EpisodeInfo] Suggest shows count=" << shows.size();
        if (shows.isEmpty() || !shows.first().isObject()) {
            qDebug() << "[EpisodeInfo] Keine Show in Suggest-Antwort gefunden.";
            setBrowserEpisodeInfoFailure();
            return;
        }

        const QJsonObject show = shows.first().toObject();
        const QString showName = show.value(QStringLiteral("name")).toString();
        const QString showUrl = show.value(QStringLiteral("url")).toString().trimmed();
        qDebug() << "[EpisodeInfo] Suggest shows[0]"
                 << "name=" << showName
                 << "url=" << showUrl;
        if (showUrl.isEmpty()) {
            setBrowserEpisodeInfoFailure();
            return;
        }

        requestEpisodeInfoPage(requestId, episodeRequest, showUrl);
    });
}

void AppController::requestEpisodeInfoPage(int requestId, const SeriesEpisodeRequest &episodeRequest, const QString &showUrl)
{
    QString showPath = showUrl.trimmed();
    if (showPath.startsWith(QStringLiteral("http://")) || showPath.startsWith(QStringLiteral("https://"))) {
        showPath = QUrl(showPath).path();
    }

    while (showPath.startsWith(QLatin1Char('/')))
        showPath.remove(0, 1);
    while (showPath.endsWith(QLatin1Char('/')))
        showPath.chop(1);

    if (showPath.isEmpty()) {
        qDebug() << "[EpisodeInfo] Leere Show-URL nach Normalisierung:" << showUrl;
        setBrowserEpisodeInfoFailure();
        return;
    }

    QUrl url;
    url.setScheme(QStringLiteral("https"));
    // url.setHost(QStringLiteral("websocket.bplaced.net"));
    url.setHost(QStringLiteral("s.to"));
    url.setPath(QStringLiteral("/") + showPath
                + QStringLiteral("/staffel-%1/episode-%2")
                      .arg(episodeRequest.season)
                      .arg(episodeRequest.episode));

    QNetworkRequest request(url);
    request.setRawHeader("Accept", "text/html,application/xhtml+xml");
    request.setRawHeader("User-Agent", "CoverFlowMP");
    request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);

    qDebug() << "[EpisodeInfo] GET episode page" << url.toString();

    QNetworkReply *reply = m_episodeInfoNetworkManager.get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply, requestId, episodeRequest, episodePageUrl = url]() {
        reply->deleteLater();

        const QVariant httpStatusAttribute = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        const int httpStatus = httpStatusAttribute.isValid() ? httpStatusAttribute.toInt() : 0;
        const QByteArray responseBody = reply->readAll();

        qDebug() << "[EpisodeInfo] Episode page finished"
                 << "requestId=" << requestId
                 << "httpStatus=" << httpStatus
                 << "networkError=" << reply->error()
                 << "errorString=" << reply->errorString()
                 << "bytes=" << responseBody.size();
        qDebug() << "[EpisodeInfo] Episode page response preview:" << debugPreview(responseBody);

        if (requestId != m_browserEpisodeInfoRequestId) {
            qDebug() << "[EpisodeInfo] Veraltete Episodenseite ignoriert" << requestId;
            return;
        }

        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << "[EpisodeInfo] Episodenseiten-Netzwerkfehler:" << reply->errorString();
            setBrowserEpisodeInfoFailure();
            return;
        }
        if (httpStatus < 200 || httpStatus >= 300) {
            qDebug() << "[EpisodeInfo] Episodenseiten-HTTP-Fehler:" << httpStatus;
            setBrowserEpisodeInfoFailure();
            return;
        }

        const QString html = QString::fromUtf8(responseBody);
        qDebug() << "[EpisodeInfo] Parse episode page"
                 << "h2Count=" << headingCount(html)
                 << "episodeCode=" << seasonEpisodeCode(episodeRequest.season, episodeRequest.episode);

        const QString title = extractEpisodeTitleFromHtml(html, episodeRequest.season, episodeRequest.episode);
        const QString description = extractEpisodeDescriptionFromHtml(html);
        const QUrl coverSource = episodeRequest.coverSource.isEmpty()
            ? extractEpisodeCoverSourceFromHtml(html, episodePageUrl)
            : episodeRequest.coverSource;
        qDebug() << "[EpisodeInfo] Parse result"
                 << "title=" << title
                 << "descriptionLength=" << description.size()
                 << "coverSource=" << coverSource.toString();

        if (title.isEmpty()) {
            setBrowserEpisodeInfoFailure();
            return;
        }

        setBrowserEpisodeInfoState(
            episodeRequest.seriesName,
            QStringLiteral("S.%1 E.%2").arg(episodeRequest.season).arg(episodeRequest.episode),
            title,
            description,
            coverSource,
            false);
    });
}

void AppController::setBrowserEpisodeInfoState(const QString &seriesTitle,
                                               const QString &seasonEpisode,
                                               const QString &title,
                                               const QString &description,
                                               const QUrl &coverSource,
                                               bool loading)
{
    if (m_browserEpisodeInfoSeriesTitle == seriesTitle
        && m_browserEpisodeInfoSeasonEpisode == seasonEpisode
        && m_browserEpisodeInfoTitle == title
        && m_browserEpisodeInfoDescription == description
        && m_browserEpisodeInfoCoverSource == coverSource
        && m_browserEpisodeInfoLoading == loading) {
        return;
    }

    m_browserEpisodeInfoSeriesTitle = seriesTitle;
    m_browserEpisodeInfoSeasonEpisode = seasonEpisode;
    m_browserEpisodeInfoTitle = title;
    m_browserEpisodeInfoDescription = description;
    m_browserEpisodeInfoCoverSource = coverSource;
    m_browserEpisodeInfoLoading = loading;
    emit browserEpisodeInfoChanged();
}

void AppController::setBrowserEpisodeInfoFailure()
{
    qDebug() << "[EpisodeInfo] Fehlerzustand wird in Sidebar angezeigt.";
    setBrowserEpisodeInfoState(
        m_browserEpisodeInfoSeriesTitle,
        m_browserEpisodeInfoSeasonEpisode,
        QStringLiteral("Episodeninfo konnte nicht abgerufen werden"),
        QString(),
        m_browserEpisodeInfoCoverSource,
        false);
}
