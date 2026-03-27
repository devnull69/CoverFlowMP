#include <QApplication>
#include <QIcon>
#include <QMessageBox>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "../shared/AppDatabase.h"
#include "../shared/AppLaunchGuard.h"
#include "ConfiguratorController.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("CoverFlowMPConfigurator"));
    app.setWindowIcon(QIcon(QStringLiteral(":/assets/configurator-icon.svg")));

    AppLaunchGuard launchGuard(QStringLiteral("CoverFlowMP Konfigurator"));
    if (!launchGuard.tryAcquire()) {
        QMessageBox::critical(nullptr,
                              QStringLiteral("CoverFlowMP Konfigurator"),
                              launchGuard.failureMessage());
        return 1;
    }

    AppDatabase database;
    if (!database.initialize()) {
        QMessageBox::critical(nullptr,
                              QStringLiteral("CoverFlowMP Konfigurator"),
                              QStringLiteral("Die gemeinsame Datenbank konnte nicht geoeffnet werden."));
        return 1;
    }

    ConfiguratorController controller(&database);

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("configController"), &controller);

    const QUrl url(QStringLiteral("qrc:/qml/configurator/Main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
                     &app, []() { QApplication::exit(-1); },
                     Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
