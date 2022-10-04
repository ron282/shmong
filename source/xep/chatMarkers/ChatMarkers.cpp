#include "ChatMarkers.h"
#include "Persistence.h"
#include "RosterController.h"

#include "QXmppClient.h"
#include "QXmppUtils.h"
#include "QXmppMessage.h"

#include <QDebug>

ChatMarkers::ChatMarkers(Persistence *persistence, RosterController *rosterController, QObject *parent) : QObject(parent),
    qXmppClient_(NULL), persistence_(persistence), rosterController_(rosterController)
{
}

void ChatMarkers::setupWithClient(QXmppClient *qXmppClient)
{
    if (qXmppClient != NULL)
    {
        qXmppClient_ = qXmppClient;
        connect(qXmppClient_, &QXmppClient::messageReceived, this, &ChatMarkers::handleMessageReceived);
    }
}


void ChatMarkers::handleMessageReceived(const QXmppMessage &message)
{
    //  First handle the received stanza
    if (message.marker() != QXmppMessage::NoMarker)
    {
        if (message.markedId().isEmpty() == false)
        {
            QString msgJid = message.from();
            if (rosterController_->isGroup(QXmppUtils::jidToBareJid(msgJid)))
            {
                // add to received list for that msg in the group
                QString groupChatMember = QXmppUtils::jidToResource(msgJid);
                persistence_->markGroupMessageReceivedByMember(message.markedId(), groupChatMember);
            }
            else
            {
                persistence_->markMessageAsReceivedById(message.markedId());
            }
        }
    }

    if (message.marker() == QXmppMessage::Displayed)
    {
        if (message.markedId().isEmpty() == false)
        {
            QString msgJid = message.from();
            if (rosterController_->isGroup(QXmppUtils::jidToBareJid(msgJid)))
            {
                // add to received list for that msg in the group
                QString groupChatMember = QXmppUtils::jidToResource(msgJid);
                persistence_->markGroupMessageReceivedByMember(message.markedId(), groupChatMember);
            }
            else
            {
                persistence_->markMessageAsDisplayedId(message.markedId());
            }
        }
    }
}

void ChatMarkers::sendDisplayedForJid(const QString& jid)
{
    bool itsAMuc = rosterController_->isGroup(QXmppUtils::jidToBareJid(jid));
    QPair<QString, int> messageIdAndState = persistence_->getNewestReceivedMessageIdAndStateOfJid(jid);

    QString displayedMsgId = messageIdAndState.first;
    int msgState = messageIdAndState.second;

    //qDebug() << "id: " << displayedMsgId << ", state: " << msgState;

    if ( ( msgState != -1 ) && (! displayedMsgId.isEmpty()) && (! jid.isEmpty()) )
    {
        QXmppMessage msg("", jid);

        msg.setMarkerId(displayedMsgId);
        msg.setMarker(QXmppMessage::Displayed);;

        if (itsAMuc == true)
        {
            msg.setTo(QXmppUtils::jidToBareJid(jid));
            msg.setType(QXmppMessage::GroupChat);

            if (jid.compare(QXmppUtils::jidToBareJid(jid), Qt::CaseInsensitive) == 0)
            {
                msg.setFrom(jid + "/" + persistence_->getResourceForMsgId(displayedMsgId));
            }
        }
        else    
        {
            msg.setTo(jid);
            msg.setType(QXmppMessage::Normal);
        }

        if(qXmppClient_ != nullptr)
        {
            qXmppClient_->sendPacket(msg);
        }

        // mark msg as confirmed. no further confirms of that msg
        persistence_->markMessageDisplayedConfirmedId(displayedMsgId);
    }
}
