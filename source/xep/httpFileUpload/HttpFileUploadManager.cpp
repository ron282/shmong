#include "HttpFileUploadManager.h"
#include "XmlHttpUploadContentHandler.h"
#include "HttpFileuploader.h"
#include "ImageProcessing.h"
#include "System.h"
#include "CryptoHelper.h"
#include "FileWithCypher.h"
#include "Shmong.h"

#include <QFile>
#include <QFileInfo>
#include <QXmlSimpleReader>
#include <QStandardPaths>
#include <QDir>
#include <QDateTime>
#include <QFileInfo>
#include <QMimeDatabase>

#include <QDebug>

#include "QXmppClient.h"
#include "QXmppHttpUploadIq.h"
#include "QXmppUploadRequestManager.h"


HttpFileUploadManager::HttpFileUploadManager(QObject *parent) : QObject(parent),
    httpUpload_(new HttpFileUploader(this)),
    serverHasFeatureHttpUpload_(false), maxFileSize_(0),
    file_(new FileWithCypher(this)), jid_(""), client_(nullptr),
    uploadServerJid_(""), statusString_(""), getUrl_(""), busy_(false), compressImages_(false), limitCompression_(400000u)
{
    connect(httpUpload_, SIGNAL(updateStatus(QString)), this, SLOT(updateStatusString(QString)));
    connect(httpUpload_, SIGNAL(uploadSuccess(QString)), this, SLOT(successReceived(QString)));
    connect(httpUpload_, SIGNAL(errorOccurred()), this, SLOT(errorReceived()));

    connect(httpUpload_, SIGNAL(updateStatus(QString)), this, SLOT(generateStatus(QString)));

    busy_ = (! this->createAttachmentPath());
}

void HttpFileUploadManager::setupWithClient(QXmppClient* client)
{
    client_ = client;
    uploadRequestManager_ = new QXmppUploadRequestManager();
    client->addExtension(uploadRequestManager_);

    connect(uploadRequestManager_, &QXmppUploadRequestManager::serviceFoundChanged, this, &HttpFileUploadManager::handleServiceFoundChanged);
}

void HttpFileUploadManager::handleServiceFoundChanged()
{
    qDebug() << "upload service found" << endl;
    
    auto uploadServices = uploadRequestManager_->uploadServices();

    if(uploadServices.size() > 0)
    {
        setServerHasFeatureHttpUpload(true);
        setUploadServerJid(uploadServices[0].jid());
        setMaxFileSize(uploadServices[0].sizeLimit());

        emit serverHasHttpUpload_(true);
    }
}

bool HttpFileUploadManager::requestToUploadFileForJid(const QString &file, const QString &jid, bool encryptFile)
{
    bool returnValue = false;

    if (busy_ == false && client_ != nullptr && serverHasFeatureHttpUpload_ == true)
    {
        QFile inputFile(file);
        QString fileToUpload = createTargetFileName(file);

        fileType_ = QMimeDatabase().mimeTypeForFile(file).name();

        // Resize image if server cannot handle it or if requested
        if(fileType_.startsWith("image"))
        {
            if((inputFile.size() > getMaxFileSize()) || (compressImages_ && inputFile.size() > limitCompression_))
            {
                fileToUpload = createTargetFileName(file, "JPG");
                fileType_ = QMimeDatabase().mimeTypeForFile(file).name();

                returnValue = ImageProcessing::prepareImageForSending(file, fileToUpload, compressImages_ ? limitCompression_ : getMaxFileSize());
            }
            else
            {
                returnValue = inputFile.copy(fileToUpload);
            }
        }
        else if(inputFile.size() <= getMaxFileSize() || getMaxFileSize() == 0)
            returnValue = inputFile.copy(fileToUpload);
        else
            returnValue = false;

        if(returnValue)
        {
            busy_ = true;
            encryptFile_ = encryptFile;
            returnValue = true;

            file_->setFileName(fileToUpload);
            jid_ = jid;

            requestHttpUploadSlot();
        }
    }

    return returnValue;
}

void HttpFileUploadManager::updateStatusString(QString string)
{
    statusString_ = string;
}

QString HttpFileUploadManager::getStatus()
{
    return statusString_;
}

void HttpFileUploadManager::setServerHasFeatureHttpUpload(bool hasFeature)
{
    serverHasFeatureHttpUpload_ = hasFeature;
}


bool HttpFileUploadManager::getServerHasFeatureHttpUpload()
{
    return serverHasFeatureHttpUpload_;
}

void HttpFileUploadManager::setUploadServerJid(QString const & uploadServerJid)
{
    uploadServerJid_ = uploadServerJid;
}

QString HttpFileUploadManager::getUploadServerJid()
{
    return uploadServerJid_;
}

void HttpFileUploadManager::setMaxFileSize(unsigned int maxFileSize)
{
    maxFileSize_ = maxFileSize;
}

unsigned int HttpFileUploadManager::getMaxFileSize()
{
    return maxFileSize_;
}

void HttpFileUploadManager::requestHttpUploadSlot()
{
    if (client_ != nullptr)
    {
        auto uploadRequestManager = client_->findExtension<QXmppUploadRequestManager>();

        QString basename = QFileInfo(file_->fileName()).baseName() + "." + QFileInfo(file_->fileName()).completeSuffix();

        if(!file_->initEncryptionOnRead(encryptFile_))
        {
            //TODO delete file and signal failure
            qWarning() << "error on init encryption for " << file_->fileName();
        }

        auto future = uploadRequestManager->requestSlot(basename, file_->size(), QMimeDatabase().mimeTypeForFile(*file_), uploadServerJid_);
        await(future, this, [=](QXmppUploadRequestManager::SlotResult result) mutable {
            if (std::get_if<QXmppStanza::Error>(&result)) {
                qWarning() << "Cannot request slot";
            }
            else {
                const auto slot = std::get<QXmppHttpUploadSlotIq>(result);

                if (slot.putUrl().isValid() && slot.getUrl().isValid())
                {
                    QUrl url(slot.getUrl());

                    if( file_->getIvAndKey().size() > 0 )
                    {
                        url.setScheme("aesgcm");
                        url.setFragment(file_->getIvAndKey());
                    }

                    getUrl_ = url.toString(); 
                    QString attachmentFileName = System::getAttachmentPath() + QDir::separator() +  CryptoHelper::getHashOfString(getUrl_, true);

                    if(! file_->rename(attachmentFileName))
                    {
                        qWarning() << "failed to rename file to " << attachmentFileName;
                        emit fileUploadFailedForJidToUrl();
                    }
                    else
                    {
                        httpUpload_->upload(slot.putUrl().toString(), file_);
                    }
                }
            }        
        });
    }
}

void HttpFileUploadManager::successReceived(QString )
{
    busy_ = false;

    emit fileUploadedForJidToUrl(jid_, getUrl_, fileType_);
}

void HttpFileUploadManager::errorReceived()
{
    if(file_->remove() == false)
    {
        qWarning() << "failed to remove file " << file_->fileName();
    }

    busy_ = false;

    emit fileUploadFailedForJidToUrl();
}

bool HttpFileUploadManager::createAttachmentPath()
{
    QString attachmentLocation = System::getAttachmentPath();
    QDir dir(attachmentLocation);

    qDebug() << "attachment location: " << attachmentLocation;

    if (!dir.exists()) {
        dir.mkpath(".");
    }

    return dir.exists();
}

// create a path and file name and change the suffix if provided
QString HttpFileUploadManager::createTargetFileName(QString source, QString suffix)
{
    QDateTime now(QDateTime::currentDateTimeUtc());
    uint unixTime = now.toTime_t();
    QFileInfo fileInfo(source);
    QString targetFileName;

    if (suffix.size() == 0)
         targetFileName = fileInfo.completeBaseName() + "." + fileInfo.suffix();
    else
         targetFileName = fileInfo.completeBaseName() + "." + suffix; 

    QString targetPath = System::getAttachmentPath() + QDir::separator() + QString::number(unixTime) + targetFileName;

    qDebug() << "target file: " << targetPath;

    return targetPath;
}

void HttpFileUploadManager::generateStatus(QString status)
{
    emit showStatus("File Upload", status);
}


void HttpFileUploadManager::setCompressImages(bool CompressImages)
{
   compressImages_ = CompressImages;
}

void HttpFileUploadManager::setLimitCompression(unsigned int LimitCompression)
{
   limitCompression_ = LimitCompression;
}