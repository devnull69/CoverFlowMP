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
        anchors.fill: parent
        source: "qrc:/assets/wallpaper.jpg"
        fillMode: Image.PreserveAspectCrop
    }

    Item {
        id: keyCatcher
        anchors.fill: parent
        focus: true

        Keys.onEscapePressed: Qt.quit()
    }

    Loader {
        anchors.fill: parent
        source: appController.playerVisible
                ? "qrc:/qml/PlayerScreen.qml"
                : "qrc:/qml/BrowserScreen.qml"
    }
}
