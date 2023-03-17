#ifndef HTTPFILEUPLOADMANAGER_H
#define HTTPFILEUPLOADMANAGER_H

#include <QObject>

class QFile;
class FileWithCypher;
class HttpFileUploader;
class QXmppClient;
class QXmppUploadRequestManager;

class HttpFileUploadManager : public QObject
{
    Q_OBJECT
public:
    explicit HttpFileUploadManager(QObject *parent = 0);

    bool requestToUploadFileForJid(QString const &file, const QString &jid, bool encryptFile);
    QString getStatus();

    void setupWithClient(QXmppClient* client);

    void setServerHasFeatureHttpUpload(bool hasFeature);
    bool getServerHasFeatureHttpUpload();

    void setUploadServerJid(QString const & uploadServerJid);
    QString getUploadServerJid();

    void setMaxFileSize(unsigned int maxFileSize);
    unsigned int getMaxFileSize();

signals:
    void fileUploadedForJidToUrl(QString, QString, QString);
    void fileUploadFailedForJidToUrl();
    void showStatus(QString, QString);
    void serverHasHttpUpload_(bool);

public slots:
    void updateStatusString(QString string);
    void successReceived(QString string);
    void errorReceived();
    void setCompressImages(bool CompressImages);
    void setLimitCompression(unsigned int limitCompression);
    void handleServiceFoundChanged();

private slots:
    void generateStatus(QString status);

private:
    HttpFileUploader* httpUpload_;
    void requestHttpUploadSlot();

    bool createAttachmentPath();
    QString createTargetFileName(QString source, QString suffix="");

    static bool encryptFile(QFile &file, QByteArray &ivAndKey);

    bool serverHasFeatureHttpUpload_;
    unsigned int maxFileSize_;

    FileWithCypher* file_;
    QString jid_;

    QXmppClient* client_;
    QXmppUploadRequestManager* uploadRequestManager_;
    QString uploadServerJid_;

    QString statusString_;
    QString getUrl_;

    bool busy_;
    bool encryptFile_;

    QString fileType_;

    bool compressImages_;
    unsigned int limitCompression_;
};

#endif // HTTPFILEUPLOADMANAGER_H
