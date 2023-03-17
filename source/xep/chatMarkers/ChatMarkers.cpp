#include "ChatMarkers.h"
#include "Persistence.h"
#include "RosterController.h"

#include "QXmppUtils.h"
#include "QXmppMessage.h"

#include <QDebug>

ChatMarkers::ChatMarkers(Persistence *persistence, RosterController *rosterController) :
    QXmppClientExtension(), persistence_(persistence), rosterController_(rosterController)
{
}

QStringList ChatMarkers::discoveryFeatures() const
{
    return QStringList("urn:xmpp:chat-markers:0");
}

bool ChatMarkers::handleMessage(const QXmppMessage &message)
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

    //continue processing
    return false;
}

void ChatMarkers::sendDisplayedForJid(const QString& jid)
{
    bool itsAMuc = rosterController_->isGroup(QXmppUtils::jidToBareJid(jid));
    QPair<QString, int> messageIdAndState = persistence_->getNewestReceivedMessageIdAndStateOfJid(jid);

    QString displayedMsgId = messageIdAndState.first;
    int msgState = messageIdAndState.second;

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

        client()->sendPacket(std::move(msg));

        // mark msg as confirmed. no further confirms of that msg
        persistence_->markMessageDisplayedConfirmedId(displayedMsgId);
    }
}
