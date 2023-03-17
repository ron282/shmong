#include "PresenceHandler.h"
#include "RosterController.h"

PresenceHandler::PresenceHandler(RosterController *rosterController) : QObject(rosterController),
    client_(NULL), rosterController_(rosterController)
{

}

void PresenceHandler::setupWithClient(Swift::Client* client)
{
    if (client != NULL)
    {
        client_ = client;
        connect(client_, QXmppClient::presenceReceived, this, PresenceHandler::handlePresenceReceived);
    }
}

void PresenceHandler::handlePresenceReceived(const QXmppPresence & presence)
{
    // Automatically approve subscription requests
    // FIXME show to user and let user decide

    QString jid(QXmppUtils::jidToBareJid(presence.from());

    if(jid != QXmppUtils::jidToBareJid(presence.to()) // only interested in updates of other clients. not our self sent presence msgs
    {
        if (presence.type() == QXmppPresence::Available)
        {
            rosterController_->updateStatusForJid(jid, presence.statusText());
        }

        RosterItem::Availability availability = RosterItem::AVAILABILITY_ONLINE;

        if (presence.type() == QXmppPresence::Unavailable
                || presence.type() == QXmppPresence::Error
                || presence.type() == QXmppPresence::Probe
                || presence.type() == QXmppPresence::Unsubscribe
                || presence.type() == QXmppPresence::Unsubscribed)
        {
            availability = RosterItem::AVAILABILITY_OFFLINE;
        }

        rosterController_->updateAvailabilityForJid(jid, availability);

        if (presence.type() == QXmppPresence::Subscribe)
        {
            QXmppPresence answer();

            // accept subscription
            asnwer.setType(QXmppPresence::Subscribed);
            answer.setFrom(presence.to());
            answer.setTo(presence.from());
            client_->send(answer);

            // request subscription
            asnwer.setType(QXmppPresence::Subscribe);
            answer.setFrom(presence.to());
            answer.setTo(presence.from());
            client_->send(answer);
        }
    }
}

