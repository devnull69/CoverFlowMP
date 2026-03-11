import QtQuick
import QtQuick.Controls
import Qt5Compat.GraphicalEffects

Item {
    id: root

    property var model
    property int currentIndex: 0
    property int coverWidth: 270
    property int coverHeight: 330
    property int coverRadius: 16
    property real tiltAngle: 18

    signal activated(int index)
    signal currentIndexChangedByUser(int index)

    function syncToCurrentIndex() {
        if (listView.count > 0 && root.currentIndex >= 0 && root.currentIndex < listView.count) {
            listView.positionViewAtIndex(root.currentIndex, ListView.Center)
        }
    }

    function moveLeft() {
        if (root.currentIndex > 0) {
            root.currentIndex = root.currentIndex - 1
            currentIndexChangedByUser(root.currentIndex)
        }
    }

    function moveRight() {
        if (root.currentIndex < listView.count - 1) {
            root.currentIndex = root.currentIndex + 1
            currentIndexChangedByUser(root.currentIndex)
        }
    }

    onModelChanged: {
        startupPositionTimer.start()
    }

    Component.onCompleted: {
        root.currentIndex = 0
        startupPositionTimer.start()
    }

    Timer {
        id: startupPositionTimer
        interval: 10
        repeat: false
        onTriggered: {
            syncToCurrentIndex()
        }
    }

    ListView {
        id: listView
        anchors.fill: parent
        anchors.leftMargin: 80
        anchors.rightMargin: 80
        anchors.topMargin: 20
        anchors.bottomMargin: 20

        orientation: ListView.Horizontal
        spacing: -70
        model: root.model
        currentIndex: root.currentIndex
        snapMode: ListView.SnapOneItem
        highlightRangeMode: ListView.StrictlyEnforceRange
        preferredHighlightBegin: width / 2 - root.coverWidth / 2
        preferredHighlightEnd: width / 2 - root.coverWidth / 2
        highlightMoveDuration: 220
        clip: true

        delegate: Item {
            id: delegateRoot

            width: root.coverWidth
            height: root.coverHeight + 110

            readonly property bool isCurrent: index === root.currentIndex
            readonly property bool isLeftSide: index < root.currentIndex
            readonly property bool isRightSide: index > root.currentIndex

            scale: isCurrent ? 1.0 : 0.84
            opacity: 1.0
            property int distanceFromCurrent: Math.abs(index - root.currentIndex)
            z: 1000 - distanceFromCurrent

            transform: Rotation {
                origin.x: delegateRoot.width / 2
                origin.y: root.coverHeight / 2
                axis { x: 0; y: 1; z: 0 }
                angle: isCurrent ? 0 : (isLeftSide ? root.tiltAngle : -root.tiltAngle)

                Behavior on angle {
                    NumberAnimation { duration: 220 }
                }
            }

            Behavior on scale {
                NumberAnimation { duration: 220 }
            }

            Behavior on opacity {
                NumberAnimation { duration: 220 }
            }

            Column {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom
                anchors.bottomMargin: isCurrent ? 24 : 0
                spacing: 12

                Behavior on anchors.bottomMargin {
                    NumberAnimation { duration: 220 }
                }

                Rectangle {
                    id: coverFrame
                    width: root.coverWidth
                    height: root.coverHeight
                    radius: root.coverRadius
                    color: "#1c1c1c"
                    border.width: isCurrent ? 3 : 1
                    border.color: isCurrent ? "white" : "#666666"
                    clip: true

                    Item {
                        anchors.fill: parent
                        anchors.margins: isCurrent ? 3 : 1
                        visible: coverPath !== ""

                        Image {
                            id: coverImage
                            anchors.fill: parent
                            fillMode: Image.PreserveAspectCrop
                            source: coverPath !== "" ? "file://" + coverPath : ""
                            visible: false
                            smooth: true
                        }

                        Rectangle {
                            id: coverMask
                            anchors.fill: parent
                            radius: root.coverRadius - (isCurrent ? 3 : 1)
                            visible: false
                        }

                        OpacityMask {
                            anchors.fill: parent
                            source: coverImage
                            maskSource: coverMask
                        }
                    }

                    Rectangle {
                        anchors.fill: parent
                        anchors.margins: isCurrent ? 3 : 1
                        visible: coverPath === ""
                        color: isDemo ? "#4c5870" : "#303030"
                        radius: root.coverRadius - (isCurrent ? 3 : 1)
                    }

                    Column {
                        anchors.centerIn: parent
                        visible: coverPath === ""
                        spacing: 8

                        Text {
                            text: isDemo ? "DEMO" : "VIDEO"
                            color: "white"
                            font.pixelSize: 28
                            horizontalAlignment: Text.AlignHCenter
                            width: root.coverWidth - 40
                        }

                        Text {
                            text: title
                            color: "#dddddd"
                            font.pixelSize: 16
                            horizontalAlignment: Text.AlignHCenter
                            wrapMode: Text.WordWrap
                            width: root.coverWidth - 40
                        }
                    }
                }

                Text {
                    anchors.horizontalCenter: coverFrame.horizontalCenter
                    width: root.coverWidth + 80
                    text: title
                    color: "white"
                    font.pixelSize: 24
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.Wrap
                    elide: Text.ElideNone
                    visible: isCurrent
                    opacity: isCurrent ? 1.0 : 0.0

                    Behavior on opacity {
                        NumberAnimation { duration: 180 }
                    }
                }

                Text {
                    width: root.coverWidth
                    text: ""
                    color: "white"
                    font.pixelSize: 24
                    horizontalAlignment: Text.AlignHCenter
                    elide: Text.ElideRight
                    visible: !isCurrent
                }
            }

            MouseArea {
                anchors.fill: parent

                onClicked: {
                    root.currentIndex = index
                    currentIndexChangedByUser(index)
                }

                onDoubleClicked: {
                    activated(index)
                }
            }
        }
    }
}
