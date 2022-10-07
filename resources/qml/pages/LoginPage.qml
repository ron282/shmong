import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.shmong 1.0

Page {
    id: loginPage
    allowedOrientations: Orientation.All
    SilicaFlickable {
        anchors.fill: parent;
        contentHeight: loginPageContent.height

        Column {
            id: loginPageContent
            anchors {
                fill: parent;
                margins: Theme.paddingMedium;
            }

            spacing: Theme.paddingSmall

            PageHeader {
                title: qsTr("Welcome to ShmoNG")
            }

            Label {
                x: Theme.paddingLarge
                width: parent.width - 2*Theme.paddingLarge
                wrapMode: Text.Wrap
                text: qsTr("Login to Jabber Server")
                truncationMode: TruncationMode.Elide
                color: Theme.secondaryHighlightColor
                font.pixelSize: Theme.fontSizeExtraLarge
            }

            TextField {
                id: jidTextField
                placeholderText: qsTr("jid@server.com")
                label: qsTr("JID")
                text: shmong.settings.Jid
                width: parent.width

                onTextChanged: {
                    checkEnableConnectButton();
                }
            }

            TextField {
                id: passTextField
                placeholderText: qsTr("password")
                echoMode: TextInput.Password
                label: qsTr("Password")
                text: shmong.settings.Password
                width: parent.width

                onTextChanged: {
                    checkEnableConnectButton();
                }
            }
            Row {
                id: credentialsRow
                width: parent.width
                Switch {
                    id: saveCredentials
                    checked: shmong.settings.SaveCredentials
                    onClicked: {
                        shmong.settings.SaveCredentials = saveCredentials.checked;
                    }
                }
                Label {
                    wrapMode: Text.Wrap
                    width: credentialsRow.width - saveCredentials.width
                    text: qsTr("Save credentials (unencrypted)")
                    font.pixelSize: Theme.fontSizeSmall
                    anchors {
                        verticalCenter: parent.verticalCenter;
                    }
                }
            }

            Button{
                id: connectButton
                x: Theme.paddingLarge
                text: qsTr("Connect")
                enabled: false

                onClicked: {
                    connectButton.enabled = false;
                    connectButton.text = qsTr("Connecting...");
                    shmong.mainConnect(jidTextField.text, passTextField.text);
                }

            }
        }
    }


    Connections {
        target: shmong
        onConnectionStateChanged: {
            if (shmong.connectionState == true) {
                pageStack.replace(pageMenu)
            }
            else {
                connectButton.enabled = true;
                connectButton.text = qsTr("Connect");
            }
        }
    }

    function checkEnableConnectButton() {
        if (jidTextField.text.length > 0 && passTextField.text.length > 0) {
            connectButton.enabled = true;
        }
        else {
            connectButton.enabled = false;
        }
    }
}
