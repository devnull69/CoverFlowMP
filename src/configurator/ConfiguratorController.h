#pragma once

#include <QObject>
#include <QVariantList>
#include <QVector>

class AppDatabase;

class ConfiguratorController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int currentSection READ currentSection WRITE setCurrentSection NOTIFY currentSectionChanged)
    Q_PROPERTY(bool useDefaultBackground READ useDefaultBackground WRITE setUseDefaultBackground NOTIFY useDefaultBackgroundChanged)
    Q_PROPERTY(QString backgroundPath READ backgroundPath WRITE setBackgroundPath NOTIFY backgroundPathChanged)
    Q_PROPERTY(bool backgroundPathValid READ backgroundPathValid NOTIFY backgroundValidationChanged)
    Q_PROPERTY(QString backgroundValidationMessage READ backgroundValidationMessage NOTIFY backgroundValidationChanged)
    Q_PROPERTY(QVariantList libraryFolders READ libraryFolders NOTIFY libraryFoldersChanged)
    Q_PROPERTY(int selectedFolderIndex READ selectedFolderIndex WRITE setSelectedFolderIndex NOTIFY selectedFolderIndexChanged)
    Q_PROPERTY(bool canDeleteFolderEntry READ canDeleteFolderEntry NOTIFY folderActionsChanged)
    Q_PROPERTY(bool canMoveFolderEntryUp READ canMoveFolderEntryUp NOTIFY folderActionsChanged)
    Q_PROPERTY(bool canMoveFolderEntryDown READ canMoveFolderEntryDown NOTIFY folderActionsChanged)
    Q_PROPERTY(QString episodeInfoHost READ episodeInfoHost WRITE setEpisodeInfoHost NOTIFY episodeInfoHostChanged)
    Q_PROPERTY(QVariantList episodeLookupEntries READ episodeLookupEntries NOTIFY episodeLookupEntriesChanged)
    Q_PROPERTY(int selectedEpisodeLookupIndex READ selectedEpisodeLookupIndex WRITE setSelectedEpisodeLookupIndex NOTIFY selectedEpisodeLookupIndexChanged)
    Q_PROPERTY(bool canDeleteEpisodeLookupEntry READ canDeleteEpisodeLookupEntry NOTIFY episodeLookupActionsChanged)
    Q_PROPERTY(bool canSave READ canSave NOTIFY canSaveChanged)

public:
    explicit ConfiguratorController(AppDatabase *database, QObject *parent = nullptr);

    int currentSection() const;
    void setCurrentSection(int section);

    bool useDefaultBackground() const;
    void setUseDefaultBackground(bool useDefault);

    QString backgroundPath() const;
    void setBackgroundPath(const QString &path);

    bool backgroundPathValid() const;
    QString backgroundValidationMessage() const;
    QVariantList libraryFolders() const;
    int selectedFolderIndex() const;
    void setSelectedFolderIndex(int index);
    bool canDeleteFolderEntry() const;
    bool canMoveFolderEntryUp() const;
    bool canMoveFolderEntryDown() const;
    QString episodeInfoHost() const;
    void setEpisodeInfoHost(const QString &host);
    QVariantList episodeLookupEntries() const;
    int selectedEpisodeLookupIndex() const;
    void setSelectedEpisodeLookupIndex(int index);
    bool canDeleteEpisodeLookupEntry() const;
    bool canSave() const;

    Q_INVOKABLE void browseBackgroundImage();
    Q_INVOKABLE void addFolderEntry();
    Q_INVOKABLE void removeSelectedFolderEntry();
    Q_INVOKABLE void moveSelectedFolderEntryUp();
    Q_INVOKABLE void moveSelectedFolderEntryDown();
    Q_INVOKABLE void addEpisodeLookupEntry();
    Q_INVOKABLE void removeSelectedEpisodeLookupEntry();
    Q_INVOKABLE void saveAndQuit();
    Q_INVOKABLE void cancelAndQuit();

signals:
    void currentSectionChanged();
    void useDefaultBackgroundChanged();
    void backgroundPathChanged();
    void backgroundValidationChanged();
    void libraryFoldersChanged();
    void selectedFolderIndexChanged();
    void folderActionsChanged();
    void episodeInfoHostChanged();
    void episodeLookupEntriesChanged();
    void selectedEpisodeLookupIndexChanged();
    void episodeLookupActionsChanged();
    void canSaveChanged();

private:
    struct FolderEntry {
        QString name;
        QString path;
    };

    struct EpisodeLookupEntry {
        QString seriesFileName;
        QString lookupKey;
    };

    void loadSettings();
    void updateBackgroundValidation();
    QString folderDialogStartPath() const;
    QString uniqueFolderNameForPath(const QString &folderPath) const;
    QString resolvedBackgroundPath() const;
    bool isValidImageFile(const QString &path) const;
    bool episodeSeriesFileNameExists(const QString &seriesFileName) const;
    QString normalizedEpisodeInfoHost() const;

    AppDatabase *m_database;
    int m_currentSection = 0;
    bool m_useDefaultBackground = true;
    QString m_backgroundPath;
    bool m_backgroundPathValid = true;
    QString m_backgroundValidationMessage;
    QVector<FolderEntry> m_libraryFolders;
    int m_selectedFolderIndex = -1;
    QString m_episodeInfoHost;
    QVector<EpisodeLookupEntry> m_episodeLookupEntries;
    int m_selectedEpisodeLookupIndex = -1;
};
