#include "ConfiguratorController.h"

#include "../shared/AppConfig.h"
#include "../shared/AppDatabase.h"

#include <QApplication>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QImageReader>
#include <QMessageBox>

namespace {

QString expandUserPath(const QString &path)
{
    if (path == QStringLiteral("~"))
        return QDir::homePath();
    if (path.startsWith(QStringLiteral("~/")))
        return QDir::homePath() + path.mid(1);
    return path;
}

}

ConfiguratorController::ConfiguratorController(AppDatabase *database, QObject *parent)
    : QObject(parent),
    m_database(database)
{
    loadSettings();
}

int ConfiguratorController::currentSection() const
{
    return m_currentSection;
}

void ConfiguratorController::setCurrentSection(int section)
{
    if (m_currentSection == section)
        return;

    m_currentSection = section;
    emit currentSectionChanged();
}

bool ConfiguratorController::useDefaultBackground() const
{
    return m_useDefaultBackground;
}

void ConfiguratorController::setUseDefaultBackground(bool useDefault)
{
    if (m_useDefaultBackground == useDefault)
        return;

    m_useDefaultBackground = useDefault;
    updateBackgroundValidation();
    emit useDefaultBackgroundChanged();
    emit canSaveChanged();
}

QString ConfiguratorController::backgroundPath() const
{
    return m_backgroundPath;
}

void ConfiguratorController::setBackgroundPath(const QString &path)
{
    if (m_backgroundPath == path)
        return;

    m_backgroundPath = path;
    updateBackgroundValidation();
    emit backgroundPathChanged();
    emit canSaveChanged();
}

bool ConfiguratorController::backgroundPathValid() const
{
    return m_backgroundPathValid;
}

QString ConfiguratorController::backgroundValidationMessage() const
{
    return m_backgroundValidationMessage;
}

bool ConfiguratorController::canSave() const
{
    return m_useDefaultBackground || m_backgroundPathValid;
}

void ConfiguratorController::browseBackgroundImage()
{
    const QString startPath = m_backgroundPath.isEmpty()
        ? QDir::homePath()
        : QFileInfo(resolvedBackgroundPath()).absolutePath();
    const QString filePath = QFileDialog::getOpenFileName(
        nullptr,
        QStringLiteral("Hintergrundbild auswaehlen"),
        startPath,
        QStringLiteral("Bilder (*.png *.jpg *.jpeg *.bmp *.gif *.webp);;Alle Dateien (*)"));

    if (!filePath.isEmpty())
        setBackgroundPath(filePath);
}

void ConfiguratorController::saveAndQuit()
{
    if (!canSave()) {
        QMessageBox::warning(nullptr,
                             QStringLiteral("Konfigurator"),
                             QStringLiteral("Bitte zuerst einen gueltigen Bildpfad angeben."));
        return;
    }

    const QString value = m_useDefaultBackground
        ? AppConfig::defaultBrowserBackground()
        : QFileInfo(resolvedBackgroundPath()).absoluteFilePath();

    if (!m_database->saveConfigString(AppConfig::browserBackgroundKey(), value)) {
        QMessageBox::critical(nullptr,
                              QStringLiteral("Konfigurator"),
                              QStringLiteral("Die Konfiguration konnte nicht gespeichert werden."));
        return;
    }

    QApplication::quit();
}

void ConfiguratorController::cancelAndQuit()
{
    QApplication::quit();
}

void ConfiguratorController::loadSettings()
{
    const QString configuredBackground = m_database->loadConfigString(
        AppConfig::browserBackgroundKey(),
        AppConfig::defaultBrowserBackground());

    m_useDefaultBackground =
        configuredBackground.isEmpty()
        || configuredBackground == AppConfig::defaultBrowserBackground();
    m_backgroundPath = m_useDefaultBackground ? QString() : configuredBackground;
    updateBackgroundValidation();
}

void ConfiguratorController::updateBackgroundValidation()
{
    const bool previousValid = m_backgroundPathValid;
    const QString previousMessage = m_backgroundValidationMessage;

    if (m_useDefaultBackground) {
        m_backgroundPathValid = true;
        m_backgroundValidationMessage = QStringLiteral("Es wird das integrierte Standard-Hintergrundbild verwendet.");
    } else {
        const QString resolvedPath = resolvedBackgroundPath();
        if (resolvedPath.isEmpty()) {
            m_backgroundPathValid = false;
            m_backgroundValidationMessage = QStringLiteral("Bitte einen Dateipfad angeben.");
        } else if (!isValidImageFile(resolvedPath)) {
            m_backgroundPathValid = false;
            m_backgroundValidationMessage = QStringLiteral("Die angegebene Datei ist kein gueltiges Bild.");
        } else {
            m_backgroundPathValid = true;
            m_backgroundValidationMessage = QStringLiteral("Die Bilddatei ist gueltig.");
        }
    }

    if (previousValid != m_backgroundPathValid
        || previousMessage != m_backgroundValidationMessage) {
        emit backgroundValidationChanged();
    }
}

QString ConfiguratorController::resolvedBackgroundPath() const
{
    const QString trimmedPath = m_backgroundPath.trimmed();
    if (trimmedPath.isEmpty())
        return {};

    return QFileInfo(expandUserPath(trimmedPath)).absoluteFilePath();
}

bool ConfiguratorController::isValidImageFile(const QString &path) const
{
    QFileInfo fileInfo(path);
    if (!fileInfo.exists() || !fileInfo.isFile())
        return false;

    QImageReader reader(path);
    return reader.canRead();
}
