import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: root
    visible: true
    visibility: Window.FullScreen
    width: 1280
    height: 720
    title: "CoverFlowMP"

    Image {
        id: browserBackground
        anchors.fill: parent
        source: appController.browserBackgroundSource
        fillMode: Image.PreserveAspectCrop

        onStatusChanged: {
            if (status === Image.Error && source !== appController.defaultBrowserBackgroundSource)
                source = appController.defaultBrowserBackgroundSource
        }
    }

    Loader {
        anchors.fill: parent
        source: appController.playerVisible
                ? "qrc:/qml/PlayerScreen.qml"
                : "qrc:/qml/BrowserScreen.qml"
    }
}
