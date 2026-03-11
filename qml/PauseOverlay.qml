import QtQuick
import QtQuick.Controls

Item {
    property double position: 0
    property double duration: 0

    Rectangle {
        anchors.fill: parent
        color: "#66000000"
    }

    Column {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 40
        spacing: 10
        width: parent.width * 0.7

        Rectangle {
            width: parent.width
            height: 10
            color: "#404040"

            Rectangle {
                width: duration > 0 ? parent.width * (position / duration) : 0
                height: parent.height
                color: "white"
            }
        }

        Text {
            text: Math.floor(position) + " / " + Math.floor(duration) + " sec"
            color: "white"
            horizontalAlignment: Text.AlignHCenter
            width: parent.width
        }
    }
}
