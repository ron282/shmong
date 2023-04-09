#include "MucManager.h"

#include "QXmppClient.h"
#include "QXmppBookmarkManager.h"
#include "QXmppBookmarkSet.h"
#include "QXmppMucManager.h"

#include <QDateTime>
#include <QDebug>
#include <iostream>


MucManager::MucManager(QObject *parent) :
    QObject(parent), client_(nullptr), triggerNewMucSignal_(true)
{
}

MucManager::~MucManager()
{
}

void MucManager::setupWithClient(QXmppClient* client)
{
    if (client != nullptr)
    {
        client_ = client;
        
        manager_ =  new QXmppMucManager();
        client_->addExtension(manager_);

        bookmarkManager_ = new QXmppBookmarkManager();
        client_->addExtension(bookmarkManager_);

        connect(manager_, &QXmppMucManager::invitationReceived, this, &MucManager::handleInvitationReceived);
        connect(bookmarkManager_, &QXmppBookmarkManager::bookmarksReceived, this, &MucManager::handleBookmarksReceived);
    }
}

void MucManager::handleInvitationReceived(const QString &roomJid, const QString &inviter, const QString &reason)
{
    this->addRoom(roomJid, reason);
}

void MucManager::handleBookmarksReceived(const QXmppBookmarkSet &bookmarks)
{
    qDebug() << "##################### handleBookmarksReceived ######################";

    auto conferences = bookmarks.conferences();

    for(int i=0; i<conferences.size(); i++)
    {
        qDebug() << "rooms: jid:" << conferences[i].jid() << ", name:" <<  conferences[i].name();

        if (triggerNewMucSignal_ == true)
        {
            qDebug() << "Signal new room. jid: " << conferences[i].jid() << ", name:" << conferences[i].name() << endl; 
            emit newGroupForContactsList( conferences[i].jid(), conferences[i].name());
        }

        // maybee join room
        joinRoomIfConfigured(conferences[i]);
    }
}

bool MucManager::isRoomAlreadyBookmarked(const QString& roomJid)
{
    qDebug() << "isRoomAlreadyBookmarked?: " << roomJid;
    bool returnValue = false;

    auto conferences = bookmarkManager_->bookmarks().conferences();

    for(int i=0; i<conferences.size(); i++)
    {
        if (roomJid.compare(conferences[i].jid(), Qt::CaseInsensitive) == 0)
        {
            returnValue = true;
            break;
        }
    }

    qDebug() << "... " << returnValue;
    return returnValue;
}

void MucManager::joinRoomIfConfigured(const QXmppBookmarkConference &bookmark)
{
    // join room if autoJoin
    if (bookmark.autoJoin())
    {
        QXmppMucRoom *room = manager_->addRoom(bookmark.name());
        QString nickName = bookmark.nickName();

        if(nickName.isEmpty())
            room->setNickName(getNickName());
        else
            room->setNickName(nickName);

        room->join();
    }
}

QString MucManager::getNickName()
{
    // FIXME get name from settings page
    QString nick = client_->configuration().jidBare();
    nick.replace("@", "(at)");

    return nick;
}


void MucManager::addRoom(QString const &roomJid, QString const &roomName)
{
    QString nickName = getNickName();

    QXmppMucRoom *room = manager_->addRoom(roomJid);
    room->setNickName(nickName);
    room->join();

    // save as bookmark if not already in
    if (isRoomAlreadyBookmarked(roomJid) == false)
    {
        auto bookmarks = bookmarkManager_->bookmarks();
        QXmppBookmarkConference bookmark;

        bookmark.setAutoJoin(true);
        bookmark.setNickName(nickName);
        bookmark.setName(roomName);
        bookmark.setJid(roomJid);

        bookmarks.conferences().append(bookmark);
         
        bookmarkManager_->setBookmarks(bookmarks);     
    }
}

QXmppMucRoom *MucManager::getRoom(QString const &roomJid)
{
    auto rooms = manager_->rooms();

    for(int i=0; i<rooms.size(); i++)
    {
        if(rooms[i]->jid().compare(roomJid, Qt::CaseInsensitive))
            return rooms[i];
    }   

    return nullptr;
}

void MucManager::removeRoom(QString const &roomJid)
{
    auto room =  getRoom(roomJid);

    if(room != nullptr)
    {
        room->leave({});
    }

    auto bookmarks = bookmarkManager_->bookmarks();
    auto rooms = bookmarks.conferences();

    for (int i = 0; i < rooms.size(); i++)
    {
        if(rooms[i].jid().compare(roomJid, Qt::CaseInsensitive) == 0)
        {
            rooms.removeAt(i);
            bookmarks.setConferences(rooms);
            bookmarkManager_->setBookmarks(bookmarks);
            break;
        }
    }

    emit removeGroupFromContactsList( roomJid );
}
