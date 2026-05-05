#include "AppConfig.h"

#include <QJsonObject>

namespace AppConfig {

QString browserBackgroundKey()
{
    return QStringLiteral("browser.background");
}

QString libraryFoldersKey()
{
    return QStringLiteral("library.folders");
}

QString episodeInfoHostKey()
{
    return QStringLiteral("episodeInfo.host");
}

QString episodeInfoLookupKeysKey()
{
    return QStringLiteral("episodeInfo.lookupKeys");
}

QString legacyPlaybackMergeCheckedKey()
{
    return QStringLiteral("migration.legacyPlaybackMergeChecked");
}

QString defaultBrowserBackground()
{
    return QStringLiteral("default");
}

QString defaultEpisodeInfoHost()
{
    return QStringLiteral("");
}

QJsonArray defaultLibraryFoldersArray()
{
    QJsonArray folders;
    QJsonObject folder;
    folder.insert(QStringLiteral("name"), QStringLiteral("Videos"));
    folder.insert(QStringLiteral("path"), QStringLiteral("~/Videos"));
    folders.append(folder);
    return folders;
}

QJsonArray defaultEpisodeInfoLookupKeysArray()
{
    return {};
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
