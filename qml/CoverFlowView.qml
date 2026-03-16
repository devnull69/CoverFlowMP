import QtQuick
import QtQuick.Controls
import Qt5Compat.GraphicalEffects

Item {
    id: root

    property var model
    property int currentIndex: 0

    property real coverAspectRatio: 270 / 330
    property real flowWidth: width
    property real flowHeight: height

    property real coverHeight: Math.min(flowHeight * 0.72, flowWidth * 0.22 / coverAspectRatio)
    property real coverWidth: coverHeight * coverAspectRatio
    property real coverRadius: coverWidth * 0.06
    property real tiltAngle: 18

    property real overlapRatio: 0.70
    property real stepX: coverWidth - (coverWidth * overlapRatio)
    property real verticalLift: coverHeight * 0.07
    property real titleWidth: coverWidth * 1.35
    property real baseYOffset: root.height * 0.10
    property real leftSideExtraOffset: coverWidth * 0.25

    signal activated(int index)
    signal currentIndexChangedByUser(int index)

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

    function moveLeft() {
        if (root.currentIndex > 0) {
            currentIndexChangedByUser(root.currentIndex - 1)
        }
    }

    function moveRight() {
        if (repeater.count > 0 && root.currentIndex < repeater.count - 1) {
            currentIndexChangedByUser(root.currentIndex + 1)
        }
    }

    function coverCenterX(index) {
        var delta = index - root.currentIndex
        var x = root.width / 2 + delta * root.stepX

        if (delta < 0) {
            x = x - Math.abs(delta) * root.leftSideExtraOffset
        }

        return x
    }

    function coverLeftX(index) {
        return coverCenterX(index) - root.coverWidth / 2
    }

    function coverBaseY(index) {
        return index === root.currentIndex
            ? root.height - root.coverHeight - root.verticalLift - root.baseYOffset
            : root.height - root.coverHeight - root.baseYOffset
    }

    Item {
        id: viewport
        anchors.fill: parent
        clip: true

        Repeater {
            id: repeater
            model: root.model

            delegate: Item {
                id: delegateRoot

                required property int index
                required property string title
                required property string coverPath
                required property double duration
                required property double resumePosition
                required property bool isDemo
                required property bool isFolder
                required property bool isParentFolder

                readonly property bool isCurrent: index === root.currentIndex
                readonly property bool isLeftSide: index < root.currentIndex
                readonly property bool isRightSide: index > root.currentIndex
                readonly property int distanceFromCurrent: Math.abs(index - root.currentIndex)
                readonly property bool showResumeBar: !isFolder
                                                      && !isParentFolder
                                                      && duration > 0
                                                      && resumePosition >= 3.0
                readonly property real resumeProgress: duration > 0
                                                      ? Math.max(0, Math.min(1, resumePosition / duration))
                                                      : 0
                readonly property bool showDurationLabel: !isFolder
                                                          && !isParentFolder
                                                          && duration > 0

                width: root.coverWidth
                height: root.coverHeight + root.height * 0.18

                x: root.coverLeftX(index)
                y: root.coverBaseY(index)

                scale: isCurrent ? 1.0 : 0.84
                opacity: 1.0
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

                Behavior on x {
                    NumberAnimation { duration: 220 }
                }

                Behavior on y {
                    NumberAnimation { duration: 220 }
                }

                Behavior on scale {
                    NumberAnimation { duration: 220 }
                }

                Column {
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.top
                    spacing: 12

                    Rectangle {
                        id: coverFrame
                        width: root.coverWidth
                        height: root.coverHeight
                        radius: root.coverRadius
                        color: "#1c1c1c"
                        border.width: isCurrent
                                      ? Math.max(2, root.coverWidth * 0.012)
                                      : Math.max(1, root.coverWidth * 0.004)
                        border.color: isCurrent ? "white" : "#666666"
                        clip: true

                        Item {
                            anchors.fill: parent
                            anchors.margins: isCurrent ? 3 : 1
                            visible: coverPath !== ""

                            Image {
                                id: coverImage
                                anchors.fill: parent
                                fillMode: Image.Stretch
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
                            visible: coverPath === "" && !isFolder
                            color: isDemo ? "#4c5870" : "#303030"
                            radius: root.coverRadius - (isCurrent ? 3 : 1)
                        }

                        Rectangle {
                            anchors.fill: parent
                            anchors.margins: isCurrent ? 3 : 1
                            visible: isFolder && coverPath === ""
                            color: isParentFolder ? "#2f4f6f" : "#345c7f"
                            radius: root.coverRadius - (isCurrent ? 3 : 1)

                            Rectangle {
                                anchors.horizontalCenter: parent.horizontalCenter
                                anchors.top: parent.top
                                anchors.topMargin: parent.height * 0.18
                                width: parent.width * 0.54
                                height: parent.height * 0.22
                                radius: parent.height * 0.18
                                color: isParentFolder ? "#6f98bd" : "#84aed2"
                            }

                            Rectangle {
                                anchors.horizontalCenter: parent.horizontalCenter
                                anchors.top: parent.top
                                anchors.topMargin: parent.height * 0.30
                                width: parent.width * 0.74
                                height: parent.height * 0.46
                                radius: parent.height * 0.10
                                color: isParentFolder ? "#7aa4ca" : "#98c1e3"
                            }
                        }

                        Column {
                            anchors.centerIn: parent
                            visible: coverPath === "" && !isFolder
                            spacing: 8

                            Text {
                                text: isDemo ? "DEMO" : "VIDEO"
                                color: "white"
                                font.pixelSize: root.coverHeight * 0.10
                                horizontalAlignment: Text.AlignHCenter
                                width: root.coverWidth - 40
                            }

                            Text {
                                text: title
                                color: "#dddddd"
                                font.pixelSize: root.coverHeight * 0.055
                                horizontalAlignment: Text.AlignHCenter
                                wrapMode: Text.WordWrap
                                width: root.coverWidth - 40
                            }
                        }

                        Rectangle {
                            visible: delegateRoot.showResumeBar
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.bottom: parent.bottom
                            anchors.leftMargin: (isCurrent ? 3 : 1) + Math.max(3, root.coverWidth * 0.02)
                            anchors.rightMargin: (isCurrent ? 3 : 1) + Math.max(3, root.coverWidth * 0.02)
                            anchors.bottomMargin: isCurrent ? 3 : 1
                            height: Math.max(3, root.coverHeight * 0.022)
                            color: "#5A1C1C"
                            opacity: 0.95

                            Rectangle {
                                anchors.left: parent.left
                                anchors.top: parent.top
                                anchors.bottom: parent.bottom
                                width: parent.width * delegateRoot.resumeProgress
                                color: "#E50914"
                            }
                        }

                        Item {
                            visible: delegateRoot.showDurationLabel
                            anchors.right: parent.right
                            anchors.bottom: parent.bottom
                            anchors.rightMargin: isCurrent ? 8 : 6
                            anchors.bottomMargin: (isCurrent ? 3 : 1) + Math.max(10, root.coverHeight * 0.045)
                            width: root.coverWidth * 0.15
                            height: Math.max(14, root.coverHeight * 0.07)

                            Text {
                                width: parent.width
                                height: parent.height
                                x: 1
                                y: 1
                                text: root.formatTime(delegateRoot.duration)
                                color: "#99000000"
                                horizontalAlignment: Text.AlignRight
                                verticalAlignment: Text.AlignVCenter
                                font.pixelSize: Math.max(10, root.coverHeight * 0.052)
                                font.bold: true
                                elide: Text.ElideNone
                            }

                            Text {
                                width: parent.width
                                height: parent.height
                                text: root.formatTime(delegateRoot.duration)
                                color: "white"
                                horizontalAlignment: Text.AlignRight
                                verticalAlignment: Text.AlignVCenter
                                font.pixelSize: Math.max(10, root.coverHeight * 0.052)
                                font.bold: true
                                elide: Text.ElideNone
                            }
                        }

                    }

                    Text {
                        anchors.horizontalCenter: coverFrame.horizontalCenter
                        width: root.titleWidth
                        text: title
                        color: "white"
                        font.pixelSize: root.coverHeight * 0.072
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
                        font.pixelSize: root.coverHeight * 0.072
                        horizontalAlignment: Text.AlignHCenter
                        visible: !isCurrent
                    }
                }

                MouseArea {
                    anchors.fill: parent

                    onClicked: {
                        currentIndexChangedByUser(index)
                    }

                    onDoubleClicked: {
                        activated(index)
                    }
                }
            }
        }
    }
}
