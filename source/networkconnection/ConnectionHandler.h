#pragma once

#include <XmppClient.h>

#include <QObject>
#include "QXmppClient.h"

class ReConnectionHandler;
class IpHeartBeatWatcher;
class XmppPingController;

//class XmppPingController;

class ConnectionHandler : public QObject
{
    Q_OBJECT
public:
    explicit ConnectionHandler(QObject *parent = 0);
    ~ConnectionHandler();

    void setupWithClient(QXmppClient* client);
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

private:
    bool connected_;
    bool initialConnectionSuccessfull_;
    bool hasInetConnection_;
    bool appIsActive_;
    QXmppClient::Error clientError_{QXmppClient::Error::NoError};

    QXmppClient* client_{nullptr};
    ReConnectionHandler *reConnectionHandler_;
    IpHeartBeatWatcher *ipHeartBeatWatcher_;
    //XmppPingController *xmppPingController_;
};
