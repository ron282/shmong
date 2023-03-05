#include "OmemoController.h"

#include <QSqlRecord>
#include <QSqlError>
#include <QSqlQuery>
#include <QDateTime>

#include <QFutureWatcher>

#include <QDebug>

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
OmemoController::OmemoController(Database *db, QObject *parent) :
    database_(db)
{
    identityTable_ = new QSqlTableModel(parent, *(db->getPointer()));
    devicesTable_ = new QSqlTableModel(parent, *(db->getPointer()));
    signedPreKeysTable_ = new QSqlTableModel(parent, *(db->getPointer()));
    preKeysTable_ = new QSqlTableModel(parent, *(db->getPointer()));
}

void OmemoController::setup()
{
    bool success;

    qDebug() << "OmemoController::setup()" << endl;

    identityTable_->setTable(Database::sqlIdentityTable_);
    identityTable_->setEditStrategy(QSqlTableModel::OnManualSubmit);
    success = identityTable_->select();
    if(!success)
        qWarning() << "Error initializing " << Database::sqlIdentityTable_; 

    devicesTable_->setTable(Database::sqlDevicesTable_);
    devicesTable_->setEditStrategy(QSqlTableModel::OnManualSubmit);
    success = devicesTable_->select();
    if(!success)
        qWarning() << "Error initializing " << Database::sqlDevicesTable_; 

    signedPreKeysTable_->setTable(Database::sqlSignedPreKeyTable_);
    signedPreKeysTable_->setEditStrategy(QSqlTableModel::OnManualSubmit);
    success = signedPreKeysTable_->select();
    if(!success)
        qWarning() << "Error initializing " << Database::sqlSignedPreKeyTable_; 

    preKeysTable_->setTable(Database::sqlPreKeyTable_);
    preKeysTable_->setEditStrategy(QSqlTableModel::OnManualSubmit);
    preKeysTable_->select();
    if(!success)
        qWarning() << "Error initializing " << Database::sqlPreKeyTable_; 
}

OmemoController::~OmemoController() 
{
    delete identityTable_;
    delete devicesTable_;
    delete signedPreKeysTable_;
    delete preKeysTable_;
}

/// \cond
QFuture<QXmppOmemoStorage::OmemoData> OmemoController::allData()
{
    qDebug() << "OmemoController read allData" << endl;

    QSqlRecord r = identityTable_->record(0);
    QXmppOmemoStorage::OmemoData d;

    if(r.isNull(Database::sqlOwnDeviceId_) == false)
    {
        d.ownDevice.emplace();
        d.ownDevice->id = r.value(Database::sqlOwnDeviceId_).toUInt();
        d.ownDevice->label = r.value(Database::sqlOwnDeviceLabel_).toString();
        d.ownDevice->privateIdentityKey = QByteArray::fromHex(r.value(Database::sqlPrivateIdentityKey_).toString().toUtf8());
        d.ownDevice->publicIdentityKey = QByteArray::fromHex(r.value(Database::sqlPublicIdentityKey_).toString().toUtf8());
        d.ownDevice->latestSignedPreKeyId = r.value(Database::sqlLatestSignedPreKeyId_).toUInt();
        d.ownDevice->latestPreKeyId = r.value(Database::sqlLatestPreKeyId_).toUInt();        
    }
    else
    {
        qDebug() << "identityTable has no record" << endl;
    }

    r = signedPreKeysTable_->record(0);
    for(int i = 0; r.isNull(Database::sqlSignedPreKeyId_) == false; i++, r = signedPreKeysTable_->record(i))
    {
        SignedPreKeyPair spk;

        spk.creationDate = r.value(Database::sqlSignedPreKeyCreationDate_).toDateTime();
        spk.data = QByteArray::fromHex(r.value(Database::sqlSignedPreKeyData_).toString().toUtf8());

        d.signedPreKeyPairs[r.value(Database::sqlSignedPreKeyId_).toUInt()] = std::move(spk);       
    }

    r = preKeysTable_->record(0);
    for(int i = 0; r.isNull(Database::sqlPreKeyId_) == false; i++, r = preKeysTable_->record(i))
    {
        d.preKeyPairs[r.value(Database::sqlPreKeyId_).toUInt()] =  QByteArray::fromHex(r.value(Database::sqlPreKeyData_).toString().toUtf8());       
    }

    r = devicesTable_->record(0);
    for(int i = 0; r.isNull(Database::sqlJid_) == false; i++, r = devicesTable_->record(i))
    {
        Device dev;

        dev.label = r.value(Database::sqlDeviceLabel_).toString();
        dev.keyId = QByteArray::fromHex(r.value(Database::sqlDeviceKeyId_).toString().toUtf8());
        dev.session = QByteArray::fromHex(r.value(Database::sqlDeviceSession_).toString().toUtf8()); 
        dev.unrespondedSentStanzasCount = r.value(Database::sqlDeviceUnrespondedSentStanza_).toInt();
        dev.unrespondedReceivedStanzasCount = r.value(Database::sqlDeviceUnrespondedReceivedStanza_).toInt();
        dev.removalFromDeviceListDate = r.value(Database::sqlRemoveDate_).toDateTime(); 

        d.devices[r.value(Database::sqlJid_).toString()][r.value(Database::sqlDeviceId_).toUInt()] = std::move(dev);       
    }

    return makeReadyFuture(std::move(d));
}

QFuture<void> OmemoController::setOwnDevice(const std::optional<OwnDevice> &device)
{
    QSqlRecord r = identityTable_->record(0);
    bool hasNoRecord = r.isNull(Database::sqlOwnDeviceId_);
    bool success = true;

    if(device.has_value())
    {
        qDebug() << "OmemoController::setOwnDevice(id=" << device->id << ")" << endl;

        r.setValue(Database::sqlOwnDeviceId_, device->id);
        r.setValue(Database::sqlOwnDeviceLabel_, device->label);
        r.setValue(Database::sqlPrivateIdentityKey_, device->privateIdentityKey.toHex());
        r.setValue(Database::sqlPublicIdentityKey_, device->publicIdentityKey.toHex());
        r.setValue(Database::sqlLatestSignedPreKeyId_, device->latestSignedPreKeyId);
        r.setValue(Database::sqlLatestPreKeyId_, device->latestPreKeyId);

        if(hasNoRecord)
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
        qDebug() << "OmemoController::setOwnDevice has no value" << endl;

        identityTable_->clear();
    }        


    return makeReadyFuture();
}

QFuture<void> OmemoController::addSignedPreKeyPair(const uint32_t keyId, const SignedPreKeyPair &keyPair)
{
    qDebug() << "OmemoController::addSignedPreKeyPair(keyId=" << QString::number(keyId) << ")" << endl;

    QSqlQuery query(*(database_->getPointer()));
    
    query.prepare("INSERT OR REPLACE INTO "+ Database::sqlSignedPreKeyTable_ + " (" + 
        Database::sqlSignedPreKeyId_ + ", " + 
        Database::sqlSignedPreKeyCreationDate_ + ", " + 
        Database::sqlSignedPreKeyData_ + ") " +
        "VALUES (?, ?, ?)");

    query.addBindValue(keyId);
    query.addBindValue(keyPair.creationDate);
    query.addBindValue(keyPair.data.toHex());
 
    if(! query.exec())
    {
        qDebug() << "Error in OmemoController::addSignedPreKeyPair";
        qDebug() << query.lastError().databaseText();
        qDebug() << query.lastError().driverText();
        qDebug() << query.lastError().text();
    }
    else
    {
        // update the model with the changes of the database
        if (signedPreKeysTable_->select() != true)
        {
            qDebug() << "error on select in OmemoController::addSignedPreKeyPair";
        }
    }

    return makeReadyFuture();
}

QFuture<void> OmemoController::removeSignedPreKeyPair(const uint32_t keyId)
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
            qDebug() << "error on select in OmemoController::removeSignedPreKeyPair";
        }
    }

    return makeReadyFuture();
}

QFuture<void> OmemoController::addPreKeyPairs(const QHash<uint32_t, QByteArray> &keyPairs)
{
    qDebug() << "OmemoController::addPreKeyPairs()" << endl;

    QHash<uint32_t, QByteArray>::const_iterator it(keyPairs.constBegin());

    while(it != keyPairs.constEnd())
    {
        QSqlRecord record = preKeysTable_->record();

        record.setValue(Database::sqlPreKeyId_, it.key());
        record.setValue(Database::sqlPreKeyData_, it.value().toHex());

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

QFuture<void> OmemoController::removePreKeyPair(const uint32_t keyId)
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
            qDebug() << "error on select in OmemoController::removePreKeyPair";
        }
    }

    return makeReadyFuture();
}

QFuture<void> OmemoController::addDevice(const QString &jid, const uint32_t deviceId, const QXmppOmemoStorage::Device &device)
{
    QSqlQuery query(*(database_->getPointer()));
    
    query.prepare("INSERT OR REPLACE INTO "+ Database::sqlDevicesTable_ + " (" + 
        Database::sqlJid_ + ", " + 
        Database::sqlDeviceId_ + ", " + 
        Database::sqlDeviceLabel_ + ", " + 
        Database::sqlDeviceKeyId_ + ", " +
        Database::sqlDeviceSession_ + ", " +
        Database::sqlDeviceUnrespondedSentStanza_ + ", " +
        Database::sqlDeviceUnrespondedReceivedStanza_ + ", " +
        Database::sqlRemoveDate_ + ") " +
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?)");

    query.addBindValue(jid);
    query.addBindValue(deviceId);
    query.addBindValue(device.label);
    query.addBindValue(device.keyId.toHex());
    query.addBindValue(device.session.toHex());
    query.addBindValue(device.unrespondedSentStanzasCount);
    query.addBindValue(device.unrespondedReceivedStanzasCount);
    query.addBindValue(device.removalFromDeviceListDate);
 
    if(! query.exec())
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
            qDebug() << "error on select in OmemoController::addDevice";
        }
    }
    return makeReadyFuture();
}

QFuture<void> OmemoController::removeDevice(const QString &jid, const uint32_t deviceId)
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
            qDebug() << "error on select in OmemoController::removeDevice";
        }
    }

    return makeReadyFuture();
}

QFuture<void> OmemoController::removeDevices(const QString &jid)
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
            qDebug() << "error on select in OmemoController::removeDevices";
        }
    }

    return makeReadyFuture();
}

QFuture<void> OmemoController::resetAll()
{
    QSqlQuery query(*(database_->getPointer()));

    if (! query.exec("DELETE FROM " + Database::sqlIdentityTable_ ))
    {
        qDebug() << query.lastError().databaseText();
        qDebug() << query.lastError().driverText();
        qDebug() << query.lastError().text();
    }
    else
    {
        // update the model with the changes of the database
        if (identityTable_->select() != true)
        {
            qDebug() << "error on deleting all rows in " << Database::sqlIdentityTable_;
        }
    }

    if (! query.exec("DELETE FROM " + Database::sqlSignedPreKeyTable_ ))
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
            qDebug() << "error on deleting all rows in " << Database::sqlSignedPreKeyTable_;
        }
    }

    if (! query.exec("DELETE FROM " + Database::sqlPreKeyTable_ ))
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
            qDebug() << "error on deleting all rows in " << Database::sqlPreKeyTable_;
        }
    }

    if (! query.exec("DELETE FROM " + Database::sqlDevicesTable_ ))
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
            qDebug() << "error on deleting all rows in " << Database::sqlDevicesTable_;
        }
    }
 
    // NOt sure persistent data has to be cleared
    return makeReadyFuture();
}

void OmemoController::printSqlError(QSqlTableModel *table)
{
    qDebug() << table->lastError().databaseText();
    qDebug() << table->lastError().driverText();
    qDebug() << table->lastError().text();
}
