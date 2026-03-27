#pragma once

#include <QObject>

class AppDatabase;

class ConfiguratorController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int currentSection READ currentSection WRITE setCurrentSection NOTIFY currentSectionChanged)
    Q_PROPERTY(bool useDefaultBackground READ useDefaultBackground WRITE setUseDefaultBackground NOTIFY useDefaultBackgroundChanged)
    Q_PROPERTY(QString backgroundPath READ backgroundPath WRITE setBackgroundPath NOTIFY backgroundPathChanged)
    Q_PROPERTY(bool backgroundPathValid READ backgroundPathValid NOTIFY backgroundValidationChanged)
    Q_PROPERTY(QString backgroundValidationMessage READ backgroundValidationMessage NOTIFY backgroundValidationChanged)
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
    bool canSave() const;

    Q_INVOKABLE void browseBackgroundImage();
    Q_INVOKABLE void saveAndQuit();
    Q_INVOKABLE void cancelAndQuit();

signals:
    void currentSectionChanged();
    void useDefaultBackgroundChanged();
    void backgroundPathChanged();
    void backgroundValidationChanged();
    void canSaveChanged();

private:
    void loadSettings();
    void updateBackgroundValidation();
    QString resolvedBackgroundPath() const;
    bool isValidImageFile(const QString &path) const;

    AppDatabase *m_database;
    int m_currentSection = 0;
    bool m_useDefaultBackground = true;
    QString m_backgroundPath;
    bool m_backgroundPathValid = true;
    QString m_backgroundValidationMessage;
};
