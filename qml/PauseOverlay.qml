import QtQuick
import QtQuick.Controls

Item {
    property double position: 0
    property double duration: 0

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

    Item {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: parent.height * 0.25

        Column {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
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
                    width: duration > 0 ? parent.width * (position / duration) : 0
                    height: parent.height
                    color: "white"
                    radius: parent.radius
                }
            }

            Text {
                text: formatTime(position) + " / " + formatTime(duration)
                color: "white"
                horizontalAlignment: Text.AlignHCenter
                width: contentColumn.width
                font.pixelSize: Math.max(20, parent.height * 0.24)
                font.bold: true
            }
            }
        }
    }
}
