import QtQuick
import QtQuick.Controls

Item {
    id: root
    property double position: 0
    property double duration: 0
    property string videoName: ""
    property double audioDelay: 0
    property bool audioDelayMode: false
    property var skipRanges: []
    property bool skipRangePending: false
    property double pendingSkipStart: 0
    property string currentClockText: ""

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

    function updateCurrentClockText() {
        var now = new Date()
        currentClockText = pad2(now.getHours()) + ":" + pad2(now.getMinutes())
    }

    function audioDelayHandleX(trackWidth, handleWidth) {
        var normalized = (Math.max(-2.0, Math.min(2.0, audioDelay)) + 2.0) / 4.0
        return normalized * Math.max(0, trackWidth - handleWidth)
    }

    function normalizedTime(value) {
        if (duration <= 0)
            return 0
        return Math.max(0, Math.min(1, value / duration))
    }

    Component.onCompleted: updateCurrentClockText()

    Timer {
        interval: 1000
        running: true
        repeat: true
        triggeredOnStart: true
        onTriggered: root.updateCurrentClockText()
    }

    Item {
        id: overlayArea
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

                Item {
                    width: contentColumn.width
                    height: Math.max(8, overlayArea.height * 0.12)

                    readonly property real clockSlotWidth: contentColumn.width * 0.12
                    readonly property real clockGap: Math.max(10, contentColumn.width * 0.018)
                    readonly property real trackX: root.audioDelayMode ? 0 : clockSlotWidth + clockGap
                    readonly property real trackWidth: root.audioDelayMode ? width : Math.max(0, width - trackX)

                    Text {
                        visible: !root.audioDelayMode
                        anchors.left: parent.left
                        anchors.verticalCenter: parent.verticalCenter
                        width: parent.clockSlotWidth
                        text: root.currentClockText
                        color: "#D8D8D8"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font.pixelSize: Math.max(8, overlayArea.height * 0.096)
                        font.bold: true
                    }

                    Rectangle {
                        x: parent.trackX
                        width: parent.trackWidth
                        height: parent.height
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

                        Repeater {
                            model: root.skipRanges

                            delegate: Rectangle {
                                property var range: modelData

                                visible: !root.audioDelayMode && duration > 0 && range.end > range.start
                                x: parent.width * root.normalizedTime(range.start)
                                width: Math.max(2, parent.width * (root.normalizedTime(range.end) - root.normalizedTime(range.start)))
                                height: parent.height
                                color: "#C92A2A"
                                opacity: 0.95
                            }
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
                }

                Row {
                    visible: !root.audioDelayMode
                    width: contentColumn.width
                    spacing: 0

                    Item {
                        width: contentColumn.width * 0.12 + Math.max(10, contentColumn.width * 0.018)
                        height: 1
                    }

                    Text {
                        text: formatTime(position)
                        color: "white"
                        horizontalAlignment: Text.AlignLeft
                        width: (parent.width - (contentColumn.width * 0.12 + Math.max(10, contentColumn.width * 0.018))) * 0.5
                        font.pixelSize: Math.max(8, overlayArea.height * 0.096)
                        font.bold: true
                    }

                    Text {
                        text: formatTime(Math.max(0, duration - position))
                        color: "white"
                        horizontalAlignment: Text.AlignRight
                        width: (parent.width - (contentColumn.width * 0.12 + Math.max(10, contentColumn.width * 0.018))) * 0.5
                        font.pixelSize: Math.max(8, overlayArea.height * 0.096)
                        font.bold: true
                    }
                }

                Text {
                    visible: root.audioDelayMode
                    text: formatAudioDelay()
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    width: contentColumn.width
                    font.pixelSize: Math.max(8, overlayArea.height * 0.096)
                    font.bold: true
                }

                Item {
                    visible: !root.audioDelayMode
                    width: contentColumn.width
                    height: Math.max(12, overlayArea.height * 0.09)

                    Text {
                        anchors.fill: parent
                        visible: root.skipRangePending
                        text: "Skip-Start: " + formatTime(root.pendingSkipStart) + "   S=Ende   C=Abbrechen   +/-=Frame"
                        color: "#FFB3B3"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font.pixelSize: Math.max(7, overlayArea.height * 0.072)
                        font.bold: true
                    }
                }

                Text {
                    text: videoName
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    width: contentColumn.width
                    elide: Text.ElideMiddle
                    font.pixelSize: Math.max(10, overlayArea.height * 0.12)
                    font.bold: true
                }
            }
        }
    }
}
