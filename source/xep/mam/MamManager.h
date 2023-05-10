#ifndef MAMMANAGER_H
#define MAMMANAGER_H

#include <QObject>
#include <QStringList>

#include "QXmppMessage.h"
#include "QXmppResultSet.h"

class Persistence;
class QXmppClient;
class QXmppMamManager;

// https://xmpp.org/extensions/xep-0313.html
// requests the mam for the client jid as soon as mamNs is discovered with disco#info
// adds all bookmarks to the jids list to only query the archive one time
// requests mam for the jids in that list
class MamManager : public QObject
{
    Q_OBJECT
public:
    explicit MamManager(Persistence* persistence, QObject *parent = nullptr);
    void setupWithClient(QXmppClient* client);

    Q_SIGNALS:
    void mamMessageReceived(const QXmppMessage &message);

#ifndef UNIT_TEST
private:
#endif
    void requestArchiveForJid(const QString& jid, const QDateTime &from = QDateTime(), const QString &after = {});

    bool serverHasFeature_;
    QStringList queridJids_;

    Persistence* persistence_;
    QXmppClient* client_;
    QXmppMamManager *qXmppMamManager_;

public slots:
    void receiveRoomWithName(QString jid, QString name);
    void addJidforArchiveQuery(QString jid);
    void setServerHasFeatureMam(bool hasFeature);

private slots:
    void handleConnected();

};

#endif // MAMMANAGER_H
