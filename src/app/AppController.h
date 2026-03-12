#pragma once

#include <QObject>
#include <QString>

class VideoLibraryModel;
class LibraryScanner;
class PlayerController;
class ResumeRepository;

class AppController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool playerVisible READ playerVisible NOTIFY playerVisibleChanged)
    Q_PROPERTY(int currentIndex READ currentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(QString currentVideoName READ currentVideoName NOTIFY currentVideoNameChanged)

public:
    explicit AppController(VideoLibraryModel *libraryModel,
                           LibraryScanner *scanner,
                           PlayerController *playerController,
                           ResumeRepository *resumeRepository,
                           QObject *parent = nullptr);

    bool playerVisible() const;
    int currentIndex() const;
    QString currentVideoName() const;

    Q_INVOKABLE void initialize(const QString &videoFolder);
    Q_INVOKABLE void playSelected(int index);
    Q_INVOKABLE void backToBrowser();
    Q_INVOKABLE void setCurrentIndex(int index);

signals:
    void playerVisibleChanged();
    void currentIndexChanged();
    void currentVideoNameChanged();

private:
    void setPlayerCursorHidden(bool hidden);

    VideoLibraryModel *m_libraryModel;
    LibraryScanner *m_scanner;
    PlayerController *m_playerController;
    ResumeRepository *m_resumeRepository;

    bool m_playerVisible = false;
    bool m_playerCursorHidden = false;
    int m_currentIndex = 0;
    QString m_currentFilePath;
    QString m_currentVideoName;
};
