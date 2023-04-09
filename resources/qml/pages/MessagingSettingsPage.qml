import QtQuick 2.0 
import QtQuick.Window 2.0;
import Sailfish.Silica 1.0 
import harbour.shmong 1.0 

Page {
    id: page
    allowedOrientations: Orientation.All
    property string conversationId
    readonly property int limitCompression : shmong.settings.LimitCompression
    property int maxUploadSize : shmong.getMaxUploadSize();

    Timer {
        interval: 2000; running: true; repeat: true
        onTriggered: {
            maxUploadSize = shmong.getMaxUploadSize();
        }
    }

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height

        VerticalScrollDecorator {}


        Image {
            //source: "image://glass/qrc:///qml/img/photo.png"
            opacity: 0.85
            sourceSize: Qt.size (Screen.width, Screen.height)
            asynchronous: false
            anchors.centerIn: parent
        }
        Column {
            id: column
            PageHeader {
                title: conversationId+qsTr(" settings")
            }

            SectionHeader { text: qsTr("Notifications") }

            ComboBox {
                label: qsTr("Chat notifications")
                width: page.width
                currentIndex: (
                    shmong.settings.ForceOnNotifications.indexOf(conversationId) >= 0 ? 1 :
                    shmong.settings.ForceOffNotifications.indexOf(conversationId) >= 0 ? 2 :
                    0
                )
                menu: ContextMenu {
                    MenuItem {
                        text: qsTr("Default setting")
                        onClicked: {
                            shmong.settings.removeForceOnNotifications(conversationId);
                            shmong.settings.removeForceOffNotifications(conversationId);
                        }
                    }
                    MenuItem {
                        text: qsTr("On")
                        onClicked: {
                            shmong.settings.addForceOnNotifications(conversationId);
                            shmong.settings.removeForceOffNotifications(conversationId);
                        }
                    }
                    MenuItem {
                        text: qsTr("Off")
                        onClicked: {
                            shmong.settings.removeForceOnNotifications(conversationId);
                            shmong.settings.addForceOffNotifications(conversationId);
                        }
                    }
                }
            }

            TextSwitch {
                id: sendOmemoMsg
                enabled: shmong.isOmemoUser(conversationId)
                checked: {
                    if ( shmong.isOmemoUser(conversationId) === false) {
                        return false;
                    }
                    else if (shmong.settings.SendPlainText.indexOf(conversationId) >= 0) {
                        return false;
                    }
                    else {
                        return true;
                    }
                }
                text: qsTr("Send omemo encrypted messages")
                onClicked: {
                    if (sendOmemoMsg.checked) {
                        shmong.settings.removeForcePlainTextSending(conversationId)
                    }
                    else {
                        shmong.settings.addForcePlainTextSending(conversationId)
                    }
                }
            }

            SectionHeader { text: qsTr("Attachments (global settings)") }
            TextSwitch {
                id: compressImagesSwitch
                checked: shmong.settings.CompressImages
                text: qsTr("Limit compression to")
                onClicked: {
                    shmong.settings.CompressImages = compressImagesSwitch.checked;
                    limitCompressionSizeSlider.enabled = compressImagesSwitch.checked;
                }
            }

            Slider {
                id: limitCompressionSlider
                enabled: shmong.settings.CompressImages
                width: parent.width
                minimumValue: 100000
                maximumValue: Math.max(maxUploadSize, limitCompression)
                stepSize: 100000
                value: limitCompression
                valueText: value/1000 + qsTr(" KB")

                onValueChanged: {
                    shmong.settings.LimitCompression = sliderValue;
                }
            }

            TextSwitch {
                id: sendOnlyImagesSwitch
                checked: shmong.settings.SendOnlyImages
                text: qsTr("Send images only")
                onClicked: {
                    shmong.settings.SendOnlyImages = sendOnlyImagesSwitch.checked;
                }
            }
        }
    }
}
