import QtQuick
import QtQuick.Controls

Item {
    id: root
    anchors.fill: parent
    focus: true
    property bool deleteDialogVisible: false
    property int deleteChoiceIndex: 1 // 0 = JA, 1 = NEIN (default)
    property bool resetDialogVisible: false
    property int resetChoiceIndex: 2 // 0 = ALLE, 1 = NUR AKTUELLER ORDNER, 2 = ABBRECHEN
    property bool actionDialogVisible: false
    property int actionChoiceIndex: 0 // 0 = BEENDEN, 1 = ZURUECK
    property bool messageDialogVisible: false
    property string messageDialogText: ""
    property bool episodeInfoSidebarVisible: false
    property date currentDateTime: new Date()

    function hideEpisodeInfoSidebar() {
        if (!episodeInfoSidebarVisible && appController.browserEpisodeInfoTitle === "")
            return

        episodeInfoSidebarVisible = false
        appController.clearBrowserEpisodeInfo()
    }

    function pad2(value) {
        return value < 10 ? "0" + value : "" + value
    }

    function weekdayName(value) {
        var weekdays = [
            "Sonntag",
            "Montag",
            "Dienstag",
            "Mittwoch",
            "Donnerstag",
            "Freitag",
            "Samstag"
        ]
        return weekdays[value]
    }

    function monthName(value) {
        var months = [
            "Januar",
            "Februar",
            "März",
            "April",
            "Mai",
            "Juni",
            "Juli",
            "August",
            "September",
            "Oktober",
            "November",
            "Dezember"
        ]
        return months[value]
    }

    function formattedTime() {
        return pad2(currentDateTime.getHours()) + ":" + pad2(currentDateTime.getMinutes())
    }

    function formattedDate() {
        return weekdayName(currentDateTime.getDay())
                + " "
                + monthName(currentDateTime.getMonth())
                + " "
                + currentDateTime.getDate()
    }

    Component.onCompleted: {
        forceActiveFocus()
    }

    Timer {
        interval: 1000
        repeat: true
        running: true
        triggeredOnStart: true
        onTriggered: root.currentDateTime = new Date()
    }

    Keys.onEscapePressed: function(event) {
        if (root.deleteDialogVisible || root.resetDialogVisible || root.actionDialogVisible || root.messageDialogVisible || root.episodeInfoSidebarVisible) {
            root.deleteDialogVisible = false
            root.resetDialogVisible = false
            root.actionDialogVisible = false
            root.messageDialogVisible = false
            root.messageDialogText = ""
            root.hideEpisodeInfoSidebar()
            appController.clearPlayerMessage()
            event.accepted = true
            return
        }

        appController.navigateUpOrQuit()
        event.accepted = true
    }

    Keys.onLeftPressed: function(event) {
        if (root.deleteDialogVisible || root.resetDialogVisible || root.actionDialogVisible || root.messageDialogVisible || root.episodeInfoSidebarVisible) {
            event.accepted = true
            return
        }
        coverFlow.moveLeft()
        event.accepted = true
    }

    Keys.onRightPressed: function(event) {
        if (root.deleteDialogVisible || root.resetDialogVisible || root.actionDialogVisible || root.messageDialogVisible || root.episodeInfoSidebarVisible) {
            event.accepted = true
            return
        }
        coverFlow.moveRight()
        event.accepted = true
    }

    Keys.onReturnPressed: function(event) {
        if (root.deleteDialogVisible) {
            if (root.deleteChoiceIndex === 0)
                appController.deleteCurrentVideo()
            root.deleteDialogVisible = false
            event.accepted = true
            return
        }
        if (root.resetDialogVisible) {
            if (root.resetChoiceIndex === 0)
                appController.resetResumeDatabase()
            else if (root.resetChoiceIndex === 1)
                appController.resetCurrentFolderResumeDatabase()
            root.resetDialogVisible = false
            event.accepted = true
            return
        }
        if (root.actionDialogVisible) {
            if (root.actionChoiceIndex === 0)
                appController.quitApplication()
            root.actionDialogVisible = false
            event.accepted = true
            return
        }
        if (root.messageDialogVisible) {
            root.messageDialogText = ""
            appController.clearPlayerMessage()
            root.messageDialogVisible = false
            event.accepted = true
            return
        }
        if (root.episodeInfoSidebarVisible) {
            root.hideEpisodeInfoSidebar()
            event.accepted = true
            return
        }
        appController.playSelected(appController.currentIndex)
        event.accepted = true
    }

    Keys.onEnterPressed: function(event) {
        if (root.deleteDialogVisible) {
            if (root.deleteChoiceIndex === 0)
                appController.deleteCurrentVideo()
            root.deleteDialogVisible = false
            event.accepted = true
            return
        }
        if (root.resetDialogVisible) {
            if (root.resetChoiceIndex === 0)
                appController.resetResumeDatabase()
            else if (root.resetChoiceIndex === 1)
                appController.resetCurrentFolderResumeDatabase()
            root.resetDialogVisible = false
            event.accepted = true
            return
        }
        if (root.actionDialogVisible) {
            if (root.actionChoiceIndex === 0)
                appController.quitApplication()
            root.actionDialogVisible = false
            event.accepted = true
            return
        }
        if (root.messageDialogVisible) {
            root.messageDialogText = ""
            appController.clearPlayerMessage()
            root.messageDialogVisible = false
            event.accepted = true
            return
        }
        if (root.episodeInfoSidebarVisible) {
            root.hideEpisodeInfoSidebar()
            event.accepted = true
            return
        }
        appController.playSelected(appController.currentIndex)
        event.accepted = true
    }

    Keys.onUpPressed: function(event) {
        if (root.messageDialogVisible || root.episodeInfoSidebarVisible) {
            event.accepted = true
            return
        }
        if (root.actionDialogVisible) {
            root.actionChoiceIndex = 0
            event.accepted = true
            return
        }
        if (root.deleteDialogVisible) {
            root.deleteChoiceIndex = 0
            event.accepted = true
            return
        }
        if (root.resetDialogVisible) {
            root.resetChoiceIndex = Math.max(0, root.resetChoiceIndex - 1)
            event.accepted = true
            return
        }
        if (appController.canOpenBrowserActionDialog()) {
            root.actionChoiceIndex = 0
            root.actionDialogVisible = true
            event.accepted = true
            return
        }
    }

    Keys.onDownPressed: function(event) {
        if (root.messageDialogVisible || root.episodeInfoSidebarVisible) {
            event.accepted = true
            return
        }
        if (root.actionDialogVisible) {
            root.actionChoiceIndex = 1
            event.accepted = true
            return
        }
        if (root.deleteDialogVisible) {
            root.deleteChoiceIndex = 1
            event.accepted = true
            return
        }
        if (root.resetDialogVisible) {
            root.resetChoiceIndex = Math.min(2, root.resetChoiceIndex + 1)
            event.accepted = true
            return
        }
    }

    Keys.onPressed: function(event) {
        if (event.key === Qt.Key_0
                && !root.deleteDialogVisible
                && !root.resetDialogVisible
                && !root.actionDialogVisible
                && !root.messageDialogVisible
                && !root.episodeInfoSidebarVisible
                && appController.canDeleteCurrentVideo()) {
            root.deleteChoiceIndex = 1
            root.deleteDialogVisible = true
            event.accepted = true
            return
        }

        if (event.key === Qt.Key_I
                && !root.deleteDialogVisible
                && !root.resetDialogVisible
                && !root.actionDialogVisible
                && !root.messageDialogVisible) {
            if (root.episodeInfoSidebarVisible) {
                root.hideEpisodeInfoSidebar()
            } else if (appController.requestCurrentBrowserEpisodeInfo()) {
                root.episodeInfoSidebarVisible = true
            }
            event.accepted = true
            return
        }

        if (event.key === Qt.Key_F
                && !root.deleteDialogVisible
                && !root.resetDialogVisible
                && !root.actionDialogVisible
                && !root.messageDialogVisible
                && !root.episodeInfoSidebarVisible) {
            root.messageDialogText = ""
            appController.toggleFastMode()
            root.messageDialogVisible = true
            event.accepted = true
            return
        }

        if (event.key === Qt.Key_R
                && !root.deleteDialogVisible
                && !root.resetDialogVisible
                && !root.actionDialogVisible
                && !root.messageDialogVisible
                && !root.episodeInfoSidebarVisible) {
            root.resetChoiceIndex = 2
            root.resetDialogVisible = true
            event.accepted = true
            return
        }

        if (event.key === Qt.Key_V
                && !root.deleteDialogVisible
                && !root.resetDialogVisible
                && !root.actionDialogVisible
                && !root.messageDialogVisible
                && !root.episodeInfoSidebarVisible) {
            root.messageDialogText = "Version " + appController.appVersion
            root.messageDialogVisible = true
            event.accepted = true
            return
        }

        if ((root.deleteDialogVisible || root.resetDialogVisible || root.actionDialogVisible || root.messageDialogVisible || root.episodeInfoSidebarVisible) && event.key === Qt.Key_B) {
            root.deleteDialogVisible = false
            root.resetDialogVisible = false
            root.actionDialogVisible = false
            root.messageDialogVisible = false
            root.messageDialogText = ""
            root.hideEpisodeInfoSidebar()
            appController.clearPlayerMessage()
            event.accepted = true
        }
    }

    Connections {
        target: appController

        function onPlayerMessageChanged() {
            root.messageDialogVisible = appController.playerMessage !== ""
            if (root.messageDialogVisible)
                root.messageDialogText = appController.playerMessage
            else
                root.messageDialogText = ""
            root.forceActiveFocus()
        }

        function onCurrentIndexChanged() {
            root.hideEpisodeInfoSidebar()
            root.forceActiveFocus()
        }
    }

    CoverFlowView {
        id: coverFlow
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom

        anchors.topMargin: parent.height * 0.52
        anchors.bottomMargin: parent.height * 0.03

        model: libraryModel
        currentIndex: appController.currentIndex

        onActivated: function(index) {
            appController.playSelected(index)
        }

        onCurrentIndexChangedByUser: function(index) {
            appController.setCurrentIndex(index)
        }
    }

    Column {
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.topMargin: parent.height * 0.045
        anchors.rightMargin: parent.width * 0.04
        spacing: Math.max(2, parent.height * 0.005)

        Item {
            width: timeText.width + 3
            height: timeText.height + 3
            anchors.right: parent.right

            Text {
                id: timeShadow
                anchors.right: timeText.right
                x: 3
                y: 3
                text: root.formattedTime()
                color: "#B3000000"
                font.bold: true
                font.pixelSize: Math.max(28, root.height * 0.05)
                horizontalAlignment: Text.AlignRight
            }

            Text {
                id: timeText
                anchors.right: parent.right
                text: root.formattedTime()
                color: "white"
                font.bold: true
                font.pixelSize: Math.max(28, root.height * 0.05)
                horizontalAlignment: Text.AlignRight
            }
        }

        Item {
            width: dateText.width + 2
            height: dateText.height + 2
            anchors.right: parent.right

            Text {
                id: dateShadow
                anchors.right: dateText.right
                x: 2
                y: 2
                text: root.formattedDate()
                color: "#B3000000"
                font.bold: true
                font.pixelSize: Math.max(16, root.height * 0.024)
                horizontalAlignment: Text.AlignRight
            }

            Text {
                id: dateText
                anchors.right: parent.right
                text: root.formattedDate()
                color: "white"
                font.bold: true
                font.pixelSize: Math.max(16, root.height * 0.024)
                horizontalAlignment: Text.AlignRight
            }
        }
    }

    Rectangle {
        id: episodeInfoSidebar
        z: 1200
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: Math.min(parent.width * 0.90, Math.max(320, parent.width * 0.34))
        x: root.episodeInfoSidebarVisible ? 0 : -width
        color: "#D91A1A1A"
        border.width: 1
        border.color: "#707070"
        clip: true

        Behavior on x {
            NumberAnimation {
                duration: 230
                easing.type: Easing.OutCubic
            }
        }

        Item {
            id: episodeInfoContent
            anchors.fill: parent
            anchors.margins: Math.max(26, root.height * 0.055)

            BusyIndicator {
                id: episodeInfoBusy
                visible: appController.browserEpisodeInfoLoading
                running: visible
                anchors.left: parent.left
                anchors.top: parent.top
                width: Math.max(34, root.height * 0.052)
                height: width
            }

            Flickable {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: appController.browserEpisodeInfoLoading ? episodeInfoBusy.bottom : parent.top
                anchors.topMargin: appController.browserEpisodeInfoLoading ? Math.max(18, root.height * 0.025) : 0
                anchors.bottom: parent.bottom
                clip: true
                contentWidth: width
                contentHeight: episodeInfoColumn.height

                Column {
                    id: episodeInfoColumn
                    width: parent.width
                    height: implicitHeight
                    spacing: Math.max(12, root.height * 0.018)

                    Text {
                        width: parent.width
                        visible: appController.browserEpisodeInfoSeriesTitle !== ""
                        text: appController.browserEpisodeInfoSeriesTitle
                        color: "#F4F4F4"
                        wrapMode: Text.WordWrap
                        font.bold: true
                        font.pixelSize: Math.max(18, Math.min(28, root.height * 0.032))
                        lineHeight: 1.08
                    }

                    Text {
                        width: parent.width
                        visible: appController.browserEpisodeInfoSeasonEpisode !== ""
                        text: appController.browserEpisodeInfoSeasonEpisode
                        color: "#CFCFCF"
                        wrapMode: Text.WordWrap
                        font.bold: true
                        font.pixelSize: Math.max(15, Math.min(22, root.height * 0.024))
                        lineHeight: 1.05
                    }

                    Text {
                        width: parent.width
                        text: appController.browserEpisodeInfoTitle
                        color: "white"
                        wrapMode: Text.WordWrap
                        font.bold: true
                        font.pixelSize: Math.max(22, Math.min(34, root.height * 0.038))
                        lineHeight: 1.12
                    }

                    Row {
                        id: episodeInfoBodyRow
                        width: parent.width
                        visible: appController.browserEpisodeInfoDescription !== ""
                                 || String(appController.browserEpisodeInfoCoverSource) !== ""
                        spacing: Math.max(14, root.width * 0.012)

                        readonly property bool hasCover: String(appController.browserEpisodeInfoCoverSource) !== ""

                        Item {
                            width: episodeInfoBodyRow.hasCover
                                   ? (episodeInfoBodyRow.width - episodeInfoBodyRow.spacing) * 0.66
                                   : episodeInfoBodyRow.width
                            height: episodeInfoDescriptionText.height

                            Text {
                                id: episodeInfoDescriptionText
                                width: parent.width
                                visible: appController.browserEpisodeInfoDescription !== ""
                                text: appController.browserEpisodeInfoDescription
                                color: "#E6E6E6"
                                wrapMode: Text.WordWrap
                                font.pixelSize: Math.max(16, Math.min(24, root.height * 0.027))
                                lineHeight: 1.18
                            }
                        }

                        Image {
                            width: (episodeInfoBodyRow.width - episodeInfoBodyRow.spacing) * 0.34
                            height: width * 1.45
                            visible: episodeInfoBodyRow.hasCover
                            source: appController.browserEpisodeInfoCoverSource
                            fillMode: Image.PreserveAspectFit
                            smooth: true
                            asynchronous: true
                        }
                    }
                }
            }
        }
    }

    Rectangle {
        id: deleteDialog
        visible: root.deleteDialogVisible
        anchors.centerIn: parent
        width: parent.width * 0.42
        height: parent.height * 0.40
        radius: 14
        color: "#D91A1A1A"
        border.width: 1
        border.color: "#808080"
        clip: true

        Column {
            id: deleteDialogContent
            anchors.fill: parent
            anchors.margins: deleteDialog.height * 0.08
            spacing: deleteDialog.height * 0.06

            Text {
                width: deleteDialogContent.width
                horizontalAlignment: Text.AlignHCenter
                color: "white"
                wrapMode: Text.WordWrap
                font.pixelSize: Math.max(20, deleteDialog.height * 0.09)
                font.bold: true
                text: appController.deleteCurrentPromptText()
            }

            Item {
                width: deleteDialogContent.width
                height: deleteDialog.height * 0.03
            }

            Rectangle {
                width: deleteDialogContent.width
                height: Math.max(56, deleteDialog.height * 0.20)
                radius: 8
                color: root.deleteChoiceIndex === 0 ? "#2AA84A" : "#303030"
                border.width: 1
                border.color: root.deleteChoiceIndex === 0 ? "#7CF1A3" : "#666666"

                Text {
                    anchors.centerIn: parent
                    text: "JA"
                    color: "white"
                    font.bold: true
                    font.pixelSize: Math.max(22, deleteDialog.height * 0.10)
                }
            }

            Rectangle {
                width: deleteDialogContent.width
                height: Math.max(56, deleteDialog.height * 0.20)
                radius: 8
                color: root.deleteChoiceIndex === 1 ? "#2AA84A" : "#303030"
                border.width: 1
                border.color: root.deleteChoiceIndex === 1 ? "#7CF1A3" : "#666666"

                Text {
                    anchors.centerIn: parent
                    text: "NEIN"
                    color: "white"
                    font.bold: true
                    font.pixelSize: Math.max(22, deleteDialog.height * 0.09)
                }
            }
        }
    }

    Rectangle {
        id: resetDialog
        visible: root.resetDialogVisible
        anchors.centerIn: parent
        width: parent.width * 0.50
        height: parent.height * 0.62
        radius: 14
        color: "#D91A1A1A"
        border.width: 1
        border.color: "#808080"
        clip: true

        Column {
            id: resetDialogContent
            anchors.fill: parent
            anchors.margins: Math.max(24, parent.height * 0.05)
            spacing: Math.max(14, parent.height * 0.025)

            Text {
                width: resetDialogContent.width
                horizontalAlignment: Text.AlignHCenter
                color: "white"
                wrapMode: Text.WordWrap
                font.pixelSize: Math.max(20, resetDialog.height * 0.075)
                font.bold: true
                text: "Welche Datenbankeintraege sollen geloescht werden?"
            }

            Item {
                width: resetDialogContent.width
                height: Math.max(6, parent.height * 0.01)
            }

            Rectangle {
                width: resetDialogContent.width
                height: Math.max(54, resetDialog.height * 0.14)
                radius: 8
                color: root.resetChoiceIndex === 0 ? "#2AA84A" : "#303030"
                border.width: 1
                border.color: root.resetChoiceIndex === 0 ? "#7CF1A3" : "#666666"

                Text {
                    anchors.centerIn: parent
                    text: "ALLE"
                    color: "white"
                    font.bold: true
                    font.pixelSize: Math.max(22, resetDialog.height * 0.075)
                }
            }

            Rectangle {
                width: resetDialogContent.width
                height: Math.max(54, resetDialog.height * 0.14)
                radius: 8
                color: root.resetChoiceIndex === 1 ? "#2AA84A" : "#303030"
                border.width: 1
                border.color: root.resetChoiceIndex === 1 ? "#7CF1A3" : "#666666"

                Text {
                    anchors.centerIn: parent
                    text: "NUR AKTUELLER ORDNER"
                    color: "white"
                    font.bold: true
                    font.pixelSize: Math.max(22, resetDialog.height * 0.065)
                }
            }

            Rectangle {
                width: resetDialogContent.width
                height: Math.max(54, resetDialog.height * 0.14)
                radius: 8
                color: root.resetChoiceIndex === 2 ? "#2AA84A" : "#303030"
                border.width: 1
                border.color: root.resetChoiceIndex === 2 ? "#7CF1A3" : "#666666"

                Text {
                    anchors.centerIn: parent
                    text: "ABBRECHEN"
                    color: "white"
                    font.bold: true
                    font.pixelSize: Math.max(22, resetDialog.height * 0.065)
                }
            }
        }
    }

    Rectangle {
        id: actionDialog
        visible: root.actionDialogVisible
        anchors.centerIn: parent
        width: parent.width * 0.46
        height: parent.height * 0.46
        radius: 14
        color: "#D91A1A1A"
        border.width: 1
        border.color: "#808080"
        clip: true

        Column {
            id: actionDialogContent
            anchors.fill: parent
            anchors.margins: actionDialog.height * 0.08
            spacing: actionDialog.height * 0.06

            Text {
                width: actionDialogContent.width
                horizontalAlignment: Text.AlignHCenter
                color: "white"
                wrapMode: Text.WordWrap
                font.pixelSize: Math.max(20, actionDialog.height * 0.09)
                font.bold: true
                text: "Welche Aktion moechten Sie durchfuehren?"
            }

            Item {
                width: actionDialogContent.width
                height: actionDialog.height * 0.03
            }

            Rectangle {
                width: actionDialogContent.width
                height: Math.max(56, actionDialog.height * 0.20)
                radius: 8
                color: root.actionChoiceIndex === 0 ? "#2AA84A" : "#303030"
                border.width: 1
                border.color: root.actionChoiceIndex === 0 ? "#7CF1A3" : "#666666"

                Text {
                    anchors.centerIn: parent
                    text: "BEENDEN"
                    color: "white"
                    font.bold: true
                    font.pixelSize: Math.max(22, actionDialog.height * 0.09)
                }
            }

            Rectangle {
                width: actionDialogContent.width
                height: Math.max(56, actionDialog.height * 0.20)
                radius: 8
                color: root.actionChoiceIndex === 1 ? "#2AA84A" : "#303030"
                border.width: 1
                border.color: root.actionChoiceIndex === 1 ? "#7CF1A3" : "#666666"

                Text {
                    anchors.centerIn: parent
                    text: "ZURUECK"
                    color: "white"
                    font.bold: true
                    font.pixelSize: Math.max(22, actionDialog.height * 0.09)
                }
            }
        }
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
                text: root.messageDialogText
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
