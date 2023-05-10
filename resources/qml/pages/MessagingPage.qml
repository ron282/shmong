import QtQuick 2.6
import QtQuick.Window 2.0;
import QtMultimedia 5.6
import Sailfish.Silica 1.0
import harbour.shmong 1.0
import Sailfish.Pickers 1.0
import Nemo.Thumbnailer 1.0

Page {
    id: page;
    allowedOrientations: Orientation.All;

    onStatusChanged: {
        if (status === PageStatus.Active) {
            pageStack.pushAttached(Qt.resolvedUrl("MessagingSettingsPage.qml"),{ "conversationId": conversationId })
        }
        if (status == PageStatus.Deactivating) {
            if (_navigation == PageNavigation.Back) {
                shmong.setCurrentChatPartner("")
            }
        }
    }

    property string conversationId : "";
    property bool isGroup : shmong.rosterController.isGroup(conversationId);
    property string imagePath : shmong.rosterController.getAvatarImagePathForJid(conversationId);
    property bool refreshDate : false;
    property int availability : shmong.rosterController.getAvailability(conversationId)

    Timer {
        interval: 60000; running: true; repeat: true
        onTriggered: {
            refreshDate = !refreshDate;
        }
    }

    Image {
        //source: "image://glass/qrc:///qml/img/photo.png";
        opacity: 0.85;
        sourceSize: Qt.size (Screen.width, Screen.height);
        asynchronous: false
        anchors.centerIn: parent;
    }

    PageHeader {
        id: banner;
        title: trimStr(shmong.rosterController.getNameForJid(conversationId));
        Image {
            id: avatar
            parent: banner.extraContent;
            width: Theme.iconSizeMedium;
            height: width;
            smooth: true;
            source: imagePath != "" ? imagePath : getImage(conversationId);
            fillMode: Image.PreserveAspectCrop;
            antialiasing: true;
            anchors {
                leftMargin: Theme.paddingMedium;
                verticalCenter: parent.verticalCenter;
            }
            Rectangle {
                z: -1;
                color: "black";
                opacity: 0.35;
                anchors.fill: parent;
            }
        }
    }
    SilicaListView {
        id: view;

        verticalLayoutDirection: ListView.BottomToTop;
        clip: true;
        focus: true;
        cacheBuffer: Screen.width // do avoid flickering when image width is changed
        spacing: Theme.paddingMedium

        model: shmong.persistence.messageController
        Component {
            id: sectionHeader
            Label {
                property string section: getMsgDate(new Date (parseInt(parent.ListView.section, 10)))
                property string nextSection: getMsgDate(new Date (parseInt(parent.ListView.nextSection, 10)))
                property bool boundary: section != nextSection

                text: boundary ? section : ""
                color: Theme.highlightColor
                height: text.length > 0 ? Theme.itemSizeExtraSmall : 0
                font.pixelSize: Theme.fontSizeMedium
                anchors.horizontalCenter: parent !== null ? parent.horizontalCenter : undefined
            }
        }

        section.property: "timestamp"
        section.criteria: ViewSection.FullString

        delegate: Item {
            id: wrapper;

            property bool sectionBoundary: ListView.previousSection != ListView.section
            property Item section

            height: section === null ? item.height : item.height + section.height
            width: parent.width

            property string file : shmong.getLocalFileForUrl(model.message)
            readonly property bool isVideo : startsWith(model.type, "video")
            readonly property bool isImage : startsWith(model.type, "image")
            readonly property bool isAudio : startsWith(model.type, "audio")
            readonly property bool isTxt : startsWith(model.type, "txt")
            readonly property bool isFile : ! isTxt

            onSectionBoundaryChanged: {
                if (sectionBoundary) {
                    section = sectionHeader.createObject(wrapper)
                } else {
                    section.destroy()
                    section = null
                }
            }

            ListItem {
                id: item
                y: section ? section.height : 0
                contentHeight: shadow.height;

                anchors {
                    left: parent.left;
                    right: parent.right;
                    margins: Theme.paddingMedium;
                }

                readonly property bool alignRight      : (model.direction == 1);
                readonly property int maxContentWidth : (width * 0.85);
                readonly property int mediaWidth : maxContentWidth * 0.75
                readonly property int mediaHeight : (mediaWidth * 2) / 3

                Rectangle {
                    id: shadow;
                    color: "white";
                    radius: 3;
                    opacity: (item.alignRight ? 0.05 : 0.15);
                    antialiasing: true;
                    anchors {
                        fill: layout;
                        margins: -Theme.paddingSmall;
                    }
                }
                Column {
                    id: layout;

                    anchors {
                        left: (item.alignRight ? parent.left : undefined);
                        right: (!item.alignRight ? parent.right : undefined);
                        margins: -shadow.anchors.margins;
                        verticalCenter: parent.verticalCenter;
                    }

                    Label {
                        text: shmong.addLinks(model.message)
                        textFormat: Text.StyledText
                        linkColor: color
                        width: item.maxContentWidth;
                        wrapMode: Text.WrapAtWordBoundaryOrAnywhere;
                        visible: isTxt

                        font {
                            family: Theme.fontFamilyHeading;
                            pixelSize: Theme.fontSizeMedium;
                        }

                        onLinkActivated: Qt.openUrlExternally(link)
                    }

                    BackgroundItem {

                        id: bkgnd
                        width: thumb.width
                        height: isVideo || isImage ? thumb.height : iconFile.height
                        visible: isFile

                        Thumbnail {
                            id: thumb

                            source: isVideo || isImage ? file : ""
                            mimeType: model.type

                            sourceSize.width: item.mediaWidth
                            sourceSize.height: item.mediaHeight

                            width: implicitWidth > 0 ? implicitWidth : item.mediaWidth
                            height: item.mediaHeight

                            fillMode: Thumbnail.PreserveAspectFit;
                            priority: Thumbnail.NormalPriority

                            Icon {
                                visible: isVideo
                                source: "image://theme/icon-m-file-video"
                                anchors.centerIn : parent
                            }
                        }
                        Row {
                            id: placeholder
                            visible: thumb.status != Thumbnail.Ready
                            Icon {
                                id: iconFile
                                source: getFileIcon(model.type)
                            }
                            Button {
                                visible: file == ""
                                anchors.verticalCenter: iconFile.verticalCenter
                                id: bnDownloadAttachment
                                text: qsTr("Download")
                                onClicked: {
                                    shmong.downloadFile(model.message, model.id);
                                }
                            }
                            Label {
                                visible: thumb.status == Thumbnail.Error
                                anchors.verticalCenter: iconFile.verticalCenter
                                text: qsTr("Impossible to load")
                                color: Theme.primaryColor;
                                width: parent.width;
                                wrapMode: Text.WrapAtWordBoundaryOrAnywhere;
                                font {
                                    family: Theme.fontFamilyHeading;
                                    pixelSize: Theme.fontSizeMedium;
                                }
                            }
                        }
                    }
                    Label {
                        visible: isGroup;
                        color: Theme.secondaryColor;
                        font {
                            family: Theme.fontFamilyHeading;
                            pixelSize: Theme.fontSizeTiny;
                        }
                        text: model.resource;
                    }
                    Row {
                        spacing: 5
                        anchors.right: (!item.alignRight ? parent.right : undefined)

                        Label {
                            id: upload
                            visible: model.msgstate == 4
                            text: qsTr("uploading")
                            color: Theme.secondaryColor;
                            font {
                                family: Theme.fontFamilyHeading;
                                pixelSize: Theme.fontSizeTiny;
                            }

                            Connections {
                                target: shmong
                                onSignalShowStatus: {
                                    if(qsTr(headline) == qsTr("File Upload") && model.msgstate == 4)
                                        upload.text = qsTr("uploading ")+body;
                                }
                                onHttpDownloadFinished: {
                                    if(attachmentMsgId == id)
                                    {
                                        file =  shmong.getLocalFileForUrl(model.message);
                                    }
                                }
                            }
                        }
                        Label {
                            text: qsTr("send failed")
                            visible: msgstate == 5
                            color: Theme.secondaryColor;
                            font {
                                family: Theme.fontFamilyHeading;
                                pixelSize: Theme.fontSizeTiny;
                            }
                        }
                        Label {
                            text: refreshDate, getDateDiffFormated(new Date (timestamp));
                            visible: msgstate != 4
                            color: Theme.secondaryColor;
                            font {
                                family: Theme.fontFamilyHeading;
                                pixelSize: Theme.fontSizeTiny;
                            }
                        }

                        Image {
                            id: chatmarker
                            source: {
                                if (msgstate == 3) {
                                    return "../img/read_green.png"
                                }
                                if (msgstate == 2) {
                                    return "../img/2check.png"
                                }
                                if (msgstate == 1) {
                                    return "../img/check.png"
                                }
                                return ""
                            }
                        }

                        Image {
                            source: {
                                if (model.security == 1) { // omemo
                                    return "image://theme/icon-s-outline-secure"
                                }
                                return ""
                            }
                        }
                    }
                }
                MouseArea {
                    anchors.fill: parent
                    enabled: isFile && thumb.status != Thumbnail.Error && file != ""

                    onClicked: {
                        if (isImage)
                            pageStack.push(Qt.resolvedUrl("ImagePage.qml"),{ 'imgUrl': file })
                        else if (isVideo)
                            pageStack.push(Qt.resolvedUrl("VideoPage.qml"),{ 'path': file })
                        else if (isAudio)
                            pageStack.push(Qt.resolvedUrl("VideoPage.qml"),{ 'path': file });
                    }

                    onPressAndHold: {
                        item.openMenu();
                    }
                }
                menu: ContextMenu {
                    MenuItem {
                        text: qsTr("Copy")
                        visible: isTxt
                        onClicked: Clipboard.text = model.message
                    }
                    MenuItem {
                        text: qsTr("Copy URL")
                        visible: isFile
                        onClicked: Clipboard.text = model.message
                    }
                    MenuItem {
                        text: qsTr("Send again")
                        visible: (model.msgstate == 5 && shmong.canSendFile())
                        onClicked: {
                            shmong.sendFile(conversationId, model.message);
                         }
                    }
                    MenuItem {
                        text: qsTr("Save")
                        visible:  isFile && (model.direction == 1)
                        onClicked: {
                            shmong.saveAttachment(file);
                         }
                    }
                    MenuItem {
                        visible: isGroup;
                        text: qsTr("Status")
                        onClicked:  {
                            shmong.persistence.gcmController.setFilterOnMsg(id);
                            pageStack.push(pageMsgStatus);
                        }
                    }
                }
            }
        }
        anchors {
            top: banner.bottom;
            left: parent.left;
            right: parent.right;
            bottom: displaymsgview.top;
        }
    }
    Row {
        id: displaymsgview

        spacing: 5

        anchors {
            left: parent.left;
            right: parent.right;
            bottom:sendmsgview.top;
            margins: view.spacing;
        }
        Label {
            id: displaymsgviewlabel
            text:  shmong.persistence.getResourcesOfNewestDisplayedMsgforJid(conversationId);
            enabled: isGroup;
            color: Theme.highlightColor;
            font {
                family: Theme.fontFamily;
                pixelSize: Theme.fontSizeTiny;
            }
        }
        Image {
            source: "../img/read_until_green.png";
            visible: displaymsgviewlabel.text.length > 0 ? true: false;
        }
    }

    Row {
        id: sendmsgview

        property var attachmentPath: ""

        anchors {
            left: parent.left;
            leftMargin: Theme.paddingMedium;
            right: sendButton.left;
            bottom: parent.bottom;
        }

        height: (previewAttachment.visible ? 
                 Math.max(editbox.height, previewAttachment.height)+Theme.paddingMedium : 
                 editbox.height+Theme.paddingMedium
                )

        TextArea {
            id: editbox;
            visible: sendmsgview.attachmentPath.length === 0

            property var userHasOmemo: shmong.isOmemoUser(conversationId);
            property var useOmemo: (shmong.settings.SendPlainText.indexOf(conversationId) < 0) ? true : false;

            property var enterMsg: qsTr("Enter message...");
            property var phT: (userHasOmemo && useOmemo) ? "Omemo: " + enterMsg : enterMsg;

            placeholderText: phT;

            font {
                family: Theme.fontFamily
                pixelSize: Theme.fontSizeMedium
            }

            onTextChanged: {
                sendButton.icon.source = getSendButtonImage()
            }

           anchors.bottom: parent.bottom 
           width: parent.width
        }
        Thumbnail {
            id: previewAttachment
            visible: sendmsgview.attachmentPath.length > 0
            width: Math.min(page.width, page.height) / 4    
            height: Math.min(page.width, page.height) / 4
            sourceSize.width: Math.min(page.width, page.height) / 4
            sourceSize.height: Math.min(page.width, page.height) / 4
            anchors {                                                                                                                            
                bottom: parent.bottom; bottomMargin: Theme.paddingMedium                           
            }
            Icon {
                visible: startsWith(previewAttachment.mimeType, "video") && previewAttachment.status != Thumbnail.Error
                source: "image://theme/icon-m-file-video"
                anchors.centerIn: parent
            }
            Icon {
                visible: previewAttachment.status == Thumbnail.Error
                source: getFileIcon(previewAttachment.mimeType)
                anchors.centerIn: parent
            }
            IconButton {
                id: removeAttachment
                anchors.right: parent.right
                anchors.top: parent.top
                icon.source: "image://theme/icon-splus-cancel" 

                onClicked: {
                    sendmsgview.attachmentPath = "";
                    sendButton.icon.source = getSendButtonImage();
                }           
            }
        }
    }
    IconButton {
        id: sendButton

        enabled: {
            if (shmong.connectionState && mainWindow.hasInetConnection) {
                return true
            }
            else {
                return false
            }
        }

        icon.source: getSendButtonImage()
        icon.width: Theme.iconSizeMedium + 2*Theme.paddingSmall                                
        icon.height: width

        anchors {                                                                              
            // icon-m-send has own padding                                                     
            right: parent.right; rightMargin: Theme.horizontalPageMargin-Theme.paddingMedium   
            bottom: parent.bottom; bottomMargin: Theme.paddingMedium                           
        } 
        onClicked: {
            if (editbox.text.length === 0 && sendmsgview.attachmentPath.length === 0 && shmong.canSendFile()) {
                sendmsgview.attachmentPath = ""
                //fileModel.searchPath = shmong.settings.ImagePaths
                pageStack.push(shmong.settings.SendOnlyImages ? imagePickerPage: filePickerPage)
            } else {
                //console.log(sendmsgview.attachmentPath)
                var msgToSend = editbox.text;

                if (sendmsgview.attachmentPath.length > 0) {
                    shmong.sendFile(conversationId, sendmsgview.attachmentPath);
                    sendmsgview.attachmentPath = ""
                }

                if (msgToSend.length > 0) {
                    shmong.sendMessage(conversationId, msgToSend, "txt");
                    editbox.text = " ";
                    editbox.text = "";
                }

                displaymsgviewlabel.text = "";
            }
        }
    }
        
        Component {
            id: filePickerPage
            ContentPickerPage {
            onSelectedContentPropertiesChanged: {
                sendmsgview.attachmentPath = selectedContentProperties.filePath
                sendButton.icon.source = getSendButtonImage()
                previewAttachment.source = sendmsgview.attachmentPath
                previewAttachment.mimeType = selectedContentProperties.mimeType
                }
            }
        }

        Component {
            id: imagePickerPage
            ImagePickerPage {
            onSelectedContentPropertiesChanged: {
                sendmsgview.attachmentPath = selectedContentProperties.filePath
                sendButton.icon.source = getSendButtonImage()
                previewAttachment.source = sendmsgview.attachmentPath
                previewAttachment.mimeType = selectedContentProperties.mimeType
                }
            }
        }

        Connections {
            target: shmong
            onCanSendFile: {
                console.log("HTTP uploads enabled");
                sendButton.icon.source = getSendButtonImage();
            }
        }

    function getSendButtonImage() {
        if (editbox.text.length === 0 && sendmsgview.attachmentPath.length === 0) {
            if (shmong.canSendFile()) {
                return "image://theme/icon-m-attach"
            } else {
                return "image://theme/icon-m-send"
            }
        } else {
                return "image://theme/icon-m-send"
        }
    }

    function basename(str)
    {
        return (str.slice(str.lastIndexOf("/")+1))
    }

    function getImage(jid) {
        if (shmong.rosterController.isGroup(jid)) {
            return "image://theme/icon-l-image";
        } else {
            return "image://theme/icon-l-people"
        }
    }
    
    function trimStr(str){
        var trimmedStr = str;
        if (str.length > 30)
        {
            trimmedStr = str.substring(1, 30);
        }

        trimmedStr = trimmedStr.replace(/(\r\n|\n|\r)/gm,"");

        return trimmedStr;
    }
    function getFileIcon(type){
        if(startsWith(type, "image")) return "image://theme/icon-l-image";
        if(startsWith(type, "video")) return "image://theme/icon-l-play";
        if(startsWith(type, "audio")) return "image://theme/icon-l-music";
        return "image://theme/icon-l-other";
    }
    function startsWith(s,start) {
        return (s.substring(0, start.length) == start); 
    }
    function getMsgDate(d) {
        var n = new Date();

        if(d.getFullYear() == n.getFullYear() &&
           d.getMonth() == n.getMonth() &&
           d.getDate() == n.getDate() )
        {
            return qsTr("Today");
        }
        else
        {
            var locale = Qt.locale();

            if(d.getFullYear() != n.getFullYear())
            {
                return d.toLocaleDateString(locale, "d MMM yyyy");
            }
            else
            {
                return d.toLocaleDateString(locale, "d MMM");
            }
        }
    }
    function getDateDiffFormated(d) {
        var n = new Date();
        var diff = (n.getTime() - d.getTime()) / 1000;
        var locale = Qt.locale();

        if(diff < 0)
            return "?"
        else if(diff < 60)
            return qsTr("now")
        else if(diff < 60*2)
            return qsTr("1 mn ago")
        else if(diff < 60*30)
            return qsTr ("") + Math.round(diff/60, 0)+ qsTr(" mns ago");

        var s = d.toLocaleTimeString(locale, "hh:mm");

        if(d.getFullYear() != n.getFullYear())
        {
            s = d.toLocaleDateString(locale, "d MMM yyyy") + " " + s;
        }
        else if (d.getMonth() != n.getMonth() || d.getDate() != n.getDate())
        {
            s = d.toLocaleDateString(locale, "d MMM") + " " +s;
        }

        return s;
    }
}
