import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.shmong 1.0

Page {
    id: page;
    allowedOrientations: Orientation.All;

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height

        VerticalScrollDecorator {}

        Column {
            id: column
            width: parent.width
            spacing: Theme.paddingSmall

            PageHeader { title: qsTr("Settings") }

            SectionHeader { text: qsTr("Notifications") }

            TextSwitch {
                id: chatNotificationSwitch
                checked: shmong.settings.DisplayChatNotifications
                text: qsTr("Display chat notifications")
                onClicked: {
                    shmong.settings.DisplayChatNotifications = chatNotificationSwitch.checked;
                }
            }
            TextSwitch {
                id: groupchatNotificationSwitch
                checked: shmong.settings.DisplayGroupchatNotifications
                text: qsTr("Display group chat notifications")
                onClicked: {
                    shmong.settings.DisplayGroupchatNotifications = groupchatNotificationSwitch.checked;
                }
            }

            SectionHeader { text: qsTr("Privacy") }

            TextSwitch {
                id: readNotificationSwitch
                checked: shmong.settings.SendReadNotifications
                text: qsTr("Send Read Notifications")
                onClicked: {
                    shmong.settings.SendReadNotifications = readNotificationSwitch.checked;
                }
            }

            SectionHeader { text: qsTr("Attachments") }

            TextSwitch {
                id: askBeforeDownloadingSwitch
                checked: shmong.settings.AskBeforeDownloading
                text: qsTr("Ask before downloading attachments")
                onClicked: {
                    shmong.settings.AskBeforeDownloading = askBeforeDownloadingSwitch.checked;
                }
            }

            SectionHeader { text: qsTr("OMEMO Encryption (v0.3)") }

            TextSwitch {
                id: softwareFeatureOmemoSwitch
                checked: shmong.settings.EnableSoftwareFeatureOmemo
                text: qsTr("Enable (Need app restart)")
                onClicked: {
                    shmong.settings.EnableSoftwareFeatureOmemo = softwareFeatureOmemoSwitch.checked;
                }
            }

            SectionHeader { text: qsTr("Debug") }

            TextSwitch {
                id: msgToConsoleSwitch
                checked: shmong.settings.MsgToConsole
                text: qsTr("Send messages to console")
                onClicked: {
                    shmong.settings.MsgToConsole = msgToConsoleSwitch.checked;
                }
            }
        }
    }
}
