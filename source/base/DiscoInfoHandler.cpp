#include "DiscoInfoHandler.h"

#include <QFuture>

#include "QXmppUtils.h"
#include "QXmppClient.h"
#include "QXmppDiscoveryIq.h"
#include "QXmppDiscoveryManager.h"


DiscoInfoHandler::DiscoInfoHandler(QObject *parent) : QObject(parent),
    client_(nullptr), discoveryManager_(nullptr)
{

}

void DiscoInfoHandler::setupWithClient(QXmppClient* client)
{
    if (client != nullptr)
    {
        client_ = client;

        // Discovery Manager
        discoveryManager_ = client_->findExtension<QXmppDiscoveryManager>();

        connect(discoveryManager_, &QXmppDiscoveryManager::infoReceived, this, &DiscoInfoHandler::discoInfoReceived);
        connect(discoveryManager_, &QXmppDiscoveryManager::itemsReceived, this, &DiscoInfoHandler::discoItemsReceived);
    }
}

void DiscoInfoHandler::discoInfoReceived(const QXmppDiscoveryIq &info)
{
    auto features = info.features();

    if(features.contains("urn:xmpp:mam:2"))
    {
        emit serverHasMam_(true);
    }
}

void DiscoInfoHandler::discoItemsReceived(const QXmppDiscoveryIq &items)
{
    auto list = items.items();

    for(int i=0; i<list.size(); i++)
    {
        discoveryManager_->requestInfo(list[i].jid());
    }
}

void DiscoInfoHandler::requestInfo()
{
    discoveryManager_->requestItems(QXmppUtils::jidToDomain(client_->configuration().jid()));
}