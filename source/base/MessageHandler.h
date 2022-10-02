#pragma once

#include <QObject>
#include <QStringList>

#include "QXmppMessage.h"

#include "Settings.h"

class XmppClient;
class DownloadManager;
class Persistence;
class ChatMarkers;
class RosterController;
class LurchAdapter;
class XMPPMessageParserClient;

class MessageHandler : public QObject
{
    Q_OBJECT
public:
    //MessageHandler(Persistence* persistence, Settings * settings, RosterController* rosterController, LurchAdapter* omemo, QObject *parent = 0);
    MessageHandler(Settings * settings, QObject *parent = 0);

    void setupWithClient(XmppClient* client);
    void sendMessage(QString const &toJid, QString const &message, QString const &type, bool isGroup);
    void sendDisplayedForJid(const QString &jid);
    void downloadFile(const QString &str, const QString &msgId);

signals:
    void messageSent(QString msgId);
    void httpDownloadFinished(QString attachmentMsgId);

public slots:
    void slotAppGetsActive(bool active);
    void sendRawMessageStanza(QString str);
    void setAskBeforeDownloading(bool AskBeforeDownloading);

private:
#ifdef DBUS
public:
#endif
    void handleMessageReceived(const QXmppMessage &message);

    XmppClient* client_;
    Persistence* persistence_;
    LurchAdapter* lurchAdapter_;
    Settings* settings_;

    DownloadManager* downloadManager_;
    ChatMarkers* chatMarkers_;

    XMPPMessageParserClient* xmppMessageParserClient_;

    bool appIsActive_;
    bool askBeforeDownloading_;
    QStringList unAckedMessageIds_;

    //void handleMessageReceived(Swift::Message::ref message);
    //void handleStanzaAcked(Swift::Stanza::ref stanza);
    //void handleDataReceived(Swift::SafeByteArray data);
};
