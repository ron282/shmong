#ifndef DISCOINFOHANDLER_H
#define DISCOINFOHANDLER_H

#include <QObject>

class QXmppDiscoveryIq;
class QXmppClient;
class QXmppDiscoveryManager;

class DiscoInfoHandler : public QObject
{
    Q_OBJECT
public:
    explicit DiscoInfoHandler(QObject *parent = 0);

    void setupWithClient(QXmppClient* client);
    void requestInfo();

signals:
    void serverHasMam_(bool);

public slots:
    void discoInfoReceived(const QXmppDiscoveryIq &info);
    void discoItemsReceived(const QXmppDiscoveryIq &items);

private:
    QXmppClient* client_;
    QXmppDiscoveryManager* discoveryManager_ ;
};

#endif // DISCOINFOHANDLER_H
