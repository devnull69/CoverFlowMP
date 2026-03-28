#include "AppConfig.h"

namespace AppConfig {

QString browserBackgroundKey()
{
    return QStringLiteral("browser.background");
}

QString libraryFoldersKey()
{
    return QStringLiteral("library.folders");
}

QString legacyPlaybackMergeCheckedKey()
{
    return QStringLiteral("migration.legacyPlaybackMergeChecked");
}

QString defaultBrowserBackground()
{
    return QStringLiteral("default");
}

QJsonObject defaultLibraryFoldersObject()
{
    QJsonObject folders;
    folders.insert(QStringLiteral("Videos"), QStringLiteral("~/Videos"));
    return folders;
}

QStringList defaultLibraryFolders()
{
    return { QStringLiteral("~/Videos") };
}

QString valueTypeName(ValueType type)
{
    switch (type) {
    case ValueType::String:
        return QStringLiteral("string");
    case ValueType::Json:
        return QStringLiteral("json");
    }

    return QStringLiteral("string");
}

} // namespace AppConfig
