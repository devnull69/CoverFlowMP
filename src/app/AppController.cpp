#include "AppController.h"

#include "../media/VideoLibraryModel.h"
#include "../media/LibraryScanner.h"
#include "../player/PlayerController.h"
#include "../persistence/ResumeRepository.h"
#include <QMetaObject>

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

    m_playerVisible = true;
    emit playerVisibleChanged();

    m_currentFilePath = item.filePath;
    const QString fileToPlay = item.filePath;
    QMetaObject::invokeMethod(this, [this, fileToPlay]() {
        if (!m_playerVisible)
            return;
        m_playerController->playFile(fileToPlay, 0.0);
    }, Qt::QueuedConnection);
}

void AppController::backToBrowser()
{
    m_playerController->stop();

    m_playerVisible = false;
    emit playerVisibleChanged();

    m_currentFilePath.clear();
}

void AppController::setCurrentIndex(int index)
{
    if (m_currentIndex == index)
        return;
    m_currentIndex = index;
    emit currentIndexChanged();
}
