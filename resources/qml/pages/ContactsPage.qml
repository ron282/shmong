import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.shmong 1.0

Page {
    id: page;
    allowedOrientations: Orientation.All;

    Component {
        id: sectionHeading

        Label {
            text: section
            color: Theme.highlightColor
            font.pixelSize: Theme.fontSizeLarge
            anchors {
                left: parent.left
                leftMargin: Theme.paddingMedium
            }
        }
    }

    SilicaListView {
        id: jidlist
        header: Column {
            anchors {
                left: parent.left;
                right: parent.right;
            }

            PageHeader {
                title: qsTr("Contacts");
            }
            //            SearchField {
            //                placeholderText: qsTr ("Filter");
            //                anchors {
            //                    left: parent.left;
            //                    right: parent.right;
            //                }
            //            }
        }
        model: shmong.rosterController.rosterList
        spacing: Theme.paddingMedium;

        section.property: "name"
        section.delegate: sectionHeading
        section.criteria: ViewSection.FirstCharacter

        delegate: ListItem {
            id: item;

            menu: ContextMenu {
                MenuItem {
                    text:  qsTr("Remove");
                    onClicked: {
                        remorseAction(qsTr("Remove contact"),
                        function() {
                            if (shmong.rosterController.isGroup(jid)) {
                                shmong.removeRoom(jid);
                            }
                            else {
                                shmong.rosterController.removeContact(jid);
                            }
                        });
                    }
                }
            }
            contentHeight: Theme.itemSizeMedium;

            function removeContact() {
                remorseAction(qsTr("Remove contact"), function()
                {
                    if (shmong.rosterController.isGroup(jid)) {
                        shmong.removeRoom(jid)
                    }
                    else {
                        shmong.rosterController.removeContact(jid)
                    }
                }) 
            }

            onClicked: {
                shmong.setCurrentChatPartner(jid);
                pageStack.push (pageMessaging, { "conversationId" : jid });
            }

            Image {
                id: img;
                width: height;
                source: imagePath != "" ? imagePath : getImage(jid)
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
            Column {
                anchors {
                    left: presence.right;
                    right: parent.right
                    margins: Theme.paddingMedium;
                    verticalCenter: parent.verticalCenter;
                }

                Label {
                    id: nameId;
                    text: name
                    wrapMode: Text.NoWrap
                    maximumLineCount: 1
                    color: (item.highlighted ? Theme.highlightColor : Theme.primaryColor);
                    font.pixelSize: Theme.fontSizeMedium;
                }
                Row {
                    width: parent.width

                    Image {
                        id: subscriptionImage;
                        visible: false
//                        visible: ! shmong.rosterController.isGroup(jid)
                        source: getSubscriptionImage(subscription);
                    }
                    Label {
                        id: jidId;
                        text: jid;
                        width: parent.width - subscriptionImage.width - Theme.paddingMedium
                        truncationMode: TruncationMode.Fade
                        color: Theme.secondaryColor;
                        font.pixelSize: Theme.fontSizeTiny;
                    }
                }
                Label {
                    id: statusId;
                    text: status;
                    color: Theme.secondaryColor;
                    font.pixelSize: Theme.fontSizeTiny;
                }
                Component {
                    id: contextMenu
                    ContextMenu {
                        MenuItem {
                            text:  qsTr("Remove");
                            onClicked: removeContact()
                        }
                    }
                }
            }
        }

        anchors.fill: parent;

        PullDownMenu {
            enabled: true
            visible: true

            //            MenuItem {
            //                text: qsTr ("Create Room TBD");
            //                onClicked: {
            //                    console.log("create room")
            //                }
            //            }

            MenuItem {
                text: qsTr("Join room by address");
                onClicked: {
                    pageStack.push(dialogJoinRoom)
                }
            }

            MenuItem {
                text: qsTr("Add contact");
                onClicked: {
                    pageStack.push(dialogCreateContact)
                }
            }

        }
    }

    function getImage(jid) {
        if (shmong.rosterController.isGroup(jid)) {
            return "image://theme/icon-l-image";
        } else {
            return "image://theme/icon-l-people"
        }
    }

    function getSubscriptionImage(subs) {
        if (subs === RosterItem.SUBSCRIPTION_NONE) {
            return "image://theme/icon-cover-cancel"
        } else if (subs === RosterItem.SUBSCRIPTION_TO) {
            return "image://theme/icon-cover-next"
        } else if (subs === RosterItem.SUBSCRIPTION_FROM) {
            return "image://theme/icon-cover-previous"
        } else {
            return "image://theme/icon-cover-transfers"
        }
    }

    function getAvailabilityImage(avail) {
        if (avail === RosterItem.AVAILABILITY_ONLINE) {
            return "image://theme/icon-s-chat"
        } else if (avail === RosterItem.AVAILABILITY_OFFLINE) {
            return "image://theme/icon-s-high-importance"
        } else {
            return "image://theme/icon-s-timer"
        }
    }

}


