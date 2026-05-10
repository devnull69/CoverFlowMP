import QtQuick

Rectangle {
    id: root

    property string text: ""
    property string iconName: "play"
    signal clicked()

    radius: Math.max(6, height * 0.16)
    color: actionMouseArea.pressed ? "#DADADA" : "#F7F7F7"

    Row {
        anchors.centerIn: parent
        spacing: Math.max(8, parent.width * 0.06)

        Canvas {
            id: actionIcon
            width: Math.max(14, Math.min(20, root.height * 0.34))
            height: width
            anchors.verticalCenter: parent.verticalCenter

            onPaint: {
                var ctx = getContext("2d")
                ctx.clearRect(0, 0, width, height)
                ctx.fillStyle = "#000000"

                if (root.iconName === "playNext") {
                    ctx.beginPath()
                    ctx.moveTo(width * 0.12, height * 0.08)
                    ctx.lineTo(width * 0.12, height * 0.92)
                    ctx.lineTo(width * 0.66, height * 0.50)
                    ctx.closePath()
                    ctx.fill()
                    ctx.fillRect(width * 0.76, height * 0.10, width * 0.12, height * 0.80)
                    return
                }

                ctx.beginPath()
                ctx.moveTo(width * 0.20, height * 0.08)
                ctx.lineTo(width * 0.20, height * 0.92)
                ctx.lineTo(width * 0.88, height * 0.50)
                ctx.closePath()
                ctx.fill()
            }

            onWidthChanged: requestPaint()
            onHeightChanged: requestPaint()
            Connections {
                target: root
                function onIconNameChanged() {
                    actionIcon.requestPaint()
                }
            }
        }

        Text {
            anchors.verticalCenter: parent.verticalCenter
            text: root.text
            color: "black"
            font.bold: true
            font.pixelSize: Math.max(15, Math.min(20, root.height * 0.36))
        }
    }

    MouseArea {
        id: actionMouseArea
        anchors.fill: parent
        onClicked: root.clicked()
    }
}
