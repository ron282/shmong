#include "XmppPingController.h"
#include "PingRequest.h"
#include "XmppClient.h"

#include <QTime>
#include <QDebug>

XmppPingController::XmppPingController() : client_(NULL)
{
}

void XmppPingController::setupWithClient(XmppClient* client)
{
    client_ = client;
}

void XmppPingController::doPing()
{
    if (client_ != NULL)
    {
//      QXmppPingRequest pingRequest;
//      client_->sendId(pingRequest);
    }
}

void XmppPingController::handlePingResponse()
{
    qDebug() << QTime::currentTime().toString() << "handlePingResponse: ";
}
