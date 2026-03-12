import QtQuick
import QtQuick.Controls
import QtQuick.Window

Item {
    id: root
    anchors.fill: parent
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
        transientParent: root.Window.window
        flags: Qt.Tool | Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint
        color: "transparent"

        x: root.Window.window ? root.Window.window.x : 0
        y: root.Window.window ? root.Window.window.y : 0
        width: root.width
        height: root.height

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
