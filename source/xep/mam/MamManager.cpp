#include "MamManager.h"
#include "Persistence.h"
#include "XmlWriter.h"
#include "ImageProcessing.h"
#include "DownloadManager.h"

#include "QXmppClient.h"
#include "QXmppMamManager.h"
#include "QXmppUtils.h"
#include "QXmppMamManager.h"

#include <QDateTime>
#include <QUrl>
#include "XmlProcessor.h"
#include <QDebug>
#include <QMimeDatabase>


const QString MamManager::mamNs = "urn:xmpp:mam:2";

MamManager::MamManager(Persistence *persistence, QObject *parent) : QObject(parent),
    serverHasFeature_(false), queridJids_(), persistence_(persistence),
#if 0
    downloadManager_(new DownloadManager(this)), 
#endif
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
        connect(qXmppMamManager_, &QXmppMamManager::resultsRecieved, this, &MamManager::resultsReceived);
        connect(qXmppMamManager_, &QXmppMamManager::archivedMessageReceived, this, &MamManager::archivedMessageReceived);
    }
}

void MamManager::handleConnected()
{
    // reset on each new connect
    queridJids_.clear();
}

void MamManager::resultsReceived(const QString &/*queryId*/, const QXmppResultSetReply &/*resultSetReply*/, bool /*complete*/)
{
    /// This slot is called when all results for a request have been received
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
    qDebug() << "MamManager::setServerHasFeatureMam: " << hasFeature;
    serverHasFeature_ = hasFeature;

    //requestArchiveForJid(client_->configuration().jidBare());
}


void MamManager::requestArchiveForJid(const QString& jid, const QDateTime &from)
{
    if (serverHasFeature_)
    {
        QDateTime start = QDateTime::currentDateTimeUtc().addDays(-14);


        qDebug() << "MamManager::requestArchiveForJid: " << jid;

        if(from.isNull() == false)
        {
            start = from;
        }

        qXmppMamManager_->retrieveArchivedMessages("", // to 
                                                   "", // node
                                                   jid,  
                                                   start, 
                                                   QDateTime::currentDateTimeUtc());
    }
}


// FIXME rewrite me!
// this fails sometimes :-(.
// use custom payload parser to filter out mam messages!

void MamManager::archivedMessageReceived(const QString &queryId, const QXmppMessage &message)
{
    unsigned int security = 0;
    if(message.encryptionMethod() == QXmppMessage::OMEMO)
    {
        security = 1;
    }

    // XEP 280
    bool sentCarbon = false;

    // If this is a carbon message, we need to retrieve the actual content
    if (message.isCarbonForwarded() == true)
    {
        if(client_->configuration().jidBare().compare(QXmppUtils::jidToBareJid(message.from()), Qt::CaseInsensitive) == 0)
        {
            sentCarbon = true;
        }
    }

    if (! message.body().isEmpty())
    {
        QUrl oobUrl(message.outOfBandUrl());
        bool isLink = false;

        QString type = "txt";
        QString messageId = message.id();

        if (! message.outOfBandUrl().isEmpty())  // it's an url
        {
            isLink = true;
            //isLink = security == 1 ? theBody.startsWith("aesgcm://") : isLink;

            if(isLink)
            {
                type = QMimeDatabase().mimeTypeForFile(oobUrl.fileName()).name();

#if 0
                if(! askBeforeDownloading_)
                    downloadManager_->doDownload(oobUrl, messageId); // keep the fragment in the sent message
#endif
            }
        }

        bool isGroupMessage = false;
        if (message.type() == QXmppMessage::GroupChat)
        {
            isGroupMessage = true;
        }

        if (messageId.isEmpty() == true)
        {
            // No message id, try xep 0359
            messageId = message.stanzaId();

            // still empty?
            if (messageId.isEmpty() == true)
            {
                messageId = QString::number(QDateTime::currentMSecsSinceEpoch());
            }
        }

        if (!sentCarbon)
        {
            persistence_->addMessage(messageId,
                                     QXmppUtils::jidToBareJid(message.from()),
                                     QXmppUtils::jidToResource(message.from()),
                                     message.body(), type, 1, security);
        } else
        {
            persistence_->addMessage(messageId,
                                     QXmppUtils::jidToBareJid(message.to()),
                                     QXmppUtils::jidToResource(message.to()),
                                     message.body(), type, 0, security);
        }
    }
}
