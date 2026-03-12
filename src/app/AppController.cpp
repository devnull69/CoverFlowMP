#include "AppController.h"

#include "../media/VideoLibraryModel.h"
#include "../media/LibraryScanner.h"
#include "../player/PlayerController.h"
#include "../persistence/ResumeRepository.h"
#include <QCursor>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QMetaObject>
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
    m_resumeRepository(resumeRepository)
{
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
    QMetaObject::invokeMethod(this, [this, fileToPlay, startPosition, audioDelay]() {
        if (!m_playerVisible || fileToPlay.isEmpty() || m_currentFilePath != fileToPlay)
            return;
        m_playerController->playFile(fileToPlay, startPosition, audioDelay);
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
    auto items = m_scanner->scan(videoFolder);

    for (auto &item : items) {
        if (!item.isFolder && !item.filePath.isEmpty())
            item.resumePosition = m_resumeRepository->loadPosition(item.filePath);
    }

    m_libraryModel->setItems(items);

    const int count = m_libraryModel->rowCount();
    const int previousIndex = m_currentIndex;
    if (count <= 0)
        m_currentIndex = 0;
    else
        m_currentIndex = std::clamp(m_currentIndex, 0, count - 1);

    if (m_currentIndex != previousIndex)
        emit currentIndexChanged();
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
    if (item.filePath.isEmpty() || item.isFolder)
        return false;

    m_resumeRepository->deletePosition(item.filePath);

    if (!QFile::remove(item.filePath))
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

bool AppController::canDeleteCurrentVideo() const
{
    const auto item = m_libraryModel->itemAt(m_currentIndex);
    return !item.filePath.isEmpty() && !item.isFolder;
}

bool AppController::resetResumeDatabase()
{
    if (!m_resumeRepository->clearAllPositions())
        return false;

    initialize(m_videoFolder);
    return true;
}

void AppController::decideResumePlayback(bool continueFromSavedPosition)
{
    if (!m_resumePromptVisible)
        return;

    const double startPosition = continueFromSavedPosition ? m_pendingResumePosition : 0.0;
    startPlayback(startPosition);
}

void AppController::backToBrowser()
{
    if (!m_resumePromptVisible) {
        const double pos = m_playerController->position();
        const double dur = m_playerController->duration();
        double savePos = std::max(0.0, pos);
        if (dur > 0.0)
            savePos = std::min(savePos, dur);

        if (!m_currentFilePath.isEmpty()) {
            m_resumeRepository->savePosition(
                m_currentFilePath,
                savePos,
                dur,
                m_playerController->audioDelay());
            m_libraryModel->updateResumePosition(m_currentFilePath, savePos);
        }
    }

    m_playerController->stop();
    setPlayerCursorHidden(false);

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

    m_currentFilePath.clear();
    m_currentAudioDelay = 0.0;
    if (!m_currentVideoName.isEmpty()) {
        m_currentVideoName.clear();
        emit currentVideoNameChanged();
    }
}

void AppController::setCurrentIndex(int index)
{
    if (m_currentIndex == index)
        return;
    m_currentIndex = index;
    emit currentIndexChanged();
}
