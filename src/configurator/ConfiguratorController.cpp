#include "ConfiguratorController.h"

#include "../shared/AppConfig.h"
#include "../shared/AppDatabase.h"

#include <QApplication>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QImageReader>
#include <QInputDialog>
#include <QJsonArray>
#include <QJsonObject>
#include <QMessageBox>
#include <algorithm>

namespace {

QString expandUserPath(const QString &path)
{
    if (path == QStringLiteral("~"))
        return QDir::homePath();
    if (path.startsWith(QStringLiteral("~/")))
        return QDir::homePath() + path.mid(1);
    return path;
}

QString baseFolderName(const QString &folderPath)
{
    const QFileInfo folderInfo(QDir::cleanPath(expandUserPath(folderPath)));
    const QString fileName = folderInfo.fileName().trimmed();
    return fileName.isEmpty() ? QStringLiteral("Ordner") : fileName;
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

QVariantList ConfiguratorController::libraryFolders() const
{
    QVariantList folders;
    folders.reserve(m_libraryFolders.size());

    for (const FolderEntry &entry : m_libraryFolders) {
        QVariantMap item;
        item.insert(QStringLiteral("name"), entry.name);
        item.insert(QStringLiteral("path"), entry.path);
        folders.append(item);
    }

    return folders;
}

int ConfiguratorController::selectedFolderIndex() const
{
    return m_selectedFolderIndex;
}

void ConfiguratorController::setSelectedFolderIndex(int index)
{
    const int normalizedIndex =
        (index >= 0 && index < m_libraryFolders.size()) ? index : -1;
    if (m_selectedFolderIndex == normalizedIndex)
        return;

    m_selectedFolderIndex = normalizedIndex;
    emit selectedFolderIndexChanged();
    emit folderActionsChanged();
}

bool ConfiguratorController::canDeleteFolderEntry() const
{
    return m_selectedFolderIndex >= 0 && m_selectedFolderIndex < m_libraryFolders.size();
}

bool ConfiguratorController::canMoveFolderEntryUp() const
{
    return canDeleteFolderEntry() && m_selectedFolderIndex > 0;
}

bool ConfiguratorController::canMoveFolderEntryDown() const
{
    return canDeleteFolderEntry() && m_selectedFolderIndex < m_libraryFolders.size() - 1;
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

void ConfiguratorController::addFolderEntry()
{
    const QString folderPath = QFileDialog::getExistingDirectory(
        nullptr,
        QStringLiteral("Ordner auswaehlen"),
        folderDialogStartPath(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (folderPath.isEmpty())
        return;

    const QString suggestedName = uniqueFolderNameForPath(folderPath);
    bool accepted = false;
    const QString enteredName = QInputDialog::getText(
        nullptr,
        QStringLiteral("Ordnernamen eingeben"),
        QStringLiteral("Name"),
        QLineEdit::Normal,
        suggestedName,
        &accepted).trimmed();

    if (!accepted)
        return;

    if (enteredName.isEmpty()) {
        QMessageBox::warning(nullptr,
                             QStringLiteral("Konfigurator"),
                             QStringLiteral("Bitte einen gueltigen Namen fuer den Ordner angeben."));
        return;
    }

    for (const FolderEntry &entry : m_libraryFolders) {
        if (entry.name.compare(enteredName, Qt::CaseInsensitive) == 0) {
            QMessageBox::warning(nullptr,
                                 QStringLiteral("Konfigurator"),
                                 QStringLiteral("Ein Ordner mit diesem Namen existiert bereits."));
            return;
        }
    }

    m_libraryFolders.append({
        enteredName,
        QDir::toNativeSeparators(QDir(folderPath).absolutePath())
    });
    emit libraryFoldersChanged();
    setSelectedFolderIndex(m_libraryFolders.size() - 1);
}

void ConfiguratorController::removeSelectedFolderEntry()
{
    if (!canDeleteFolderEntry())
        return;

    m_libraryFolders.removeAt(m_selectedFolderIndex);
    emit libraryFoldersChanged();

    if (m_libraryFolders.isEmpty()) {
        setSelectedFolderIndex(-1);
    } else {
        setSelectedFolderIndex(
            std::min(m_selectedFolderIndex, static_cast<int>(m_libraryFolders.size()) - 1));
    }
}

void ConfiguratorController::moveSelectedFolderEntryUp()
{
    if (!canMoveFolderEntryUp())
        return;

    m_libraryFolders.swapItemsAt(m_selectedFolderIndex, m_selectedFolderIndex - 1);
    emit libraryFoldersChanged();
    setSelectedFolderIndex(m_selectedFolderIndex - 1);
}

void ConfiguratorController::moveSelectedFolderEntryDown()
{
    if (!canMoveFolderEntryDown())
        return;

    m_libraryFolders.swapItemsAt(m_selectedFolderIndex, m_selectedFolderIndex + 1);
    emit libraryFoldersChanged();
    setSelectedFolderIndex(m_selectedFolderIndex + 1);
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

    QJsonArray libraryFoldersArray;
    for (const FolderEntry &entry : m_libraryFolders) {
        QJsonObject folderObject;
        folderObject.insert(QStringLiteral("name"), entry.name);
        folderObject.insert(QStringLiteral("path"), entry.path);
        libraryFoldersArray.append(folderObject);
    }

    if (!m_database->saveConfigArray(AppConfig::libraryFoldersKey(), libraryFoldersArray)) {
        QMessageBox::critical(nullptr,
                              QStringLiteral("Konfigurator"),
                              QStringLiteral("Die Ordner-Konfiguration konnte nicht gespeichert werden."));
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
    const QJsonArray configuredFolders = m_database->loadConfigArray(
        AppConfig::libraryFoldersKey(),
        AppConfig::defaultLibraryFoldersArray());

    m_useDefaultBackground =
        configuredBackground.isEmpty()
        || configuredBackground == AppConfig::defaultBrowserBackground();
    m_backgroundPath = m_useDefaultBackground ? QString() : configuredBackground;

    m_libraryFolders.clear();
    for (const QJsonValue &entry : configuredFolders) {
        if (!entry.isObject())
            continue;

        const QJsonObject folderObject = entry.toObject();
        const QString name = folderObject.value(QStringLiteral("name")).toString().trimmed();
        const QString path = folderObject.value(QStringLiteral("path")).toString().trimmed();
        if (name.isEmpty() || path.isEmpty())
            continue;

        m_libraryFolders.append({ name, path });
    }

    if (m_libraryFolders.isEmpty()) {
        const QJsonArray fallbackFolders = AppConfig::defaultLibraryFoldersArray();
        for (const QJsonValue &entry : fallbackFolders) {
            if (!entry.isObject())
                continue;

            const QJsonObject folderObject = entry.toObject();
            const QString name = folderObject.value(QStringLiteral("name")).toString();
            const QString path = folderObject.value(QStringLiteral("path")).toString();
            if (!name.isEmpty() && !path.isEmpty())
                m_libraryFolders.append({ name, path });
        }
    }

    m_selectedFolderIndex = m_libraryFolders.isEmpty() ? -1 : 0;
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

QString ConfiguratorController::folderDialogStartPath() const
{
    if (canDeleteFolderEntry()) {
        const QString selectedPath = m_libraryFolders.at(m_selectedFolderIndex).path;
        const QString resolvedPath = QFileInfo(expandUserPath(selectedPath)).absoluteFilePath();
        if (QDir(resolvedPath).exists())
            return resolvedPath;
    }

    return QDir::homePath();
}

QString ConfiguratorController::uniqueFolderNameForPath(const QString &folderPath) const
{
    const QString baseName = baseFolderName(folderPath);
    QString candidate = baseName;
    int suffix = 2;

    auto nameExists = [this](const QString &name) {
        for (const FolderEntry &entry : m_libraryFolders) {
            if (entry.name.compare(name, Qt::CaseInsensitive) == 0)
                return true;
        }
        return false;
    };

    while (nameExists(candidate)) {
        candidate = QStringLiteral("%1 %2").arg(baseName).arg(suffix);
        ++suffix;
    }

    return candidate;
}

bool ConfiguratorController::isValidImageFile(const QString &path) const
{
    QFileInfo fileInfo(path);
    if (!fileInfo.exists() || !fileInfo.isFile())
        return false;

    QImageReader reader(path);
    return reader.canRead();
}
