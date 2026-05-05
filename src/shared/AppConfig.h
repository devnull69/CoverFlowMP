#pragma once

#include <QJsonArray>
#include <QString>
#include <QStringList>

namespace AppConfig {

enum class ValueType {
    String,
    Json
};

QString browserBackgroundKey();
QString libraryFoldersKey();
QString episodeInfoHostKey();
QString episodeInfoLookupKeysKey();
QString legacyPlaybackMergeCheckedKey();

QString defaultBrowserBackground();
QString defaultEpisodeInfoHost();
QJsonArray defaultLibraryFoldersArray();
QJsonArray defaultEpisodeInfoLookupKeysArray();
QStringList defaultLibraryFolders();

QString valueTypeName(ValueType type);

} // namespace AppConfig
