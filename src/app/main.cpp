#include <QApplication>
#include <QCommandLineParser>
#include <QDir>
#include <QIcon>
#include <QJsonArray>
#include <QJsonObject>
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

    const AppDatabase::LegacyMergeResult legacyMergeResult = database.mergeLegacyDatabaseOnce();
    switch (legacyMergeResult.status) {
    case AppDatabase::LegacyMergeResult::Status::Merged: {
        QString message;
        if (legacyMergeResult.mergedPlaybackRows > 0 || legacyMergeResult.mergedSkipFiles > 0) {
            message =
                QStringLiteral("Es wurde eine alte Wiedergabe-Datenbank gefunden und einmalig uebernommen.\n\n"
                               "Resume-Eintraege: %1\n"
                               "Skip-Dateien: %2")
                    .arg(legacyMergeResult.mergedPlaybackRows)
                    .arg(legacyMergeResult.mergedSkipFiles);
        } else {
            message =
                QStringLiteral("Es wurde eine alte Wiedergabe-Datenbank gefunden, aber es waren keine uebernehmbaren Daten vorhanden.");
        }

        QMessageBox::information(nullptr,
                                 QStringLiteral("CoverFlowMP"),
                                 message);
        break;
    }
    case AppDatabase::LegacyMergeResult::Status::Failed: {
        const QString details = legacyMergeResult.errorMessage.isEmpty()
            ? QStringLiteral("Unbekannter Fehler.")
            : legacyMergeResult.errorMessage;
        QMessageBox::warning(nullptr,
                             QStringLiteral("CoverFlowMP"),
                             QStringLiteral("Die alte Wiedergabe-Datenbank konnte nicht automatisch uebernommen werden.\n\n%1")
                                 .arg(details));
        break;
    }
    case AppDatabase::LegacyMergeResult::Status::AlreadyChecked:
    case AppDatabase::LegacyMergeResult::Status::NoLegacyDatabase:
        break;
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

    const QJsonArray configuredFolders = database.loadConfigArray(
        AppConfig::libraryFoldersKey(),
        AppConfig::defaultLibraryFoldersArray());
    QString videoFolder;
    for (const QJsonValue &entry : configuredFolders) {
        if (!entry.isObject())
            continue;

        const QString candidateFolder = normalizedFolderPath(
            entry.toObject().value(QStringLiteral("path")).toString());
        if (isUsableFolder(candidateFolder)) {
            videoFolder = candidateFolder;
            break;
        }
    }

    QString fallbackFolderValue = QStringLiteral("~/Videos");
    const QJsonArray fallbackFolders = AppConfig::defaultLibraryFoldersArray();
    if (!fallbackFolders.isEmpty() && fallbackFolders.first().isObject()) {
        fallbackFolderValue = fallbackFolders.first().toObject()
                                  .value(QStringLiteral("path"))
                                  .toString(fallbackFolderValue);
    }
    const QString fallbackVideoFolder =
        normalizedFolderPath(fallbackFolderValue);
    if (!isUsableFolder(videoFolder))
        videoFolder = fallbackVideoFolder;

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
