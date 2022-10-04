#ifndef CHATMARKERS_H
#define CHATMARKERS_H

#include <QObject>

class Persistence;
class RosterController;
class QXmppClient;
class QXmppMessage;

class ChatMarkers : public QObject
{
    Q_OBJECT
public:
    ChatMarkers(Persistence* persistence, RosterController* rosterController, QObject *parent = 0);
    void setupWithClient(QXmppClient *qXmppClient);

    void sendDisplayedForJidAndMessageId(QString jid, QString messageId);
    void sendDisplayedForJid(const QString &jid);

signals:

public slots:
    void handleMessageReceived(const QXmppMessage& message);

private:

    QXmppClient* qXmppClient_;
    Persistence* persistence_;
    RosterController* rosterController_;
};

#endif // CHATMARKERS_H
