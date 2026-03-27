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

QString defaultBrowserBackground()
{
    return QStringLiteral("default");
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
