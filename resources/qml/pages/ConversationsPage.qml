import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.shmong 1.0

Page {
    id: page;
    allowedOrientations: Orientation.All;

    SilicaListView {
        id: view;
        header: Column {
            spacing: Theme.paddingMedium;
            anchors {
                left: parent.left;
                right: parent.right;
            }

            PageHeader {
                title: qsTr("Conversations");
            }
        }
        ViewPlaceholder {
            enabled: view.count == 0
            text: qsTr("Empty")
            hintText: qsTr("Select a contact to start a conversation")
        }
        model: shmong.persistence.sessionController
        delegate: ListItem {
            id: item;
            contentHeight: Theme.itemSizeMedium;
            property int availability : shmong.rosterController.getAvailability(jid)

            onClicked: {
                console.log("set current chat partner: " + jid);
                pageStack.push (pageMessaging, { "conversationId" : jid });
                shmong.setCurrentChatPartner(jid);
            }

            Image {
                id: img;
                width: height;
                source: getImage(jid)
                anchors {
                    top: parent.top;
                    left: parent.left;
                    leftMargin: Theme.paddingMedium
                    bottom: parent.bottom;
                }

                Rectangle {
                    z: -1;
                    color: (model.index % 2 ? "black" : "white");
                    opacity: 0.15;
                    anchors.fill: parent;
                }
            }
            Rectangle {
                id: presence
                anchors {
                    left: img.right
                    top: img.top
                }
                width: Theme.paddingSmall
                height: img.height
                color: availability === RosterItem.AVAILABILITY_ONLINE ? "lime" : availability === RosterItem.AVAILABILITY_OFFLINE ? "gray" : "transparent"
            }
            Rectangle {
                width: Math.max(lblUnread.implicitWidth+radius, height)
                height: lblUnread.implicitHeight
                color: Theme.highlightBackgroundColor
                radius: height*0.5
                anchors {
                    top: img.top
                    right: img.right
                    topMargin: Theme.paddingSmall
                    rightMargin: Theme.paddingSmall
                }
                visible: (unreadmessages > 0) ? true : false
                Label {
                    id: lblUnread
                    font.bold: true
                    text: unreadmessages
                    font.pixelSize: Theme.fontSizeTiny
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
            Column {
                anchors {
                    left: presence.right;
                    margins: Theme.paddingMedium;
                    verticalCenter: parent.verticalCenter;
                    right: parent.right
                }

                Row {
                    Label {
                        id: nameId;
                        wrapMode: Text.NoWrap
                        maximumLineCount: 1
                        text: shmong.rosterController.getNameForJid(jid)
                        color: (item.highlighted ? Theme.highlightColor : Theme.primaryColor);
                        font.pixelSize: Theme.fontSizeMedium;
                    }
                }
                Label {
                    id: jidId;
                    text: jid;
                    color: Theme.secondaryColor;
                    font.pixelSize: Theme.fontSizeTiny;
                }
                Label {
                    id: statusId;
                    text: lastmessage;
                    anchors {
                        left: parent.left
                        right: parent.right
                    }
                    wrapMode: Text.WrapAnywhere
                    maximumLineCount: 1
                    color: Theme.secondaryColor;
                    font.pixelSize: Theme.fontSizeTiny;
                }
            }

            menu: ContextMenu {
                MenuItem {
                    text: qsTr("Delete")
                    onClicked: {
                        remorseAction(qsTr("Delete conversation"),
                                      function() {
                                            shmong.persistence.removeConversation(jid);
                                        })
                    }
                }
            }
            Connections {
                target: shmong
                onRosterListChanged: {
                    availability = shmong.rosterController.getAvailability(jid);
                }
            }
        }

        anchors.fill: parent;

    }

    function getImage(jid) {
        var imagePath = shmong.rosterController.getAvatarImagePathForJid(jid);

        if (imagePath.length > 0) {
            return imagePath;
        } else if (shmong.rosterController.isGroup(jid)) {
            return "image://theme/icon-l-image";
        } else {
            return "image://theme/icon-l-people"
        }
    }
}


