#ifndef CONNECTIONHANDLER_H
#define CONNECTIONHANDLER_H

#include <QObject>
#include "QXmppClient.h"

class ReConnectionHandler;
class IpHeartBeatWatcher;
class XmppPingController;
class XmppClient;

class ConnectionHandler : public QObject
{
    Q_OBJECT
public:
    explicit ConnectionHandler(QObject *parent = 0);
    ~ConnectionHandler();

    void setupWithClient(XmppClient* client);
    void setHasInetConnection(bool connected);

    bool isConnected();

signals:
    void connectionStateChanged();
    void signalHasInetConnection(bool connected);

    void signalInitialConnectionEstablished();

public slots:
    void tryStablishServerConnection();
    void slotAppGetsActive(bool active);

private slots:
    void tryReconnect();

private:
    void handleStateChanged(QXmppClient::State state);
    void enableMessageCarbons();

    bool connected_;
    bool initialConnectionSuccessfull_;
    bool hasInetConnection_;
    bool appIsActive_;

    XmppClient* client_{nullptr};
    ReConnectionHandler *reConnectionHandler_;
    IpHeartBeatWatcher *ipHeartBeatWatcher_;
    XmppPingController *xmppPingController_;
};

#endif // CONNECTIONHANDLER_H
