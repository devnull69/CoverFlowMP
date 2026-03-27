#include "AppPaths.h"

#include <QDir>
#include <QStandardPaths>

namespace AppPaths {

QString sharedDataDirectory()
{
    const QString baseDirectory =
        QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    const QString appDirectory = baseDirectory + QStringLiteral("/coverflowmp");
    QDir().mkpath(appDirectory);
    return appDirectory;
}

QString databasePath()
{
    return sharedDataDirectory() + QStringLiteral("/playback.sqlite");
}

QString launchLockFilePath()
{
    return sharedDataDirectory() + QStringLiteral("/coverflowmp.lock");
}

} // namespace AppPaths
