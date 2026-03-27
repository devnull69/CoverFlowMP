#pragma once

#include <QString>

class QLockFile;

class AppLaunchGuard
{
public:
    explicit AppLaunchGuard(const QString &applicationLabel);
    ~AppLaunchGuard();

    bool tryAcquire();
    QString failureMessage() const;

private:
    QString m_applicationLabel;
    QLockFile *m_lockFile = nullptr;
};
