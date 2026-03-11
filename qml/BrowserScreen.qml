import QtQuick
import QtQuick.Controls

Item {
    id: root
    anchors.fill: parent
    focus: true

    Component.onCompleted: {
        forceActiveFocus()
    }

    Keys.onEscapePressed: Qt.quit()

    Keys.onLeftPressed: function(event) {
        coverFlow.moveLeft()
        event.accepted = true
    }

    Keys.onRightPressed: function(event) {
        coverFlow.moveRight()
        event.accepted = true
    }

    Keys.onReturnPressed: function(event) {
        appController.playSelected(appController.currentIndex)
        event.accepted = true
    }

    Keys.onEnterPressed: function(event) {
        appController.playSelected(appController.currentIndex)
        event.accepted = true
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
}
