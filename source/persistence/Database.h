#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QSqlDatabase>

class Database : public QObject
{
    Q_OBJECT

public:
    explicit Database(QObject *parent = nullptr);
    bool open(QString const &jid);
    bool isValid();
    QSqlDatabase* getPointer();

    void dumpDataToStdOut() const;

    static const QString sqlMsgName_;
    static const QString sqlMsgMessage_;
    static const QString sqlMsgDirection_;
    static const QString sqlMsgType_;
    static const QString sqlMsgState_;
    static const QString security_;

    static const QString sqlSessionName_;
    static const QString sqlSessionLastMsg_;
    static const QString sqlSessionUnreadMsg_;

    static const QString sqlGcmName_;
    static const QString sqlGcmChatMemberName_;
    static const QString sqlGcmState_;

    static const QString sqlId_;
    static const QString sqlJid_;
    static const QString sqlResource_;
    static const QString sqlTimestamp_;

    static const QString sqlIdentityTable_;
    static const QString sqlOwnDeviceId_;
    static const QString sqlOwnDeviceLabel_;
    static const QString sqlPrivateIdentityKey_;
    static const QString sqlPublicIdentityKey_;
    static const QString sqlLatestSignedPreKeyId_;
    static const QString sqlLatestPreKeyId_;

    static const QString sqlSignedPreKeyTable_;
    static const QString sqlSignedPreKeyId_;
    static const QString sqlSignedPreKeyCreationDate_;
    static const QString sqlSignedPreKeyData_;
    static const QString sqlSignedPreKeyDataLen_;

    static const QString sqlPreKeyTable_;
    static const QString sqlPreKeyId_;
    static const QString sqlPreKeyData_;

    static const QString sqlDevicesTable_;
    static const QString sqlDeviceId_;
    static const QString sqlDeviceLabel_;
    static const QString sqlDeviceKeyId_;
    static const QString sqlDeviceSession_;
    static const QString sqlDeviceUnrespondedSentStanza_;
    static const QString sqlDeviceUnrespondedReceivedStanza_;
    static const QString sqlRemoveDate_;

signals:

public slots:

private:
    bool databaseValid_;
    QSqlDatabase database_;
};

#endif // DATABASE_H
