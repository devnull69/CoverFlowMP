#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDir>
#include <QCommandLineParser>
#include <QIcon>
#include <clocale>
#include <QtGlobal>

#include "src/app/AppController.h"
#include "src/media/VideoLibraryModel.h"
#include "src/media/LibraryScanner.h"
#include "src/media/ThumbnailService.h"
#include "src/player/PlayerController.h"
#include "src/persistence/ResumeRepository.h"

int main(int argc, char *argv[])
{
    // mpv wid embedding is X11-based; enforce xcb when running from a Wayland session.
    if (qEnvironmentVariableIsSet("WAYLAND_DISPLAY") &&
        qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM")) {
        qputenv("QT_QPA_PLATFORM", QByteArray("xcb"));
    }

    QGuiApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/assets/appicon.png"));
    
    std::setlocale(LC_NUMERIC, "C");

    ResumeRepository resumeRepository;
    resumeRepository.initialize();

    ThumbnailService thumbnailService;
    LibraryScanner scanner(&thumbnailService);

    VideoLibraryModel libraryModel;
    PlayerController playerController;
    AppController appController(&libraryModel, &scanner, &playerController, &resumeRepository);

    QCommandLineParser parser;
    parser.setApplicationDescription("CoverFlowMP");
    parser.addHelpOption();
    parser.addPositionalArgument("video_root", "Optionaler Root-Ordner fuer die Videobibliothek.");
    parser.process(app);

    QString videoFolder = QDir::homePath() + "/Videos";
    const QStringList positionalArgs = parser.positionalArguments();
    if (!positionalArgs.isEmpty()) {
        videoFolder = QDir(positionalArgs.first()).absolutePath();
    }

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
