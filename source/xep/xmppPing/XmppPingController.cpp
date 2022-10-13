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
//        PingRequest::ref pingRequest = PingRequest::create(Swift::JID(client_->getJID().getDomain()),
//                                                           client_->getIQRouter());
//        pingRequest->onResponse.connect(boost::bind(&XmppPingController::handlePingResponse, this, _1));
//        pingRequest->send();
    }
}

void XmppPingController::handlePingResponse()
{
    qDebug() << QTime::currentTime().toString() << "handlePingResponse: ";
}
