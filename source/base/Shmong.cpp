#include "Shmong.h"

#include <iostream>

#include <QtConcurrent>
#include <QDateTime>
#include <QUrl>
#include <QMimeDatabase>

#include <QTimer>

#include <QDebug>

#include "RosterController.h"
#include "Persistence.h"
#include "OmemoController.h"
#include "MessageController.h"
#include "ConnectionHandler.h"
#include "CryptoHelper.h"

#include "HttpFileUploadManager.h"
#include "ConnectionHandler.h"
#include "DiscoInfoHandler.h"
#include "MucManager.h"
#include "Settings.h"

#include "MessageHandler.h"
#include "MamManager.h"
#include "QXmppLogger.h"
#include "QXmppUtils.h"
#include "QXmppGlobal.h"
#include "QXmppMessage.h"
#include "QXmppTrustManager.h"
#include "QXmppTrustMemoryStorage.h"
#include "QXmppAtmTrustMemoryStorage.h"
#include "QXmppAtmManager.h"
#include "QXmppOmemoManager.h"
#include "QXmppOmemoMemoryStorage.h"
#include "QXmppPubSubManager.h"
#include "QXmppCarbonManagerV2.h"
#include "QXmppClient.h"
#include "QXmppPubSubItem.h"
#include "QXmppUploadRequestManager.h"
#include "QXmppMessageReceiptManager.h"

#include "System.h"

Shmong::Shmong(QObject *parent) : QObject(parent),
    client_(new QXmppClient(this)),
    rosterController_(new RosterController(this)),
    persistence_(new Persistence(this)),
    settings_(new Settings(this)),
    connectionHandler_(new ConnectionHandler(this)),
    httpFileUploadManager_(new HttpFileUploadManager(this)),
    messageHandler_(new MessageHandler(persistence_, settings_, rosterController_, this)),
    mamManager_(new MamManager(persistence_, this)),
    mucManager_(new MucManager(this)),
    discoInfoHandler_(new DiscoInfoHandler(this)),
    jid_(""), password_(""),
    version_("0.1.0"),
    notSentMsgId_(""),
    omemoLoaded_(false)
{
    qApp->setApplicationVersion(version_);

    connect(connectionHandler_, &ConnectionHandler::signalInitialConnectionEstablished, this, &Shmong::intialSetupOnFirstConnection);
    connect(httpFileUploadManager_, SIGNAL(fileUploadedForJidToUrl(QString,QString,QString)),
            this, SLOT(fileUploaded(QString,QString,QString)));
    connect(httpFileUploadManager_, SIGNAL(fileUploadFailedForJidToUrl()), 
            this, SLOT(attachmentUploadFailed()));

    connect(mucManager_, SIGNAL(newGroupForContactsList(QString,QString)), rosterController_, SLOT(addGroupAsContact(QString,QString)));
    connect(mucManager_, SIGNAL(removeGroupFromContactsList(QString)), rosterController_, SLOT(removeGroupFromContacts(QString)) );

    connect(httpFileUploadManager_, SIGNAL(serverHasHttpUpload_(bool)), this, SIGNAL(signalCanSendFile(bool)));
    connect(discoInfoHandler_, SIGNAL(serverHasMam_(bool)), mamManager_, SLOT(setServerHasFeatureMam(bool)));
#if 0
    connect(mucManager_, SIGNAL(newGroupForContactsList(QString,QString)), mamManager_, SLOT(receiveRoomWithName(QString, QString)));
#endif

    // send read notification if app gets active
    connect(this, SIGNAL(signalAppGetsActive(bool)), this, SLOT(sendReadNotification(bool)));

    // inform connectionHandler and messageHandler about app status
    connect(this, SIGNAL(signalAppGetsActive(bool)), connectionHandler_, SLOT(slotAppGetsActive(bool)));
    connect(this, SIGNAL(signalAppGetsActive(bool)), messageHandler_, SLOT(slotAppGetsActive(bool)));

    // proxy signal to qml ui
    connect(connectionHandler_, SIGNAL(signalHasInetConnection(bool)), this, SIGNAL(signalHasInetConnection(bool)));
    connect(connectionHandler_, SIGNAL(connectionStateChanged()), this, SIGNAL(connectionStateChanged()));

#if 0
    // show errors to user
    connect(mucManager_, SIGNAL(signalShowMessage(QString,QString)), this, SIGNAL(signalShowMessage(QString,QString)));
    connect(rosterController_, SIGNAL(signalShowMessage(QString,QString)), this, SIGNAL(signalShowMessage(QString,QString)));
#endif
    // show status to user
    connect(httpFileUploadManager_, SIGNAL(showStatus(QString, QString)), this, SIGNAL(signalShowStatus(QString, QString)));

    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(slotAboutToQuit()));
    connect(settings_, SIGNAL(compressImagesChanged(bool)), httpFileUploadManager_, SLOT(setCompressImages(bool)));
    connect(settings_, SIGNAL(limitCompressionChanged(unsigned int)), httpFileUploadManager_, SLOT(setLimitCompression(unsigned int)));
    connect(messageHandler_, SIGNAL(httpDownloadFinished(QString)), this, SIGNAL(httpDownloadFinished(QString)));
}

Shmong::~Shmong()
{
    qDebug() << "Shmong::~Shmong";

    if (connectionHandler_->isConnected())
    {
        delete client_;
    }
}

void Shmong::slotAboutToQuit()
{
    if (connectionHandler_->isConnected())
    {
        client_->disconnectFromServer();
    }
}

void Shmong::mainConnect(const QString &jid, const QString &pass)
{
    const QObject context;

    qDebug() << "main connect " << jid;
    persistence_->openDatabaseForJid(jid);

    QString resourceName;

    resourceName = QString("Shmong.") + System::getUniqueResourceId();
    QString completeJid = jid + "/" + resourceName;

#ifndef SFOS
    completeJid += "Desktop";
    setHasInetConnection(true);
#endif

    // setup the xmpp client

    connectionHandler_->setupWithClient(client_);
    messageHandler_->setupWithClient(client_);
    discoInfoHandler_->setupWithClient(client_);
    mucManager_->setupWithClient(client_);
    httpFileUploadManager_->setupWithClient(client_);
    rosterController_->setupWithClient(client_);
    mamManager_->setupWithClient(client_);
    connect(mamManager_, &MamManager::mamMessageReceived, messageHandler_, &MessageHandler::handleMessageReceived);

    client_->logger()->setLoggingType(QXmppLogger::StdoutLogging);

    pubsubManager_ = new QXmppPubSubManager();
    client_->addExtension(pubsubManager_);

    // Omemo
    if (settings_->getSoftwareFeatureOmemoEnabled() == true)
    {
        // Need to add a persistent storage
        trustStorage_ = new QXmppTrustMemoryStorage();
        trustManager_ = new QXmppTrustManager(trustStorage_);
        client_->addExtension(trustManager_);
        
        omemoManager_ = new QXmppOmemoManager(persistence_->getOmemoController());
        client_->addExtension(omemoManager_);

        omemoManager_->setSecurityPolicy(QXmpp::TrustSecurityPolicy::NoSecurityPolicy);
        omemoManager_->setNewDeviceAutoSessionBuildingEnabled(true);
        omemoManager_->setAcceptedSessionBuildingTrustLevels(ANY_TRUST_LEVEL);

        auto future = omemoManager_->load();
        future.then(this, [=](bool isLoaded) {
            if(isLoaded == false) {
                qDebug() << "Error loading Omemo data" << endl; 
            }
            omemoLoaded_ = isLoaded;
        });
    }

    QXmppConfiguration config;

    config.setJid(jid);
    config.setPassword(pass);
    config.setResource(resourceName);

    client_->connectToServer(config);

    // for saving on connection success
    if (settings_->getSaveCredentials() == true)
    {
        jid_ = jid;
        password_ = pass;
    }

    httpFileUploadManager_->setCompressImages(settings_->getCompressImages());
    httpFileUploadManager_->setLimitCompression(settings_->getLimitCompression());
}

void Shmong::mainDisconnect()
{
    if (connectionState())
    {
        client_->disconnect();
    }
}

void Shmong::reConnect()
{
#if 0
    if (client_ != nullptr)
    {
        client_->connect();
    }
#endif
}

void Shmong::omemoResetAll()
{
    if(omemoManager_ != nullptr)
    {
        omemoManager_->resetAll();
    }    
}

void Shmong::intialSetupOnFirstConnection()
{
    if(omemoLoaded_ == false && settings_->getSoftwareFeatureOmemoEnabled() == true) {
        omemoManager_->setUp();
    }

    discoInfoHandler_->requestInfo();

    // Save account data
    settings_->setJid(jid_);
    settings_->setPassword(password_);
}

void Shmong::setCurrentChatPartner(QString const &jid)
{
    persistence_->setCurrentChatPartner(jid);

    sendReadNotification(true);
}

QString Shmong::getCurrentChatPartner()
{
    return persistence_->getCurrentChatPartner();
}

void Shmong::sendMessage(QString const &toJid, QString const &message, QString const &type)
{
    bool isGroup = rosterController_->isGroup(toJid);
    messageHandler_->sendMessage(toJid, message, type, isGroup);
}

void Shmong::sendMessage(QString const &message, QString const &type)
{
    const QString toJid = getCurrentChatPartner();

    if (! toJid.isEmpty())
    {
        bool isGroup = rosterController_->isGroup(toJid);
        messageHandler_->sendMessage(toJid, message, type, isGroup);
    }
    else
    {
        qDebug() << "tried to send msg without current chat partner selected!";
    }
}


void Shmong::sendFile(QString const &toJid, QString const &file)
{
    bool shouldEncryptFile = settings_->getSoftwareFeatureOmemoEnabled() && (! settings_->getSendPlainText().contains(toJid));
    notSentMsgId_ = QXmppUtils::generateStanzaUuid();

    // messsage is added to the database 
    persistence_->addMessage( notSentMsgId_,
                              QXmppUtils::jidToBareJid(toJid),
                              QXmppUtils::jidToResource(toJid),
                              file, QMimeDatabase().mimeTypeForFile(file).name(), 0, shouldEncryptFile ? 1 : 0);

    persistence_->markMessageAsUploadingAttachment(notSentMsgId_);

    bool success = httpFileUploadManager_->requestToUploadFileForJid(file, toJid, shouldEncryptFile);

    if(!success)
    {
        persistence_->markMessageAsSendFailed(notSentMsgId_);
    }
}

void Shmong::sendFile(QUrl const &file)
{
    const QString toJid = getCurrentChatPartner();
    QString localFile = file.toLocalFile();

    sendFile(toJid, localFile);
}


void Shmong::sendReadNotification(bool active)
{
    QString currentChatPartner = persistence_->getCurrentChatPartner();

    if (active == true && (! currentChatPartner.isEmpty()))
    {
        messageHandler_->sendDisplayedForJid(currentChatPartner);
    }
}

RosterController* Shmong::getRosterController()
{
    return rosterController_;
}

Persistence* Shmong::getPersistence()
{
    return persistence_;
}

Settings* Shmong::getSettings()
{
    return settings_;
}

bool Shmong::connectionState() const
{
    return client_->isConnected();
}

bool Shmong::canSendFile()
{
    return httpFileUploadManager_->getServerHasFeatureHttpUpload();
}

bool Shmong::isOmemoUser(const QString& jid)
{
    /*
    bool returnValue;

    auto future = omemoManager_->devices(QStringList(jid));
    await(future, this, [=, &returnValue](QVector<QXmppOmemoDevice> devices)) {
     returnValue == devices.isEmpty() == false;
    }
    return returnValue;
    */
    return true;
}

QString Shmong::getAttachmentPath()
{
    return System::getAttachmentPath();
}

QString Shmong::getLocalFileForUrl(const QString& str)
{
    QUrl url(str);

    if(url.isRelative())
    {
        return str;
    }
    else
    {
        QString localFile;

        localFile = System::getAttachmentPath() + QDir::separator() + CryptoHelper::getHashOfString(url.toString(), true);
        if(QFile::exists(localFile) && QFileInfo(localFile).size() > 0)
            return localFile;
        else
            return "";
    }
}

void Shmong::downloadFile(const QString& str, const QString& msgId)
{
    messageHandler_->downloadFile(str, msgId);
}

void Shmong::setHasInetConnection(bool connected)
{
    connectionHandler_->setHasInetConnection(connected);
}

void Shmong::setAppIsActive(bool active)
{
    emit signalAppGetsActive(active);
}

QString Shmong::getVersion()
{
    return version_;
}

void Shmong::joinRoom(QString const &roomJid, QString const &roomName)
{
    //TODO Check jid is valid
    setCurrentChatPartner(roomJid); // this prevents notifications for each initial history message
    mucManager_->addRoom(roomJid, roomName);
}

void Shmong::removeRoom(QString const &roomJid)
{
    mucManager_->removeRoom(roomJid);
}

void Shmong::attachmentUploadFailed()
{
    persistence_->markMessageAsSendFailed(notSentMsgId_);
    notSentMsgId_ = "";
}

void Shmong::saveAttachment(const QString& msg)
{
    //TODO Error management + Destination selection
    QFile::copy(getLocalFileForUrl(msg), QStandardPaths::locate(QStandardPaths::DownloadLocation, "", QStandardPaths::LocateDirectory)  +
                    QDir::separator() + CryptoHelper::getHashOfString(QUrl(msg).toString(), true));
}

void Shmong::fileUploaded(QString const&toJid, QString const&message, QString const&type)
{
    persistence_->removeMessage(notSentMsgId_, toJid);
    notSentMsgId_ = "";
    sendMessage(toJid, message, type);
}

unsigned int Shmong::getMaxUploadSize()
{
    return httpFileUploadManager_->getMaxFileSize();
}

QString Shmong::addLinks(const QString &str)
{
    QStringList list=str.split(' ');

    QStringList::iterator it;

    for (it = list.begin(); it != list.end(); it++)
    {
        if((*it).contains('.') && (*it).endsWith('.') == false && (*it).contains("..") == false)
        {
            QUrl url(*it, QUrl::StrictMode);

            if(url.isValid())
            {
                if(url.isRelative())
                {
                    (*it) = "<a href=\"http://" + (*it) + "\">" + (*it) +"</a>";
                }
                else
                {
                    (*it) = "<a href=\"" + (*it) + "\">" + (*it) +"</a>";
                }
            }
        }
    }

    return list.join(' ');
}

