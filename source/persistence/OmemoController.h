#ifndef OMEMOCONTROLLER_H
#define OMEMOCONTROLLER_H

#include "QXmppOmemoStorage.h"
#include "QXmppTask.h"
#include "Database.h"

#include <QSqlTableModel>


class OmemoController : public QXmppOmemoStorage
{
public:
    OmemoController(Database *db, QObject *parent);
    ~OmemoController() override;

    void setup();

    QXmppTask<QXmppOmemoStorage::OmemoData> allData() override;

    QXmppTask<void> setOwnDevice(const std::optional<OwnDevice> &device) override;

    QXmppTask<void> addSignedPreKeyPair(uint32_t keyId, const SignedPreKeyPair &keyPair) override;
    QXmppTask<void> removeSignedPreKeyPair(uint32_t keyId) override;

    QXmppTask<void> addPreKeyPairs(const QHash<uint32_t, QByteArray> &keyPairs) override;
    QXmppTask<void> removePreKeyPair(uint32_t keyId) override;

    QXmppTask<void> addDevice(const QString &jid, uint32_t deviceId, const Device &device) override;
    QXmppTask<void> removeDevice(const QString &jid, uint32_t deviceId) override;
    QXmppTask<void> removeDevices(const QString &jid) override;

    QXmppTask<void> resetAll() override;

private:
    bool isDevicePresentInDatabase(const QString &jid, uint32_t deviceId);
    void printSqlError(QSqlTableModel *table);

    Database *database_;
    QSqlTableModel *identityTable_;
    QSqlTableModel *devicesTable_;
    QSqlTableModel *signedPreKeysTable_;
    QSqlTableModel *preKeysTable_;
};


#endif  // OMEMOCONTROLLER_H
