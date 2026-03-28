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
QString legacyPlaybackMergeCheckedKey();

QString defaultBrowserBackground();
QJsonArray defaultLibraryFoldersArray();
QStringList defaultLibraryFolders();

QString valueTypeName(ValueType type);

} // namespace AppConfig
