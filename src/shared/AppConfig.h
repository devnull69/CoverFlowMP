#pragma once

#include <QJsonObject>
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
QJsonObject defaultLibraryFoldersObject();
QStringList defaultLibraryFolders();

QString valueTypeName(ValueType type);

} // namespace AppConfig
