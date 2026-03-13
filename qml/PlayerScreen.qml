import QtQuick
import QtQuick.Controls
import QtQuick.Window

Item {
    id: root
    anchors.fill: parent
    focus: true
    property int resumeChoiceIndex: 0
    property bool messageDialogVisible: false
    property bool audioDelayMode: false
    readonly property int audioDelayStepMs: 50
    readonly property int audioDelayMaxMs: 2000

    function pad2(value) {
        return value < 10 ? "0" + value : "" + value
    }

    function formatTime(seconds) {
        var total = Math.max(0, Math.floor(seconds))
        var h = Math.floor(total / 3600)
        var m = Math.floor((total % 3600) / 60)
        var s = total % 60
        if (h > 0)
            return h + ":" + pad2(m) + ":" + pad2(s)
        return m + ":" + pad2(s)
    }

    function clampAudioDelayMs(value) {
        return Math.max(-audioDelayMaxMs, Math.min(audioDelayMaxMs, value))
    }

    function adjustAudioDelay(deltaMs) {
        var currentMs = Math.round((playerController.audioDelay * 1000) / audioDelayStepMs) * audioDelayStepMs
        var nextMs = clampAudioDelayMs(currentMs + deltaMs)
        playerController.setAudioDelay(nextMs / 1000.0)
    }

    Component.onCompleted: {
        Window.window.visibility = Window.FullScreen
        forceActiveFocus()
        playerController.attachToWindow(Window.window)
    }

    onVisibleChanged: {
        if (visible) {
            forceActiveFocus()
        } else {
            audioDelayMode = false
        }
    }

    Connections {
        target: root.Window.window

        function onActiveChanged() {
            if (root.Window.window && root.Window.window.active)
                root.forceActiveFocus()
        }
    }

    Rectangle {
        anchors.fill: parent
        color: "black"
    }

    Window {
        id: pauseOverlayWindow
        visible: root.visible
                 && playerController.paused
                 && root.Window.window
                 && root.Window.window.active
        transientParent: root.Window.window
        flags: Qt.Tool | Qt.FramelessWindowHint | Qt.WindowDoesNotAcceptFocus
        color: "transparent"

        x: root.Window.window ? root.Window.window.x : 0
        y: root.Window.window ? root.Window.window.y : 0
        width: root.width
        height: root.height

        PauseOverlay {
            anchors.fill: parent
            position: playerController.position
            duration: playerController.duration
            videoName: appController.currentVideoName
            audioDelay: playerController.audioDelay
            audioDelayMode: root.audioDelayMode
            skipRanges: playerController.skipRanges
            skipRangePending: playerController.skipRangePending
            pendingSkipStart: playerController.pendingSkipStart
        }

        Rectangle {
            id: messageDialog
            visible: root.messageDialogVisible
            anchors.centerIn: parent
            width: parent.width * 0.42
            height: parent.height * 0.32
            radius: 14
            color: "#D91A1A1A"
            border.width: 1
            border.color: "#808080"
            clip: true

            Column {
                id: messageDialogContent
                anchors.fill: parent
                anchors.margins: messageDialog.height * 0.10
                spacing: messageDialog.height * 0.08

                Text {
                    width: messageDialogContent.width
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    color: "white"
                    wrapMode: Text.WordWrap
                    font.pixelSize: Math.max(20, messageDialog.height * 0.12)
                    font.bold: true
                    text: appController.playerMessage
                }

                Item {
                    width: messageDialogContent.width
                    height: messageDialog.height * 0.05
                }

                Rectangle {
                    width: messageDialogContent.width
                    height: Math.max(56, messageDialog.height * 0.26)
                    radius: 8
                    color: "#2AA84A"
                    border.width: 1
                    border.color: "#7CF1A3"

                    Text {
                        anchors.centerIn: parent
                        text: "OK"
                        color: "white"
                        font.bold: true
                        font.pixelSize: Math.max(22, messageDialog.height * 0.12)
                    }
                }
            }
        }
    }

    Rectangle {
        id: resumeDialog
        visible: appController.resumePromptVisible
        anchors.centerIn: parent
        width: parent.width * 0.42
        height: parent.height * 0.40
        radius: 14
        color: "#D91A1A1A"
        border.width: 1
        border.color: "#808080"
        clip: true

        Column {
            id: resumeDialogContent
            anchors.fill: parent
            anchors.margins: resumeDialog.height * 0.08
            spacing: resumeDialog.height * 0.06

            Text {
                width: resumeDialogContent.width
                horizontalAlignment: Text.AlignHCenter
                color: "white"
                wrapMode: Text.WordWrap
                font.pixelSize: Math.max(20, resumeDialog.height * 0.09)
                font.bold: true
                text: "Fortsetzen bei " + formatTime(appController.pendingResumePosition) + "?"
            }

            Item {
                width: resumeDialogContent.width
                height: resumeDialog.height * 0.03
            }

            Rectangle {
                width: resumeDialogContent.width
                height: Math.max(56, resumeDialog.height * 0.20)
                radius: 8
                color: root.resumeChoiceIndex === 0 ? "#2AA84A" : "#303030"
                border.width: 1
                border.color: root.resumeChoiceIndex === 0 ? "#7CF1A3" : "#666666"

                Text {
                    anchors.centerIn: parent
                    text: "Weiter"
                    color: "white"
                    font.bold: true
                    font.pixelSize: Math.max(22, resumeDialog.height * 0.10)
                }
            }

            Rectangle {
                width: resumeDialogContent.width
                height: Math.max(56, resumeDialog.height * 0.20)
                radius: 8
                color: root.resumeChoiceIndex === 1 ? "#2AA84A" : "#303030"
                border.width: 1
                border.color: root.resumeChoiceIndex === 1 ? "#7CF1A3" : "#666666"

                Text {
                    anchors.centerIn: parent
                    text: "Von vorne"
                    color: "white"
                    font.bold: true
                    font.pixelSize: Math.max(22, resumeDialog.height * 0.09)
                }
            }
        }
    }

    Connections {
        target: appController
        function onResumePromptVisibleChanged() {
            if (appController.resumePromptVisible)
                root.resumeChoiceIndex = 0
        }

        function onPlayerMessageChanged() {
            root.messageDialogVisible = appController.playerMessage !== ""
            root.forceActiveFocus()
        }
    }

    Connections {
        target: playerController
        function onPausedChanged() {
            if (!playerController.paused)
                root.audioDelayMode = false
        }
    }

    Keys.onSpacePressed: {
        if (appController.resumePromptVisible)
            return
        if (root.audioDelayMode)
            return
        playerController.togglePause()
    }

    Keys.onLeftPressed: function(event) {
        if (appController.resumePromptVisible) {
            event.accepted = true
            return
        }
        if (root.messageDialogVisible) {
            event.accepted = true
            return
        }
        if (root.audioDelayMode) {
            root.adjustAudioDelay(-root.audioDelayStepMs)
            event.accepted = true
            return
        }
        playerController.seekRelative(-10.0)
        event.accepted = true
    }

    Keys.onRightPressed: function(event) {
        if (appController.resumePromptVisible) {
            event.accepted = true
            return
        }
        if (root.messageDialogVisible) {
            event.accepted = true
            return
        }
        if (root.audioDelayMode) {
            root.adjustAudioDelay(root.audioDelayStepMs)
            event.accepted = true
            return
        }
        playerController.seekRelative(10.0)
        event.accepted = true
    }

    Keys.onUpPressed: function(event) {
        if (!appController.resumePromptVisible)
            return
        root.resumeChoiceIndex = 0
        event.accepted = true
    }

    Keys.onDownPressed: function(event) {
        if (!appController.resumePromptVisible)
            return
        root.resumeChoiceIndex = 1
        event.accepted = true
    }

    Keys.onReturnPressed: function(event) {
        if (root.messageDialogVisible) {
            appController.clearPlayerMessage()
            event.accepted = true
            return
        }
        if (!appController.resumePromptVisible)
            return
        appController.decideResumePlayback(root.resumeChoiceIndex === 0)
        event.accepted = true
    }

    Keys.onEnterPressed: function(event) {
        if (root.messageDialogVisible) {
            appController.clearPlayerMessage()
            event.accepted = true
            return
        }
        if (!appController.resumePromptVisible)
            return
        appController.decideResumePlayback(root.resumeChoiceIndex === 0)
        event.accepted = true
    }

    Keys.onPressed: function(event) {
        if (root.messageDialogVisible) {
            if (event.key === Qt.Key_B || event.key === Qt.Key_Escape || event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
                appController.clearPlayerMessage()
                event.accepted = true
            }
            return
        }

        if (appController.resumePromptVisible)
            return

        if (playerController.paused && !root.audioDelayMode && event.key === Qt.Key_Plus) {
            playerController.stepFrameForward()
            event.accepted = true
            return
        }

        if (playerController.paused && !root.audioDelayMode && event.key === Qt.Key_Minus) {
            playerController.stepFrameBackward()
            event.accepted = true
            return
        }

        if (playerController.paused && !root.audioDelayMode && event.key === Qt.Key_S) {
            playerController.markSkipBoundary()
            event.accepted = true
            return
        }

        if (playerController.paused && !root.audioDelayMode && event.key === Qt.Key_X) {
            appController.exportCurrentSkipRanges()
            event.accepted = true
            return
        }

        if (playerController.paused && !root.audioDelayMode && event.key === Qt.Key_I) {
            appController.importCurrentSkipRanges()
            event.accepted = true
            return
        }

        if (playerController.paused && !root.audioDelayMode
                && (event.key === Qt.Key_C || event.key === Qt.Key_Backspace)) {
            playerController.clearPendingSkipRange()
            event.accepted = true
            return
        }

        if (event.key === Qt.Key_A && playerController.paused && !appController.resumePromptVisible) {
            root.audioDelayMode = true
            event.accepted = true
            return
        }

        if (event.key === Qt.Key_Escape && root.audioDelayMode) {
            root.audioDelayMode = false
            event.accepted = true
            return
        }

        if (event.key === Qt.Key_B) {
            appController.backToBrowser()
            event.accepted = true
        }
    }

}
