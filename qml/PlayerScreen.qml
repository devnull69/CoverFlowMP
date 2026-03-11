import QtQuick
import QtQuick.Controls

Item {
    focus: true

    Rectangle {
        anchors.fill: parent
        color: "black"
    }

    Text {
        anchors.centerIn: parent
        text: "Hier kommt später die mpv-Videoausgabe hinein"
        color: "white"
    }

    PauseOverlay {
        anchors.fill: parent
        visible: playerController.paused
        position: playerController.position
        duration: playerController.duration
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
