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
    property bool messageDialogVisible: false
    property string messageDialogText: ""

    Component.onCompleted: {
        forceActiveFocus()
    }

    Keys.onEscapePressed: function(event) {
        if (root.deleteDialogVisible || root.resetDialogVisible || root.messageDialogVisible) {
            root.deleteDialogVisible = false
            root.resetDialogVisible = false
            root.messageDialogVisible = false
            root.messageDialogText = ""
            appController.clearPlayerMessage()
            event.accepted = true
            return
        }

        appController.navigateUpOrQuit()
        event.accepted = true
    }

    Keys.onLeftPressed: function(event) {
        if (root.deleteDialogVisible || root.resetDialogVisible || root.messageDialogVisible) {
            event.accepted = true
            return
        }
        coverFlow.moveLeft()
        event.accepted = true
    }

    Keys.onRightPressed: function(event) {
        if (root.deleteDialogVisible || root.resetDialogVisible || root.messageDialogVisible) {
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
        if (root.messageDialogVisible) {
            root.messageDialogText = ""
            appController.clearPlayerMessage()
            root.messageDialogVisible = false
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
        if (root.messageDialogVisible) {
            root.messageDialogText = ""
            appController.clearPlayerMessage()
            root.messageDialogVisible = false
            event.accepted = true
            return
        }
        appController.playSelected(appController.currentIndex)
        event.accepted = true
    }

    Keys.onUpPressed: function(event) {
        if (root.messageDialogVisible) {
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
    }

    Keys.onDownPressed: function(event) {
        if (root.messageDialogVisible) {
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
                && appController.canDeleteCurrentVideo()) {
            root.deleteChoiceIndex = 1
            root.deleteDialogVisible = true
            event.accepted = true
            return
        }

        if (event.key === Qt.Key_F
                && !root.deleteDialogVisible
                && !root.resetDialogVisible
                && !root.messageDialogVisible) {
            root.messageDialogText = ""
            appController.toggleFastMode()
            root.messageDialogVisible = true
            event.accepted = true
            return
        }

        if (event.key === Qt.Key_R && !root.deleteDialogVisible && !root.resetDialogVisible) {
            root.resetChoiceIndex = 2
            root.resetDialogVisible = true
            event.accepted = true
            return
        }

        if (event.key === Qt.Key_V
                && !root.deleteDialogVisible
                && !root.resetDialogVisible
                && !root.messageDialogVisible) {
            root.messageDialogText = "Version " + appController.appVersion
            root.messageDialogVisible = true
            event.accepted = true
            return
        }

        if ((root.deleteDialogVisible || root.resetDialogVisible || root.messageDialogVisible) && event.key === Qt.Key_B) {
            root.deleteDialogVisible = false
            root.resetDialogVisible = false
            root.messageDialogVisible = false
            root.messageDialogText = ""
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
