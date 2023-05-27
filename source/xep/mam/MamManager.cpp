#include "MamManager.h"
#include "Persistence.h"
#include "XmlWriter.h"
#include "ImageProcessing.h"
#include "DownloadManager.h"
#include "Settings.h"

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
    if (! queridJids_.contains(jid))
    {
        queridJids_.append(jid);
        requestArchiveForJid(jid);
    }
}

void MamManager::setServerHasFeatureMam(bool hasFeature)
{
    serverHasFeature_ = hasFeature;

    if(serverHasFeature_) {
        requestArchiveForJid(client_->configuration().jidBare());
    }
}

void MamManager::requestArchiveForJid(const QString& jid, const QString &after)
{
    if (serverHasFeature_)
    {
        QDateTime from;
        QString  afterMsgId(after); 

        if(afterMsgId.isEmpty()) {
            afterMsgId = Settings().getMamLastMsgId(jid);

            if(afterMsgId.isEmpty()) {
                from = QDateTime::currentDateTimeUtc().addDays(-14);
            }
        }

        QXmppResultSetQuery resultSetQuery;
        resultSetQuery.setAfter(afterMsgId);

        auto future = qXmppMamManager_->retrieveMessages(QString(), QString(), QString(), from, QDateTime(), resultSetQuery);

        auto processMamResult = [this, jid](QXmppMamManager::RetrieveResult result) {
            auto error = std::get_if<QXmppError>(&result);
            if (error) {
                qWarning() << "Cannot retrieve Mam messages";
            }
            else {
                const auto &retrievedMessages = std::get<QXmppMamManager::RetrievedMessages>(result);
                QVectorIterator<QXmppMessage> it(retrievedMessages.messages);
                
                while(it.hasNext()) {
                    emit mamMessageReceived(it.next());
                }

                if(retrievedMessages.result.complete() == false)
                {
                    requestArchiveForJid(jid, retrievedMessages.result.resultSetReply().last());
                }
                else
                {
                    if(!retrievedMessages.result.resultSetReply().last().isEmpty())
                        Settings().setMamLastMsgId(jid, retrievedMessages.result.resultSetReply().last());
                }
            }
        };

        future.then(this, processMamResult);
    }
}
