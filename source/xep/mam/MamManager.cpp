#include "MamManager.h"
#include "Persistence.h"
#include "XmlWriter.h"
#include "ImageProcessing.h"
#include "DownloadManager.h"

#include "QXmppClient.h"
#include "QXmppMamManager.h"
#include "QXmppUtils.h"
#include "QXmppMamManager.h"
#include "QXmppTask.h"

#include <QDateTime>
#include <QUrl>
#include "XmlProcessor.h"
#include <QDebug>
#include <QMimeDatabase>


MamManager::MamManager(Persistence *persistence, QObject *parent) : QObject(parent),
    serverHasFeature_(false), queridJids_(), persistence_(persistence),
    client_(nullptr)
{
}

void MamManager::setupWithClient(QXmppClient* client)
{
    if(client != nullptr)
    {
        client_ = client;

        qXmppMamManager_ = new QXmppMamManager;
        client_->addExtension(qXmppMamManager_);

        connect(client_, &QXmppClient::connected, this, &MamManager::handleConnected);
    }
}

void MamManager::handleConnected()
{
    // reset on each new connect
    queridJids_.clear();
}

void MamManager::receiveRoomWithName(QString jid, QString name)
{
    (void) name;
    addJidforArchiveQuery(jid);
}

void MamManager::addJidforArchiveQuery(QString jid)
{
    //qDebug() << "MamManager::addJidforArchiveQuery " << jid;

    if (! queridJids_.contains(jid))
    {
        queridJids_.append(jid);
        requestArchiveForJid(jid);
    }
}

void MamManager::setServerHasFeatureMam(bool hasFeature)
{
    // qDebug() << "MamManager::setServerHasFeatureMam: " << hasFeature;
    serverHasFeature_ = hasFeature;

    requestArchiveForJid(client_->configuration().jidBare());
}

void MamManager::requestArchiveForJid(const QString& jid, const QDateTime &from)
{
    if (serverHasFeature_)
    {
        QDateTime start = QDateTime::currentDateTimeUtc().addDays(-14);

        if(from.isNull() == false)
        {
            start = from;
        }

        qDebug() << "MamManager::requestArchiveForJid: " << jid;
        auto future = qXmppMamManager_->retrieveMessages(QString(), QString(), jid, start, QDateTime::currentDateTimeUtc());

        future.then(this, [this](QXmppMamManager::RetrieveResult result) {
            auto error = std::get_if<QXmppError>(&result);
            if (error) {
                qWarning() << "Cannot retrieve Mam messages";
            }
            else {
                const auto &retrievedMessages = std::get<QXmppMamManager::RetrievedMessages>(result);
                QVectorIterator<QXmppMessage> it(retrievedMessages.messages);
                
                qDebug() << "Mam messages received: " << retrievedMessages.messages.size();

                while(it.hasNext()) {
                    emit mamMessageReceived(it.next());
                }
            } 
        });
    }
}
