import QtQuick
import QtQuick.Controls
import QtQuick.Window

Item {
    id: root
    focus: true

    Component.onCompleted: {
        Window.window.visibility = Window.FullScreen
        forceActiveFocus()
        playerController.attachToWindow(Window.window)
    }

    onVisibleChanged: {
        if (visible)
            forceActiveFocus()
    }

    Rectangle {
        anchors.fill: parent
        color: "black"
    }

    Window {
        id: pauseOverlayWindow
        visible: root.visible && playerController.paused
        transientParent: Window.window
        flags: Qt.Tool | Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint
        color: "transparent"

        x: Window.window ? Window.window.x : 0
        y: Window.window ? Window.window.y : 0
        width: Window.window ? Window.window.width : 0
        height: Window.window ? Window.window.height : 0

        PauseOverlay {
            anchors.fill: parent
            position: playerController.position
            duration: playerController.duration
        }
    }

    Keys.onSpacePressed: {
        playerController.togglePause()
    }

    Keys.onPressed: function(event) {
        if (event.key === Qt.Key_B) {
            appController.backToBrowser()
            event.accepted = true
        }
    }
}
