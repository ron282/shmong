
//#include "Persistence.h"
#include "XmppClient.h"
#include "Settings.h"
#include "QXmppTrustLevel.h"

#include <QObject>
#include <QStringList>
#include <QList>
#include <QFutureWatcher>


class RosterController;
class Persistence;
class ConnectionHandler;
class QXmppPubSubManager;
class QXmppTrustMemoryStorage;
class QXmppTrustManager;
class QXmppOmemoStorage;
class QXmppOmemoManager;
class QXmppCarbonManager;
class QXmppClient;
class QXmppAtmTrustMemoryStorage;
class QXmppAtmManager;

class HttpFileUploadManager;
#if 0
class MucManager;
#endif
class DiscoInfoHandler;
class MessageHandler;
class MamManager;

constexpr auto ANY_TRUST_LEVEL = QXmpp::TrustLevel::Undecided |
            QXmpp::TrustLevel::AutomaticallyDistrusted | 
            QXmpp::TrustLevel::ManuallyDistrusted |
            QXmpp::TrustLevel::AutomaticallyTrusted |
            QXmpp::TrustLevel::ManuallyTrusted |
            QXmpp::TrustLevel::Authenticated;

template<typename T, typename Handler>
void await(const QFuture<T> &future, QObject *context, Handler handler)
{
    auto *watcher = new QFutureWatcher<T>(context);
    QObject::connect(watcher, &QFutureWatcherBase::finished,
                     context, [watcher, handler = std::move(handler)]() mutable {
                     handler(watcher->result());
                         watcher->deleteLater();
                     });
    watcher->setFuture(future);
}

class Shmong : public QObject
{
    Q_OBJECT

    Q_PROPERTY(RosterController* rosterController READ getRosterController NOTIFY rosterControllerChanged)
    Q_PROPERTY(Persistence* persistence READ getPersistence NOTIFY persistenceChanged)
    Q_PROPERTY(bool connectionState READ connectionState NOTIFY connectionStateChanged)
    Q_PROPERTY(Settings* settings READ getSettings NOTIFY settingsChanged)

public:
    Shmong(QObject *parent = 0);
    ~Shmong();

    Q_INVOKABLE void mainDisconnect();
    Q_INVOKABLE void mainConnect(const QString &jid, const QString &pass);
    Q_INVOKABLE void reConnect();
    Q_INVOKABLE void setCurrentChatPartner(QString const &jid);
    Q_INVOKABLE QString getCurrentChatPartner();

    Q_INVOKABLE QString getAttachmentPath();
    Q_INVOKABLE void setHasInetConnection(bool connected_);
    Q_INVOKABLE void setAppIsActive(bool active);

    Q_INVOKABLE void joinRoom(QString const &roomJid, QString const &roomName);
    Q_INVOKABLE void removeRoom(QString const &roomJid);

    Q_INVOKABLE QString getLocalFileForUrl(const QString& str);
    Q_INVOKABLE void downloadFile(const QString& str, const QString& msgId);

    Q_INVOKABLE bool canSendFile();
    Q_INVOKABLE QString getVersion();

    Q_INVOKABLE bool isOmemoUser(const QString& jid);
    Q_INVOKABLE void omemoResetAll();

    Q_INVOKABLE void saveAttachment(const QString &msg);
    Q_INVOKABLE unsigned int getMaxUploadSize();

    bool connectionState() const;

public slots:
    void sendMessage(QString const &toJid, QString const &message, const QString &type);
    void sendMessage(QString const &message, const QString &type);
    void sendFile(QString const &toJid, QString const &file);
    void sendFile(QUrl const &file);
    void attachmentUploadFailed();
    void fileUploaded(QString const&toJid, QString const&message, const QString &type);

private slots:
    void sendReadNotification(bool active);
    void intialSetupOnFirstConnection();

    void slotAboutToQuit();

signals:
    void rosterControllerChanged();
    void persistenceChanged();
    void settingsChanged();

    void connectionStateChanged();

    void signalCanSendFile(bool);

    void signalShowMessage(QString headline, QString body);
    void signalShowStatus(QString headline, QString body);

    void signalHasInetConnection(bool connected);
    void signalAppGetsActive(bool active);

    void httpDownloadFinished(QString attachmentMsgId);

private:
#ifdef DBUS
public:
#endif

    void requestHttpUploadSlot();
    void handleHttpUploadResponse(const std::string response);

    RosterController* getRosterController();
    Persistence* getPersistence();
    Settings* getSettings();

    QXmppClient* client_;
    RosterController* rosterController_;
    Persistence* persistence_;
    Settings* settings_;
    ConnectionHandler* connectionHandler_;
    QXmppPubSubManager *pubsubManager_;
    QXmppTrustMemoryStorage *trustStorage_;
    QXmppTrustManager* trustManager_; 
    QXmppOmemoManager* omemoManager_;     
    QXmppCarbonManager *carbonManager_;

    HttpFileUploadManager* httpFileUploadManager_;
#if 0
    MucManager *mucManager_;
#endif
    DiscoInfoHandler* discoInfoHandler_;
    MessageHandler* messageHandler_;
    MamManager *mamManager_;

    QString jid_;
    QString password_;

    const QString version_;
    QString notSentMsgId_;

    bool omemoLoaded_;
};

