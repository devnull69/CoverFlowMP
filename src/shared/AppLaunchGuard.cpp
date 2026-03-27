#include "AppLaunchGuard.h"

#include "AppPaths.h"

#include <QLockFile>

AppLaunchGuard::AppLaunchGuard(const QString &applicationLabel)
    : m_applicationLabel(applicationLabel)
{
}

AppLaunchGuard::~AppLaunchGuard()
{
    delete m_lockFile;
}

bool AppLaunchGuard::tryAcquire()
{
    if (!m_lockFile) {
        m_lockFile = new QLockFile(AppPaths::launchLockFilePath());
        m_lockFile->setStaleLockTime(0);
    }

    return m_lockFile->tryLock();
}

QString AppLaunchGuard::failureMessage() const
{
    return m_applicationLabel
           + QStringLiteral(" kann nicht gestartet werden, weil bereits eine andere CoverFlowMP-Anwendung laeuft.");
}
