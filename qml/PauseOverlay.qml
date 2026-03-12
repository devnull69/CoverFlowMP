import QtQuick
import QtQuick.Controls

Item {
    id: root
    property double position: 0
    property double duration: 0
    property string videoName: ""
    property double audioDelay: 0
    property bool audioDelayMode: false

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

    function formatAudioDelay() {
        var delayMs = Math.round(audioDelay * 1000)
        var prefix = delayMs > 0 ? "+" : ""
        return prefix + delayMs + " ms"
    }

    function audioDelayHandleX(trackWidth, handleWidth) {
        var normalized = (Math.max(-2.0, Math.min(2.0, audioDelay)) + 2.0) / 4.0
        return normalized * Math.max(0, trackWidth - handleWidth)
    }

    Item {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: parent.height * 0.25

        Column {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            anchors.verticalCenterOffset: -root.height * 0.10
            spacing: parent.height * 0.08
            width: parent.width * 0.7

            Rectangle {
                anchors.fill: contentColumn
                anchors.margins: -Math.max(10, parent.height * 0.1)
                color: "#66000000"
                radius: Math.max(12, parent.height * 0.14)
                z: -1
            }

            Column {
                id: contentColumn
                spacing: parent.spacing
                width: parent.width

                Rectangle {
                    width: contentColumn.width
                    height: Math.max(16, parent.height * 0.24)
                    color: "#404040"
                    radius: height / 2
                    clip: true

                    Rectangle {
                        visible: !root.audioDelayMode
                        width: duration > 0 ? parent.width * (position / duration) : 0
                        height: parent.height
                        color: "#39C5FF"
                        radius: parent.radius
                    }

                    Rectangle {
                        visible: root.audioDelayMode
                        width: Math.max(16, parent.width * 0.045)
                        height: parent.height
                        x: root.audioDelayHandleX(parent.width, width)
                        color: "#39C5FF"
                        radius: parent.radius
                    }
                }

                Row {
                    visible: !root.audioDelayMode
                    width: contentColumn.width
                    spacing: 0

                    Text {
                        text: formatTime(position)
                        color: "white"
                        horizontalAlignment: Text.AlignLeft
                        width: parent.width * 0.5
                        font.pixelSize: Math.max(20, contentColumn.parent.height * 0.24)
                        font.bold: true
                    }

                    Text {
                        text: formatTime(Math.max(0, duration - position))
                        color: "white"
                        horizontalAlignment: Text.AlignRight
                        width: parent.width * 0.5
                        font.pixelSize: Math.max(20, contentColumn.parent.height * 0.24)
                        font.bold: true
                    }
                }

                Text {
                    visible: root.audioDelayMode
                    text: formatAudioDelay()
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    width: contentColumn.width
                    font.pixelSize: Math.max(20, contentColumn.parent.height * 0.24)
                    font.bold: true
                }

                Text {
                    text: videoName
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    width: contentColumn.width
                    elide: Text.ElideMiddle
                    font.pixelSize: Math.max(26, contentColumn.parent.height * 0.30)
                    font.bold: true
                }
            }
        }
    }
}
