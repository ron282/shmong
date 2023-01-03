#include "OmemoDbStorage.h"

#include <QSqlRecord>
#include <QSqlError>
#include <QSqlQuery>
#include <QDateTime>

#include <QFutureWatcher>

template<typename T>
QFuture<T> makeReadyFuture(T &&value)
{
    QFutureInterface<T> interface(QFutureInterfaceBase::Started);
    interface.reportResult(std::move(value));
    interface.reportFinished();
    return interface.future();
}

inline QFuture<void> makeReadyFuture()
{
    using State = QFutureInterfaceBase::State;
    return QFutureInterface<void>(State(State::Started | State::Finished)).future();
}

///
/// Constructs an OMEMO memory storage.
///
QXmppOmemoDbStorage::QXmppOmemoDbStorage(Database *db) :
    database_(db)
{
    identityTable_ = new QSqlTableModel(db->parent(), *(db->getPointer()));
    devicesTable_ = new QSqlTableModel(db->parent(), *(db->getPointer()));
    signedPreKeysTable_ = new QSqlTableModel(db->parent(), *(db->getPointer()));
    preKeysTable_ = new QSqlTableModel(db->parent(), *(db->getPointer()));
}

void QXmppOmemoDbStorage::setup()
{
    identityTable_->setEditStrategy(QSqlTableModel::OnManualSubmit);
    devicesTable_->setEditStrategy(QSqlTableModel::OnManualSubmit);
    signedPreKeysTable_->setEditStrategy(QSqlTableModel::OnManualSubmit);
    preKeysTable_->setEditStrategy(QSqlTableModel::OnManualSubmit);

    identityTable_->setTable(Database::sqlIdentityTable_);
    devicesTable_->setTable(Database::sqlDevicesTable_);
    signedPreKeysTable_->setTable(Database::sqlSignedPreKeyTable_);
    preKeysTable_->setTable(Database::sqlPreKeyTable_);
}

QXmppOmemoDbStorage::~QXmppOmemoDbStorage() 
{
    delete identityTable_;
    delete devicesTable_;
    delete signedPreKeysTable_;
    delete preKeysTable_;
}

/// \cond
QFuture<QXmppOmemoStorage::OmemoData> QXmppOmemoDbStorage::allData()
{
    QSqlRecord r = identityTable_->record(0);
    QXmppOmemoStorage::OmemoData d;

    if(r.isEmpty() == false)
    {
        d.ownDevice.emplace();
        d.ownDevice->id = r.value(Database::sqlOwnDeviceId_).toUInt();
        d.ownDevice->label = r.value(Database::sqlOwnDeviceLabel_).toString();
        d.ownDevice->privateIdentityKey = r.value(Database::sqlPrivateIdentityKey_).toByteArray();
        d.ownDevice->publicIdentityKey = r.value(Database::sqlPublicIdentityKey_).toByteArray();
        d.ownDevice->latestSignedPreKeyId = r.value(Database::sqlLatestSignedPreKeyId_).toUInt();
        d.ownDevice->latestPreKeyId = r.value(Database::sqlLatestPreKeyId_).toUInt();        
    }

    r = signedPreKeysTable_->record(0);
    for(int i = 0; r.isEmpty() == false; i++, r = signedPreKeysTable_->record(i))
    {
        SignedPreKeyPair spk;

        spk.creationDate = r.value(Database::sqlSignedPreKeyCreationDate_).toDateTime();
        spk.data = r.value(Database::sqlSignedPreKeyData_).toByteArray();

        d.signedPreKeyPairs[r.value(Database::sqlSignedPreKeyId_).toUInt()] = std::move(spk);       
    }

    r = preKeysTable_->record(0);
    for(int i = 0; r.isEmpty() == false; i++, r = preKeysTable_->record(i))
    {
        d.preKeyPairs[r.value(Database::sqlPreKeyId_).toUInt()] =  r.value(Database::sqlPreKeyData_).toByteArray();       
    }

    r = preKeysTable_->record(0);
    for(int i = 0; r.isEmpty() == false; i++, r = preKeysTable_->record(i))
    {
        Device dev;

        dev.label = r.value(Database::sqlDeviceLabel_).toString();
        dev.keyId = r.value(Database::sqlDeviceKeyId_).toByteArray();
        dev.session = r.value(Database::sqlDeviceSession_).toByteArray(); 
        dev.unrespondedSentStanzasCount = r.value(Database::sqlDeviceUnrespondedSentStanza_).toInt();
        dev.unrespondedReceivedStanzasCount = r.value(Database::sqlDeviceUnrespondedReceivedStanza_).toInt();
        dev.removalFromDeviceListDate = r.value(Database::sqlRemoveDate_).toDateTime(); 

        d.devices[r.value(Database::sqlJid_).toString()][r.value(Database::sqlDeviceId_).toUInt()] = std::move(dev);       
    }

    return makeReadyFuture(std::move(d));
}

QFuture<void> QXmppOmemoDbStorage::setOwnDevice(const std::optional<OwnDevice> &device)
{
    QSqlRecord r = identityTable_->record(0);
    bool success = true;

    if(device.has_value())
    {
        r.setValue(Database::sqlOwnDeviceId_, device->id);
        r.setValue(Database::sqlOwnDeviceLabel_, device->label);
        r.setValue(Database::sqlPrivateIdentityKey_, device->privateIdentityKey);
        r.setValue(Database::sqlPublicIdentityKey_, device->publicIdentityKey);
        r.setValue(Database::sqlLatestSignedPreKeyId_, device->latestSignedPreKeyId);
        r.setValue(Database::sqlLatestPreKeyId_, device->latestPreKeyId);

        if(r.isEmpty())
        {
            success = identityTable_->insertRecord(-1, r);
        }
        else
        {
            success = identityTable_->setRecord(0, r);
        }    

        if(! success)
        {
            printSqlError(identityTable_);        
        }
        else
        {
            if (identityTable_->submitAll())
            {
                identityTable_->database().commit();
            }
            else
            {
                identityTable_->database().rollback();
                printSqlError(identityTable_);
            }
        }
    }
    else
    {
        identityTable_->clear();
    }        

    return makeReadyFuture();
}

QFuture<void> QXmppOmemoDbStorage::addSignedPreKeyPair(const uint32_t keyId, const SignedPreKeyPair &keyPair)
{
    QSqlRecord record = signedPreKeysTable_->record();

    record.setValue(Database::sqlSignedPreKeyId_, keyId);
    record.setValue(Database::sqlSignedPreKeyCreationDate_, keyPair.creationDate);
    record.setValue(Database::sqlSignedPreKeyData_, keyPair.data);
 
    if (! signedPreKeysTable_->insertRecord(-1, record))
    {
        printSqlError(signedPreKeysTable_);
    }
    else
    {
        if (signedPreKeysTable_->submitAll())
        {
            signedPreKeysTable_->database().commit();
        }
        else
        {
            signedPreKeysTable_->database().rollback();
            printSqlError(signedPreKeysTable_);
        }
    }

    return makeReadyFuture();
}

QFuture<void> QXmppOmemoDbStorage::removeSignedPreKeyPair(const uint32_t keyId)
{
    // First parse the list of messages with attachments

    QSqlQuery query(*(database_->getPointer()));

    // Then remove messages

    if (! query.exec("DELETE FROM " + Database::sqlSignedPreKeyTable_
                     + " WHERE " + Database::sqlSignedPreKeyId_ + " = \"" + keyId + "\"" ))
    {
        qDebug() << query.lastError().databaseText();
        qDebug() << query.lastError().driverText();
        qDebug() << query.lastError().text();
    }
    else
    {
        // update the model with the changes of the database
        if (signedPreKeysTable_->select() != true)
        {
            qDebug() << "error on select in QXmppOmemoDbStorage::removeSignedPreKeyPair";
        }
    }

    return makeReadyFuture();
}

QFuture<void> QXmppOmemoDbStorage::addPreKeyPairs(const QHash<uint32_t, QByteArray> &keyPairs)
{
    QHash<uint32_t, QByteArray>::const_iterator it(keyPairs.constBegin());

    while(it != keyPairs.constEnd())
    {
        QSqlRecord record = preKeysTable_->record();

        record.setValue(Database::sqlPreKeyId_, it.key());
        record.setValue(Database::sqlPreKeyData_, it.value());

        if (! preKeysTable_->insertRecord(-1, record))
        {
            printSqlError(preKeysTable_);
        }
        else
        {
            if (preKeysTable_->submitAll())
            {
                preKeysTable_->database().commit();
            }
            else
            {
                preKeysTable_->database().rollback();
                printSqlError(preKeysTable_);
            }
        }

        it++;        
    }

    return makeReadyFuture();
}

QFuture<void> QXmppOmemoDbStorage::removePreKeyPair(const uint32_t keyId)
{
    // First parse the list of messages with attachments

    QSqlQuery query(*(database_->getPointer()));

    // Then remove messages

    if (! query.exec("DELETE FROM " + Database::sqlPreKeyTable_
                     + " WHERE " + Database::sqlPreKeyId_ + " = \"" + keyId + "\"" ))
    {
        qDebug() << query.lastError().databaseText();
        qDebug() << query.lastError().driverText();
        qDebug() << query.lastError().text();
    }
    else
    {
        // update the model with the changes of the database
        if (preKeysTable_->select() != true)
        {
            qDebug() << "error on select in QXmppOmemoDbStorage::removePreKeyPair";
        }
    }

    return makeReadyFuture();
}

QFuture<void> QXmppOmemoDbStorage::addDevice(const QString &jid, const uint32_t deviceId, const QXmppOmemoStorage::Device &device)
{
    QSqlRecord record = devicesTable_->record();

    record.setValue(Database::sqlJid_, jid);
    record.setValue(Database::sqlDeviceId_, deviceId);
    record.setValue(Database::sqlDeviceLabel_, device.label);
    record.setValue(Database::sqlDeviceKeyId_, device.keyId);
    record.setValue(Database::sqlDeviceSession_, device.session);
    record.setValue(Database::sqlDeviceUnrespondedSentStanza_, device.unrespondedSentStanzasCount);
    record.setValue(Database::sqlDeviceUnrespondedReceivedStanza_, device.unrespondedReceivedStanzasCount);
    record.setValue(Database::sqlRemoveDate_, device.removalFromDeviceListDate);
 
    if (! devicesTable_->insertRecord(-1, record))
    {
        printSqlError(devicesTable_);
    }
    else
    {
        if (devicesTable_->submitAll())
        {
            devicesTable_->database().commit();
        }
        else
        {
            devicesTable_->database().rollback();
            printSqlError(devicesTable_);
        }
    }
 
    return makeReadyFuture();
}

QFuture<void> QXmppOmemoDbStorage::removeDevice(const QString &jid, const uint32_t deviceId)
{
    // First parse the list of messages with attachments

    QSqlQuery query(*(database_->getPointer()));

    // Then remove messages

    if (! query.exec("DELETE FROM " + Database::sqlDevicesTable_
                     + " WHERE " + Database::sqlJid_ + " = \"" + jid + "\"" 
                     + " AND " + Database::sqlDeviceId_ + " = " + deviceId))
    {
        qDebug() << query.lastError().databaseText();
        qDebug() << query.lastError().driverText();
        qDebug() << query.lastError().text();
    }
    else
    {
        // update the model with the changes of the database
        if (devicesTable_->select() != true)
        {
            qDebug() << "error on select in QXmppOmemoDbStorage::removeDevice";
        }
    }

    return makeReadyFuture();
}

QFuture<void> QXmppOmemoDbStorage::removeDevices(const QString &jid)
{
    // First parse the list of messages with attachments

    QSqlQuery query(*(database_->getPointer()));

    // Then remove messages

    if (! query.exec("DELETE FROM " + Database::sqlDevicesTable_
                     + " WHERE " + Database::sqlJid_ + " = \"" + jid + "\"" ))
    {
        qDebug() << query.lastError().databaseText();
        qDebug() << query.lastError().driverText();
        qDebug() << query.lastError().text();
    }
    else
    {
        // update the model with the changes of the database
        if (devicesTable_->select() != true)
        {
            qDebug() << "error on select in QXmppOmemoDbStorage::removeDevices";
        }
    }

    return makeReadyFuture();
}

QFuture<void> QXmppOmemoDbStorage::resetAll()
{
    // NOt sure persistent data has to be cleared
    return makeReadyFuture();
}

void QXmppOmemoDbStorage::printSqlError(QSqlTableModel *table)
{
    qDebug() << table->lastError().databaseText();
    qDebug() << table->lastError().driverText();
    qDebug() << table->lastError().text();
}
