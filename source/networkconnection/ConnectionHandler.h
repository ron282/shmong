#pragma once

#include <XmppClient.h>

#include <QObject>

class ReConnectionHandler;
class IpHeartBeatWatcher;
//class XmppPingController;

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
    void handleConnected();
    void handleDisconnected();
    void errorReceived(QXmppClient::Error error);

private:
    bool connected_;
    bool initialConnectionSuccessfull_;
    bool hasInetConnection_;
    bool appIsActive_;
    QXmppClient::Error clientError_{QXmppClient::Error::NoError};

    XmppClient* client_{nullptr};
    ReConnectionHandler *reConnectionHandler_;
    IpHeartBeatWatcher *ipHeartBeatWatcher_;
    //XmppPingController *xmppPingController_;
};
