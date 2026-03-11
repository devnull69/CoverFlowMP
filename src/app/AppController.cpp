#include "AppController.h"

#include "../media/VideoLibraryModel.h"
#include "../media/LibraryScanner.h"
#include "../player/PlayerController.h"
#include "../persistence/ResumeRepository.h"

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

bool AppController::playerVisible() const
{
    return m_playerVisible;
}

int AppController::currentIndex() const
{
    return m_currentIndex;
}

void AppController::initialize(const QString &videoFolder)
{
    auto items = m_scanner->scan(videoFolder);

    for (auto &item : items) {
        item.resumePosition = m_resumeRepository->loadPosition(item.filePath);
    }

    m_libraryModel->setItems(items);
}

void AppController::playSelected(int index)
{
    const auto item = m_libraryModel->itemAt(index);
    if (item.filePath.isEmpty())
        return;

    m_currentIndex = index;
    emit currentIndexChanged();

    m_currentFilePath = item.filePath;
    m_playerController->playFile(item.filePath, item.resumePosition);

    m_playerVisible = true;
    emit playerVisibleChanged();
}

void AppController::backToBrowser()
{
    const double pos = m_playerController->position();
    const double dur = m_playerController->duration();

    if (!m_currentFilePath.isEmpty()) {
        m_resumeRepository->savePosition(m_currentFilePath, pos, dur);
        m_libraryModel->updateResumePosition(m_currentFilePath, pos);
    }

    m_playerController->stop();

    m_playerVisible = false;
    emit playerVisibleChanged();
}

void AppController::setCurrentIndex(int index)
{
    if (m_currentIndex == index)
        return;
    m_currentIndex = index;
    emit currentIndexChanged();
}
