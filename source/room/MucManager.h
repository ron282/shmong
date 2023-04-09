#ifndef MUCMANAGER_H
#define MUCMANAGER_H

#include <QObject>

class QXmppClient;
class QXmppBookmarkConference;
class QXmppBookmarkSet;
class QXmppMucManager;
class QXmppMucRoom;
class QXmppBookmarkManager;

class MucManager : public QObject
{
    Q_OBJECT

public:
    explicit MucManager(QObject *parent = 0);
    ~MucManager();

    void setupWithClient(QXmppClient* client);
    bool isRoomAlreadyBookmarked(const QString& roomJid);
    void joinRoomIfConfigured(const QXmppBookmarkConference &bookmark);
    QString getNickName();
    void removeRoom(QString const &roomJid);
    void addRoom(QString const &roomJid, QString const &roomName);

signals:
    void newGroupForContactsList(QString groupJid, QString groupName);
    void removeGroupFromContactsList(QString groupJid);
    void roomJoinComplete(QString);
    void signalShowMessage(QString headline, QString body);

public slots:
    void handleInvitationReceived(const QString &roomJid, const QString &inviter, const QString &reason);
    void handleBookmarksReceived(const QXmppBookmarkSet &bookmarks);

private:    

    QXmppMucRoom *getRoom(QString const &roomJid);

    QXmppClient* client_;
    QXmppMucManager *manager_;
    QXmppBookmarkManager *bookmarkManager_;
    bool triggerNewMucSignal_;
};

#endif // MUCMANAGER_H
