#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDir>

#include "src/app/AppController.h"
#include "src/media/VideoLibraryModel.h"
#include "src/media/LibraryScanner.h"
#include "src/media/ThumbnailService.h"
#include "src/player/PlayerController.h"
#include "src/persistence/ResumeRepository.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    ResumeRepository resumeRepository;
    resumeRepository.initialize();

    ThumbnailService thumbnailService;
    LibraryScanner scanner(&thumbnailService);

    VideoLibraryModel libraryModel;
    PlayerController playerController;
    AppController appController(&libraryModel, &scanner, &playerController, &resumeRepository);

    // Für den Anfang hart verdrahtet; später über Settings/Dialog
    const QString videoFolder = QDir::homePath() + "/Videos";
    appController.initialize(videoFolder);

    QQmlApplicationEngine engine;

    engine.rootContext()->setContextProperty("appController", &appController);
    engine.rootContext()->setContextProperty("libraryModel", &libraryModel);
    engine.rootContext()->setContextProperty("playerController", &playerController);

    const QUrl url(u"qrc:/qml/Main.qml"_qs);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
                     &app, []() { QCoreApplication::exit(-1); },
                     Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
