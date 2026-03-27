import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: root
    width: 1024
    height: 640
    visible: true
    title: "CoverFlowMP Konfigurator"
    color: "#EEF2F6"

    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#F5F8FB" }
            GradientStop { position: 1.0; color: "#DCE5ED" }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 18

        Label {
            text: "Konfigurator"
            font.pixelSize: 28
            font.bold: true
            color: "#17324D"
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: 18
            color: "#FAFCFE"
            border.color: "#C8D4E0"
            border.width: 1

            RowLayout {
                anchors.fill: parent
                spacing: 0

                Rectangle {
                    Layout.preferredWidth: 250
                    Layout.fillHeight: true
                    color: "#193A5A"
                    radius: 18

                    ListView {
                        anchors.fill: parent
                        anchors.margins: 16
                        model: ["Ansicht", "Ordner"]
                        spacing: 10
                        currentIndex: configController.currentSection
                        clip: true

                        delegate: Rectangle {
                            required property int index
                            required property string modelData
                            width: ListView.view.width
                            height: 54
                            radius: 12
                            color: ListView.isCurrentItem ? "#2F648F" : "transparent"
                            border.width: ListView.isCurrentItem ? 0 : 1
                            border.color: "#446986"

                            Text {
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.left: parent.left
                                anchors.leftMargin: 16
                                text: modelData
                                color: "white"
                                font.pixelSize: 20
                                font.bold: ListView.isCurrentItem
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: configController.currentSection = index
                            }
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: "transparent"

                    StackLayout {
                        anchors.fill: parent
                        anchors.margins: 26
                        currentIndex: configController.currentSection

                        ColumnLayout {
                            spacing: 18

                            Label {
                                text: "Ansicht"
                                font.pixelSize: 24
                                font.bold: true
                                color: "#17324D"
                            }

                            Label {
                                text: "Lege fest, welches Hintergrundbild der Browser der Haupt-App verwenden soll."
                                wrapMode: Text.WordWrap
                                color: "#4D6277"
                                Layout.fillWidth: true
                            }

                            ButtonGroup {
                                id: backgroundModeGroup
                            }

                            RadioButton {
                                text: "Standard"
                                checked: configController.useDefaultBackground
                                ButtonGroup.group: backgroundModeGroup
                                onToggled: if (checked) configController.useDefaultBackground = true
                            }

                            RadioButton {
                                text: "Dateipfad"
                                checked: !configController.useDefaultBackground
                                ButtonGroup.group: backgroundModeGroup
                                onToggled: if (checked) configController.useDefaultBackground = false
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 12
                                enabled: !configController.useDefaultBackground

                                TextField {
                                    Layout.fillWidth: true
                                    text: configController.backgroundPath
                                    placeholderText: "/pfad/zum/hintergrundbild.jpg"
                                    selectByMouse: true
                                    onTextChanged: configController.backgroundPath = text
                                }

                                Button {
                                    text: "Auswaehlen"
                                    onClicked: configController.browseBackgroundImage()
                                }
                            }

                            Rectangle {
                                Layout.fillWidth: true
                                radius: 12
                                color: configController.backgroundPathValid ? "#E7F7EA" : "#FDECEC"
                                border.width: 1
                                border.color: configController.backgroundPathValid ? "#8BC99B" : "#E5A6A6"
                                implicitHeight: validationText.implicitHeight + 20

                                Text {
                                    id: validationText
                                    anchors.fill: parent
                                    anchors.margins: 10
                                    text: configController.backgroundValidationMessage
                                    wrapMode: Text.WordWrap
                                    color: configController.backgroundPathValid ? "#245C33" : "#9A2E2E"
                                }
                            }

                            Item {
                                Layout.fillHeight: true
                            }
                        }

                        ColumnLayout {
                            spacing: 18

                            Label {
                                text: "Ordner"
                                font.pixelSize: 24
                                font.bold: true
                                color: "#17324D"
                            }

                            Label {
                                text: "Dieser Bereich ist fuer den ersten Schritt vorbereitet und folgt als naechstes."
                                wrapMode: Text.WordWrap
                                color: "#4D6277"
                                Layout.fillWidth: true
                            }

                            Item {
                                Layout.fillHeight: true
                            }
                        }
                    }
                }
            }
        }

        RowLayout {
            Layout.alignment: Qt.AlignRight
            spacing: 12

            Button {
                text: "Abbrechen"
                onClicked: configController.cancelAndQuit()
            }

            Button {
                text: "Speichern"
                enabled: configController.canSave
                onClicked: configController.saveAndQuit()
            }
        }
    }
}
