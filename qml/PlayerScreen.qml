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
    property bool infoMode: false
    property bool clearSkipDialogVisible: false
    property int clearSkipChoiceIndex: 1 // 0 = JA, 1 = NEIN
    property int skipImportChoiceIndex: 0 // 0 = JA, 1 = NEIN
    property int selectedSkipRangeIndex: -1
    property bool nextEpisodeButtonVisible: false
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

    function updateSubtitlePosition() {
        playerController.setSubtitlesRaised(playerController.paused || root.audioDelayMode)
    }

    function seekBy(seconds) {
        if (playerController.paused)
            playerController.seekRelativeClamped(seconds)
        else
            playerController.seekRelative(seconds)
    }

    function selectSkipRange(direction) {
        var count = playerController.skipRanges.length
        if (count <= 0) {
            root.selectedSkipRangeIndex = -1
            return
        }

        if (root.selectedSkipRangeIndex < 0
                || root.selectedSkipRangeIndex >= count) {
            root.selectedSkipRangeIndex = direction < 0 ? count - 1 : 0
            return
        }

        root.selectedSkipRangeIndex = Math.max(
                    0,
                    Math.min(count - 1, root.selectedSkipRangeIndex + direction))
    }

    function canShowNextEpisodeButton() {
        return appController.playerNextEpisodeAvailable
                && !playerController.paused
                && !root.infoMode
                && !root.audioDelayMode
                && !root.messageDialogVisible
                && !root.clearSkipDialogVisible
                && !appController.resumePromptVisible
                && !appController.skipImportPromptVisible
    }

    function hideNextEpisodeButton() {
        root.nextEpisodeButtonVisible = false
        nextEpisodeButtonTimer.stop()
    }

    function showNextEpisodeButton() {
        if (!root.canShowNextEpisodeButton())
            return false

        root.nextEpisodeButtonVisible = true
        nextEpisodeButtonTimer.restart()
        return true
    }

    function playNextEpisodeFromButton() {
        root.hideNextEpisodeButton()
        appController.playNextEpisode()
    }

    function confirmNextEpisodeButton(event) {
        if (!root.nextEpisodeButtonVisible || !root.canShowNextEpisodeButton())
            return false

        root.playNextEpisodeFromButton()
        event.accepted = true
        return true
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
            infoMode = false
            clearSkipDialogVisible = false
            nextEpisodeButtonVisible = false
            skipImportChoiceIndex = 0
            selectedSkipRangeIndex = -1
        }
    }

    onAudioDelayModeChanged: updateSubtitlePosition()

    Connections {
        target: root.Window.window

        function onActiveChanged() {
            if (root.Window.window && root.Window.window.active)
                root.forceActiveFocus()
        }
    }

    Timer {
        id: nextEpisodeButtonTimer
        interval: 5000
        repeat: false
        onTriggered: root.nextEpisodeButtonVisible = false
    }

    Rectangle {
        anchors.fill: parent
        color: "black"
    }

    Window {
        id: nextEpisodeOverlayWindow
        visible: root.visible
                 && root.nextEpisodeButtonVisible
                 && root.canShowNextEpisodeButton()
                 && root.Window.window
                 && root.Window.window.active
        transientParent: root.Window.window
        flags: Qt.Tool | Qt.FramelessWindowHint | Qt.WindowDoesNotAcceptFocus
        color: "transparent"

        x: root.Window.window ? root.Window.window.x : 0
        y: root.Window.window ? root.Window.window.y : 0
        width: root.width
        height: root.height

        readonly property int buttonMargin: Math.max(28, height * 0.055)

        PlaybackActionButton {
            id: nextEpisodeButton
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.rightMargin: nextEpisodeOverlayWindow.buttonMargin
            anchors.bottomMargin: nextEpisodeOverlayWindow.buttonMargin
            width: Math.min(parent.width - nextEpisodeOverlayWindow.buttonMargin * 2,
                            Math.max(320, parent.width * 0.30))
            height: Math.max(42, Math.min(58, root.height * 0.058))
            text: "Nächste Folge abspielen"
            iconName: "playNext"
            onClicked: root.playNextEpisodeFromButton()
        }
    }

    Window {
        id: pauseOverlayWindow
        visible: root.visible
                 && (playerController.paused || root.infoMode)
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
            skipRanges: appController.fastMode ? [] : playerController.skipRanges
            selectedSkipRangeIndex: root.selectedSkipRangeIndex
            skipRangePending: appController.fastMode ? false : playerController.skipRangePending
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
        Rectangle {
            id: clearSkipDialog
            visible: root.clearSkipDialogVisible
            anchors.centerIn: parent
            width: parent.width * 0.42
            height: parent.height * 0.40
            radius: 14
            color: "#D91A1A1A"
            border.width: 1
            border.color: "#808080"
            clip: true

            Column {
                id: clearSkipDialogContent
                anchors.fill: parent
                anchors.margins: clearSkipDialog.height * 0.08
                spacing: clearSkipDialog.height * 0.06

                Text {
                    width: clearSkipDialogContent.width
                    horizontalAlignment: Text.AlignHCenter
                    color: "white"
                    wrapMode: Text.WordWrap
                    font.pixelSize: Math.max(20, clearSkipDialog.height * 0.09)
                    font.bold: true
                    text: "Skip-Bereiche dieses Videos loeschen?"
                }

                Item {
                    width: clearSkipDialogContent.width
                    height: clearSkipDialog.height * 0.03
                }

                Rectangle {
                    width: clearSkipDialogContent.width
                    height: Math.max(56, clearSkipDialog.height * 0.20)
                    radius: 8
                    color: root.clearSkipChoiceIndex === 0 ? "#2AA84A" : "#303030"
                    border.width: 1
                    border.color: root.clearSkipChoiceIndex === 0 ? "#7CF1A3" : "#666666"

                    Text {
                        anchors.centerIn: parent
                        text: "JA"
                        color: "white"
                        font.bold: true
                        font.pixelSize: Math.max(22, clearSkipDialog.height * 0.10)
                    }
                }

                Rectangle {
                    width: clearSkipDialogContent.width
                    height: Math.max(56, clearSkipDialog.height * 0.20)
                    radius: 8
                    color: root.clearSkipChoiceIndex === 1 ? "#2AA84A" : "#303030"
                    border.width: 1
                    border.color: root.clearSkipChoiceIndex === 1 ? "#7CF1A3" : "#666666"

                    Text {
                        anchors.centerIn: parent
                        text: "NEIN"
                        color: "white"
                        font.bold: true
                        font.pixelSize: Math.max(22, clearSkipDialog.height * 0.09)
                    }
                }
            }
        }

        Rectangle {
            id: skipImportDialog
            visible: appController.skipImportPromptVisible
            anchors.centerIn: parent
            width: parent.width * 0.42
            height: parent.height * 0.40
            radius: 14
            color: "#D91A1A1A"
            border.width: 1
            border.color: "#808080"
            clip: true

            Column {
                id: skipImportDialogContent
                anchors.fill: parent
                anchors.margins: skipImportDialog.height * 0.08
                spacing: skipImportDialog.height * 0.06

                Text {
                    width: skipImportDialogContent.width
                    horizontalAlignment: Text.AlignHCenter
                    color: "white"
                    wrapMode: Text.WordWrap
                    font.pixelSize: Math.max(20, skipImportDialog.height * 0.09)
                    font.bold: true
                    text: "Passende Skip-Datei gefunden. Soll sie importiert werden?"
                }

                Item {
                    width: skipImportDialogContent.width
                    height: skipImportDialog.height * 0.03
                }

                Rectangle {
                    width: skipImportDialogContent.width
                    height: Math.max(56, skipImportDialog.height * 0.20)
                    radius: 8
                    color: root.skipImportChoiceIndex === 0 ? "#2AA84A" : "#303030"
                    border.width: 1
                    border.color: root.skipImportChoiceIndex === 0 ? "#7CF1A3" : "#666666"

                    Text {
                        anchors.centerIn: parent
                        text: "JA"
                        color: "white"
                        font.bold: true
                        font.pixelSize: Math.max(22, skipImportDialog.height * 0.10)
                    }
                }

                Rectangle {
                    width: skipImportDialogContent.width
                    height: Math.max(56, skipImportDialog.height * 0.20)
                    radius: 8
                    color: root.skipImportChoiceIndex === 1 ? "#2AA84A" : "#303030"
                    border.width: 1
                    border.color: root.skipImportChoiceIndex === 1 ? "#7CF1A3" : "#666666"

                    Text {
                        anchors.centerIn: parent
                        text: "NEIN"
                        color: "white"
                        font.bold: true
                        font.pixelSize: Math.max(22, skipImportDialog.height * 0.09)
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
            root.hideNextEpisodeButton()
            if (appController.resumePromptVisible)
                root.resumeChoiceIndex = 0
        }

        function onSkipImportPromptVisibleChanged() {
            root.hideNextEpisodeButton()
            if (appController.skipImportPromptVisible)
                root.skipImportChoiceIndex = 0
        }

        function onPlayerNextEpisodeChanged() {
            if (!appController.playerNextEpisodeAvailable)
                root.hideNextEpisodeButton()
        }

        function onPlayerMessageChanged() {
            root.hideNextEpisodeButton()
            root.messageDialogVisible = appController.playerMessage !== ""
            root.forceActiveFocus()
        }

        function onFastModeChanged() {
            root.selectedSkipRangeIndex = -1
        }
    }

    Connections {
        target: playerController
        function onPausedChanged() {
            root.hideNextEpisodeButton()
            root.updateSubtitlePosition()
            if (playerController.paused) {
                root.infoMode = false
            } else {
                root.audioDelayMode = false
                root.selectedSkipRangeIndex = -1
            }
        }

        function onSkipRangesChanged() {
            root.selectedSkipRangeIndex = -1
        }
    }

    Keys.onSpacePressed: {
        if (appController.resumePromptVisible)
            return
        if (appController.skipImportPromptVisible)
            return
        if (root.audioDelayMode)
            return
        if (root.infoMode)
            root.infoMode = false
        playerController.togglePause()
    }

    Keys.onLeftPressed: function(event) {
        if (appController.resumePromptVisible) {
            event.accepted = true
            return
        }
        if (appController.skipImportPromptVisible) {
            event.accepted = true
            return
        }
        if (root.clearSkipDialogVisible) {
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
        root.seekBy((event.modifiers & Qt.ShiftModifier) ? -5.0 : -10.0)
        event.accepted = true
    }

    Keys.onRightPressed: function(event) {
        if (appController.resumePromptVisible) {
            event.accepted = true
            return
        }
        if (appController.skipImportPromptVisible) {
            event.accepted = true
            return
        }
        if (root.clearSkipDialogVisible) {
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
        root.seekBy((event.modifiers & Qt.ShiftModifier) ? 5.0 : 10.0)
        event.accepted = true
    }

    Keys.onUpPressed: function(event) {
        if (appController.resumePromptVisible) {
            root.resumeChoiceIndex = 0
            event.accepted = true
            return
        }
        if (appController.skipImportPromptVisible) {
            root.skipImportChoiceIndex = 0
            event.accepted = true
            return
        }
        if (root.clearSkipDialogVisible) {
            root.clearSkipChoiceIndex = 0
            event.accepted = true
            return
        }
        if (root.messageDialogVisible) {
            event.accepted = true
            return
        }
        if (root.audioDelayMode) {
            event.accepted = true
            return
        }
        root.seekBy(60.0)
        event.accepted = true
    }

    Keys.onDownPressed: function(event) {
        if (appController.resumePromptVisible) {
            root.resumeChoiceIndex = 1
            event.accepted = true
            return
        }
        if (appController.skipImportPromptVisible) {
            root.skipImportChoiceIndex = 1
            event.accepted = true
            return
        }
        if (root.clearSkipDialogVisible) {
            root.clearSkipChoiceIndex = 1
            event.accepted = true
            return
        }
        if (root.messageDialogVisible) {
            event.accepted = true
            return
        }
        if (root.audioDelayMode) {
            event.accepted = true
            return
        }
        root.seekBy(-60.0)
        event.accepted = true
    }

    Keys.onReturnPressed: function(event) {
        if (root.confirmNextEpisodeButton(event))
            return

        if (root.infoMode) {
            event.accepted = true
            return
        }
        if (root.messageDialogVisible) {
            appController.clearPlayerMessage()
            event.accepted = true
            return
        }
        if (appController.skipImportPromptVisible) {
            appController.respondToSkipImportPrompt(root.skipImportChoiceIndex === 0)
            event.accepted = true
            return
        }
        if (root.clearSkipDialogVisible) {
            if (root.clearSkipChoiceIndex === 0)
                appController.clearCurrentSkipRanges()
            root.clearSkipDialogVisible = false
            event.accepted = true
            return
        }
        if (!appController.resumePromptVisible)
            return
        appController.decideResumePlayback(root.resumeChoiceIndex === 0)
        event.accepted = true
    }

    Keys.onEnterPressed: function(event) {
        if (root.confirmNextEpisodeButton(event))
            return

        if (root.infoMode) {
            event.accepted = true
            return
        }
        if (root.messageDialogVisible) {
            appController.clearPlayerMessage()
            event.accepted = true
            return
        }
        if (appController.skipImportPromptVisible) {
            appController.respondToSkipImportPrompt(root.skipImportChoiceIndex === 0)
            event.accepted = true
            return
        }
        if (root.clearSkipDialogVisible) {
            if (root.clearSkipChoiceIndex === 0)
                appController.clearCurrentSkipRanges()
            root.clearSkipDialogVisible = false
            event.accepted = true
            return
        }
        if (!appController.resumePromptVisible)
            return
        appController.decideResumePlayback(root.resumeChoiceIndex === 0)
        event.accepted = true
    }

    Keys.onPressed: function(event) {
        if (event.key === Qt.Key_Control) {
            if (root.showNextEpisodeButton())
                event.accepted = true
            return
        }

        if (event.key === Qt.Key_U && !root.infoMode
                && !appController.resumePromptVisible && !appController.skipImportPromptVisible
                && !root.clearSkipDialogVisible && !root.messageDialogVisible) {
            playerController.disableSubtitles()
            event.accepted = true
            return
        }

        if (event.key === Qt.Key_Y && !playerController.paused && !root.audioDelayMode
                && !appController.resumePromptVisible && !appController.skipImportPromptVisible
                && !root.clearSkipDialogVisible && !root.messageDialogVisible) {
            root.hideNextEpisodeButton()
            root.infoMode = !root.infoMode
            event.accepted = true
            return
        }

        if (root.infoMode) {
            if (event.key === Qt.Key_B) {
                appController.backToBrowser()
                event.accepted = true
                return
            }
            if (event.key !== Qt.Key_Space
                    && event.key !== Qt.Key_Y
                    && event.key !== Qt.Key_B)
                event.accepted = true
            return
        }

        if (root.messageDialogVisible) {
            if (event.key === Qt.Key_B || event.key === Qt.Key_Escape || event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
                appController.clearPlayerMessage()
                event.accepted = true
            }
            return
        }

        if (root.clearSkipDialogVisible) {
            if (event.key === Qt.Key_Escape || event.key === Qt.Key_B) {
                root.clearSkipDialogVisible = false
                event.accepted = true
            }
            return
        }

        if (appController.skipImportPromptVisible) {
            if (event.key === Qt.Key_Escape || event.key === Qt.Key_B) {
                appController.respondToSkipImportPrompt(false)
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

        if (playerController.paused && !root.audioDelayMode && event.key === Qt.Key_E) {
            playerController.jumpToLastFrame()
            event.accepted = true
            return
        }

        if (playerController.paused && !root.audioDelayMode && !appController.fastMode && event.key === Qt.Key_S) {
            playerController.markSkipBoundary()
            event.accepted = true
            return
        }

        if (playerController.paused && !root.audioDelayMode && !appController.fastMode && event.key === Qt.Key_X) {
            appController.exportCurrentSkipRanges()
            event.accepted = true
            return
        }

        if (playerController.paused && !root.audioDelayMode && !appController.fastMode && event.key === Qt.Key_I) {
            appController.importCurrentSkipRanges()
            event.accepted = true
            return
        }

        if (playerController.paused && !root.audioDelayMode && !appController.fastMode
                && event.key === Qt.Key_C) {
            playerController.clearPendingSkipRange()
            event.accepted = true
            return
        }

        if (playerController.paused && !root.audioDelayMode && !appController.fastMode
                && (event.key === Qt.Key_Comma || event.key === Qt.Key_Period)) {
            root.selectSkipRange(event.key === Qt.Key_Comma ? -1 : 1)
            event.accepted = true
            return
        }

        if (playerController.paused && !root.audioDelayMode && !appController.fastMode
                && event.key === Qt.Key_Delete && root.selectedSkipRangeIndex >= 0) {
            playerController.deleteSkipRange(root.selectedSkipRangeIndex)
            event.accepted = true
            return
        }

        if (playerController.paused && !root.audioDelayMode && !appController.fastMode
                && (event.key === Qt.Key_Delete || event.key === Qt.Key_Backspace)) {
            root.clearSkipChoiceIndex = 1
            root.clearSkipDialogVisible = true
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
