#include <QApplication>
#include <QCommandLineParser>
#include <QDir>
#include <QIcon>
#include <QMessageBox>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <clocale>
#include <QtGlobal>

#include "../media/LibraryScanner.h"
#include "../media/ThumbnailService.h"
#include "../media/VideoLibraryModel.h"
#include "../platform/SessionInhibitor.h"
#include "../player/PlayerController.h"
#include "../shared/AppConfig.h"
#include "../shared/AppDatabase.h"
#include "../shared/AppLaunchGuard.h"
#include "AppController.h"

namespace {

QString expandUserPath(const QString &path)
{
    if (path == QStringLiteral("~"))
        return QDir::homePath();
    if (path.startsWith(QStringLiteral("~/")))
        return QDir::homePath() + path.mid(1);
    return path;
}

QString normalizedFolderPath(const QString &path)
{
    return QDir(expandUserPath(path)).absolutePath();
}

bool isUsableFolder(const QString &path)
{
    return !path.isEmpty() && QDir(path).exists();
}

}

int main(int argc, char *argv[])
{
    if (qEnvironmentVariableIsSet("WAYLAND_DISPLAY")
        && qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM")) {
        qputenv("QT_QPA_PLATFORM", QByteArray("xcb"));
    }

    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("CoverFlowMPApp"));
    app.setWindowIcon(QIcon(QStringLiteral(":/assets/appicon.png")));

    std::setlocale(LC_NUMERIC, "C");

    AppLaunchGuard launchGuard(QStringLiteral("CoverFlowMP"));
    if (!launchGuard.tryAcquire()) {
        QMessageBox::critical(nullptr,
                              QStringLiteral("CoverFlowMP"),
                              launchGuard.failureMessage());
        return 1;
    }

    AppDatabase database;
    if (!database.initialize()) {
        QMessageBox::critical(nullptr,
                              QStringLiteral("CoverFlowMP"),
                              QStringLiteral("Die gemeinsame Datenbank konnte nicht geoeffnet werden."));
        return 1;
    }

    ThumbnailService thumbnailService;
    LibraryScanner scanner(&thumbnailService);

    VideoLibraryModel libraryModel;
    PlayerController playerController;
    AppController appController(&libraryModel, &scanner, &playerController, &database);
    SessionInhibitor sessionInhibitor;

    QObject::connect(&appController, &AppController::playerVisibleChanged, &app, [&]() {
        sessionInhibitor.setVideoPlaybackActive(appController.playerVisible());
    });
    sessionInhibitor.setVideoPlaybackActive(appController.playerVisible());

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("CoverFlowMP"));
    parser.addHelpOption();
    parser.addPositionalArgument(QStringLiteral("video_root"),
                                 QStringLiteral("Optionaler Root-Ordner fuer die Videobibliothek."));
    parser.process(app);

    const QStringList configuredFolders = database.loadConfigStringList(
        AppConfig::libraryFoldersKey(),
        AppConfig::defaultLibraryFolders());
    const QString configuredDefaultFolder = configuredFolders.isEmpty()
        ? QString()
        : normalizedFolderPath(configuredFolders.first());
    const QString fallbackVideoFolder =
        normalizedFolderPath(AppConfig::defaultLibraryFolders().value(0));
    QString videoFolder = isUsableFolder(configuredDefaultFolder)
        ? configuredDefaultFolder
        : fallbackVideoFolder;

    const QStringList positionalArgs = parser.positionalArguments();
    if (!positionalArgs.isEmpty()) {
        const QString cliFolder = QDir(positionalArgs.first()).absolutePath();
        if (isUsableFolder(cliFolder))
            videoFolder = cliFolder;
    }

    appController.initialize(videoFolder);

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("appController"), &appController);
    engine.rootContext()->setContextProperty(QStringLiteral("libraryModel"), &libraryModel);
    engine.rootContext()->setContextProperty(QStringLiteral("playerController"), &playerController);

    const QUrl url(QStringLiteral("qrc:/qml/app/Main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
                     &app, []() { QApplication::exit(-1); },
                     Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
