import QtQuick
import QtQuick.Controls

Item {
    id: root
    anchors.fill: parent
    focus: true
    property bool deleteDialogVisible: false
    property int deleteChoiceIndex: 1 // 0 = JA, 1 = NEIN (default)

    Component.onCompleted: {
        forceActiveFocus()
    }

    Keys.onEscapePressed: Qt.quit()

    Keys.onLeftPressed: function(event) {
        if (root.deleteDialogVisible) {
            event.accepted = true
            return
        }
        coverFlow.moveLeft()
        event.accepted = true
    }

    Keys.onRightPressed: function(event) {
        if (root.deleteDialogVisible) {
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
        appController.playSelected(appController.currentIndex)
        event.accepted = true
    }

    Keys.onUpPressed: function(event) {
        if (!root.deleteDialogVisible)
            return
        root.deleteChoiceIndex = 0
        event.accepted = true
    }

    Keys.onDownPressed: function(event) {
        if (!root.deleteDialogVisible)
            return
        root.deleteChoiceIndex = 1
        event.accepted = true
    }

    Keys.onPressed: function(event) {
        if (event.key === Qt.Key_0 && !root.deleteDialogVisible && appController.canDeleteCurrentVideo()) {
            root.deleteChoiceIndex = 1
            root.deleteDialogVisible = true
            event.accepted = true
            return
        }

        if (root.deleteDialogVisible && event.key === Qt.Key_B) {
            root.deleteDialogVisible = false
            event.accepted = true
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
                text: "Dieses Video wirklich loeschen?"
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
}
