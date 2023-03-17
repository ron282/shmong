#ifndef CHATMARKERS_H
#define CHATMARKERS_H

#include <QObject>

#include "QXmppClientExtension.h"
#include "QXmppMessageHandler.h"

class Persistence;
class RosterController;

class ChatMarkers : public QXmppClientExtension, public QXmppMessageHandler
{
    Q_OBJECT
public:
    ChatMarkers(Persistence* persistence, RosterController* rosterController);

    QStringList discoveryFeatures() const override;
    bool handleMessage(const QXmppMessage &) override;
    void sendDisplayedForJid(const QString &jid);

private:

    Persistence* persistence_;
    RosterController* rosterController_;
};

#endif // CHATMARKERS_H
