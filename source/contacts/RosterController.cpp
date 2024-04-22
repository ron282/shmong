#include "RosterController.h"
#include "System.h"

#include "QXmppPresence.h"
#include "QXmppTask.h"

#include <QQmlContext>
#include <QImage>
#include <QStandardPaths>
#include <QDir>
#include <QFuture>
#include <QCryptographicHash>

#include <QDebug>

RosterController::RosterController(QObject *parent) : QObject(parent),
    qXmppClient_(nullptr), rosterManager_(nullptr), vCardManager_(nullptr), rosterList_()
{
    QString avatarLocation = System::getAvatarPath();
    QDir dir(avatarLocation);

    if (!dir.exists())
    {
        dir.mkpath(".");
    }
}

void RosterController::setupWithClient(QXmppClient *qXmppClient)
{
    if (qXmppClient != nullptr)
    {
        qXmppClient_ = qXmppClient;
        rosterManager_ = qXmppClient_->findExtension<QXmppRosterManager>();
        vCardManager_ = qXmppClient_->findExtension<QXmppVCardManager>();

        connect(qXmppClient_, &QXmppClient::presenceReceived, this, &RosterController::handlePresenceReceived);

        if(rosterManager_ != nullptr)
        {
            connect(rosterManager_, &QXmppRosterManager::rosterReceived, this, &RosterController::handleRosterReceived);
            connect(rosterManager_, &QXmppRosterManager::presenceChanged, this, &RosterController::handlePresenceChanged);
        }

        if(vCardManager_ != nullptr)
        {
            connect(vCardManager_, &QXmppVCardManager::vCardReceived, this, &RosterController::handleVCardChanged);
        }

        bindJidUpdateMethodes();
    }
}

/*
 * called after an contactAdd
 * iq get/set rosterItem
 */
void RosterController::handleJidAdded(const QString &bareJid)
{
    //qDebug() << "################# RosterController::handleJidAdded: " << bareJid;
    //dumpRosterList();

    if (rosterManager_ != nullptr)
    {
        appendToRosterIfNotAlreadyIn(bareJid);

        sortRosterList();

        // request subscription
        rosterManager_->subscribeTo(bareJid);

        emit rosterListChanged();
    }

    //qDebug() << "##################### handleJidAdded: rL_.size: " << rosterList_.size();
}

void RosterController::handleJidUpdated(const QString &bareJid)
{
    //qDebug() << "############# RosterController::handleJidUpdated " << bareJid;
    dumpRosterList();

    if (isJidInRoster(bareJid) == false && rosterManager_ != nullptr)
    {
        QXmppRosterIq::Item entry = rosterManager_->getRosterEntry(bareJid);

        bool changed1 = updateNameForJid(bareJid, entry.name());
        bool changed2 = updateSubscriptionForJid(bareJid, entry.subscriptionType());


        if (changed1 || changed2)
        {
            emit rosterListChanged();
        }
    }

    //qDebug() << "#####################handleJidUpdated: rL_.size: " << rosterList_.size();
}

bool RosterController::updateNameForJid(const QString &bareJid, const QString &name)
{
    //qDebug()  << "-- updateNameForJid: " << bareJid << ", name: " << name;
    bool somethingChanged = false;

    appendToRosterIfNotAlreadyIn(bareJid);

    for (auto item: rosterList_)
    {
        if (item->getJid().compare(bareJid, Qt::CaseInsensitive) == 0)
        {
            item->setName(name);
            somethingChanged = true;
            break;
        }
    }

    return somethingChanged;
}

bool RosterController::updateSubscriptionForJid(const QString &bareJid, QXmppRosterIq::Item::SubscriptionType subscription)
{
    //qDebug()  << "-- updateSubscriptionForJid: " << bareJid << ", subs: " << subscription;

    bool somethingChanged = false;

    if(subscription != QXmppRosterIq::Item::Remove)
        appendToRosterIfNotAlreadyIn(bareJid);

    for (auto item: rosterList_)
    {
        if (item->getJid().compare(bareJid, Qt::CaseInsensitive) == 0)
        {
            RosterItem::Subscription sub = item->getSubscription();

            if(subscription == QXmppRosterIq::Item::None)
                sub = RosterItem::SUBSCRIPTION_NONE;
            else if(subscription == QXmppRosterIq::Item::From)
                sub = RosterItem::SUBSCRIPTION_FROM;
            else if(subscription == QXmppRosterIq::Item::To)
                sub = RosterItem::SUBSCRIPTION_TO;
            else if(subscription == QXmppRosterIq::Item::Both)
                sub = RosterItem::SUBSCRIPTION_BOTH;

            if(sub != item->getSubscription())
            {
                // FIXME need two signal here for testing?
                item->setSubscription(sub);
                emit rosterListChanged();
                emit subscriptionUpdated(sub);
                somethingChanged = true;
            }

            break;
        }
    }

    return somethingChanged;
}

bool RosterController::updateStatusForJid(const QString& jid, const QString& status)
{
    bool somethingChanged = false;

    appendToRosterIfNotAlreadyIn(jid);

    if (! status.isEmpty())
    {
        for (auto item: rosterList_)
        {
            if (item->getJid().compare(jid, Qt::CaseInsensitive) == 0)
            {
                item->setStatus(status);
                emit rosterListChanged();

                somethingChanged = true;
                break;
            }
        }
    }

    return somethingChanged;
}

bool RosterController::updateAvailabilityForJid(const QString &jid, RosterItem::Availability availability)
{
    bool somethingChanged = false;

    appendToRosterIfNotAlreadyIn(jid);

    for (auto item: rosterList_)
    {
        if (item->getJid().compare(jid, Qt::CaseInsensitive) == 0)
        {
            if(availability != item->getAvailability())
            {
                item->setAvailability(availability);
                emit rosterListChanged();
                somethingChanged = true;
            }
            break;
        }
    }

    return somethingChanged;
}

int RosterController::getAvailability(const QString& jid)
{
    for (auto item: rosterList_)
    {
        if (item->getJid().compare(jid, Qt::CaseInsensitive) == 0)
        {
            return item->getAvailability();
        }
    }

    return RosterItem::AVAILABILITY_UNKNOWN;
}

void RosterController::handlePresenceReceived(const QXmppPresence &presence)
{
    // Automatically approve subscription requests
    // FIXME show to user and let user decide
    if(rosterManager_ != nullptr)
    {
        if (presence.type() == QXmppPresence::Subscribe)
        {
            rosterManager_->acceptSubscription(presence.from());

            // request subscription
            rosterManager_->subscribeTo(presence.from());
        }
    }
}

void RosterController::handlePresenceChanged(const QString &bareJid, const QString& resource)
{
    qDebug() << "RosterController::handlePresenceChanged";

    if(rosterManager_ != nullptr)
    {
        QXmppPresence presence = rosterManager_->getPresence(bareJid, resource);

        bool changed1 = false;
        bool changed2 = false;

        if (bareJid.compare(qXmppClient_->configuration().jidBare(), Qt::CaseInsensitive) != 0 ) // only interested in updates of other clients. not our self sent presence msgs
        {
            QString status = "";

            if (presence.type() == QXmppPresence::Available)
            {
                changed1 = updateStatusForJid(bareJid, presence.statusText());
            }

            RosterItem::Availability availability = RosterItem::AVAILABILITY_ONLINE;

            if (presence.type() == QXmppPresence::Unavailable
                    || presence.type() == QXmppPresence::Error
                    || presence.type() == QXmppPresence::Probe
                    || presence.type() == QXmppPresence::Unsubscribe
                    || presence.type() == QXmppPresence::Unsubscribed
                    )
            {
                availability = RosterItem::AVAILABILITY_OFFLINE;
            }
            changed2 = updateAvailabilityForJid(bareJid, availability);
        }

        if (changed1 || changed2)
        {
            emit rosterListChanged();
        }
    }
}

void RosterController::handleJidRemoved(const QString &bareJid)
{
    //qDebug() << "############ RosterController::handleJidRemoved: " << bareJid;
    dumpRosterList();

    bool somethingChanged = false;

    QList<RosterItem*>::iterator it = rosterList_.begin();
    while (it != rosterList_.end())
    {
        if ((*it)->getJid().compare(bareJid, Qt::CaseInsensitive) == 0)
        {
            it = rosterList_.erase(it);
            // TODO what about deleting the RosterItem object?!

            somethingChanged = true;
            break;
        }
        else
        {
            ++it;
        }
    }

    if (somethingChanged)
    {
        //qDebug() << "handleJidRemoved: emit rosterListChanged";

        sortRosterList();
        emit rosterListChanged();
    }

    //qDebug() << "#####################handleJidRemoved: rL_.size: " << rosterList_.size();
}

/*
 * called after the requested roster list is received.
 * Attention. In real world updated from presence will arrive before the roster is received.
 */
void RosterController::handleRosterReceived()
{
    bool changed = false;

    qDebug() << "RosterController::handleRosterReceived";

    const QStringList jids = rosterManager_->getRosterBareJids();
    for (const QString &bareJid : jids) {

        if(appendToRosterIfNotAlreadyIn(bareJid))
        {
            // request subscription
            rosterManager_->subscribeTo(bareJid);

            // request vCard
            if(vCardManager_)
                vCardManager_->requestVCard(bareJid);

            changed = true;
        }
    }


    if(changed)
    {
        sortRosterList();
        emit rosterListChanged();
    }
}

void RosterController::handleVCardChanged(const QXmppVCardIq &vCard)
{
    qDebug() << "vCard received";

    const QString bareJid = vCard.from();
    const QString newHash = QCryptographicHash::hash(vCard.photo(), QCryptographicHash::Md5);

    if (checkHashDiffers(bareJid, newHash) == true)
    {
        QByteArray byteArray = vCard.photo();

        // try to extract photo type for processing to QImage
        QString photoType = vCard.photoType();
        QString imageType = photoType;
        if (imageType.contains("/"))
        {
            QStringList splitedImageType = imageType.split("/");
            if (splitedImageType.size() == 2)
            {
                imageType = splitedImageType.at(1);
            }
        }

        // create a QImage out of the vCard photo data
        QImage image = QImage::fromData(byteArray, imageType.toStdString().c_str());

        if (image.isNull() == false)
        {
            // write image file to disk, try to delete old one
            QString imageName = bareJid;
            imageName.replace("@", "-at-");
            QString imageBasePath = System::getAvatarPath() + QDir::separator() + imageName;
            QString imagePath = imageBasePath + ".png";

            QFile oldImageFile(imagePath);
            oldImageFile.remove();

            if (! image.save(imagePath, "PNG"))
            {
                qDebug() << "cant save image to: " << imagePath;
            }

            // write hash file to disk, try to delete old one
            QString imageHash = imageBasePath + ".hash";
            QFile hashFile(imageHash);
            hashFile.remove();
            if (hashFile.open(QIODevice::WriteOnly | QIODevice::Text))
            {
                QTextStream out(&hashFile);
                out << newHash;
                hashFile.close();
            }
            else
            {
                qDebug() << "cant save hash to: " << imageHash;
            }

            // signal new avatar to the rosterItem
            foreach(RosterItem *item, rosterList_)
            {
                if (item->getJid().compare(bareJid, Qt::CaseInsensitive) == 0)
                {
                    item->triggerNewImage();
                }
            }
        }
    }
}

bool RosterController::checkHashDiffers(QString const &jid, QString const &newHash)
{
    bool returnValue = false;

    // read existing hash from file
    QString jidPart = jid;
    jidPart.replace("@", "-at-");

    QString imageHashPath = System::getAvatarPath() + QDir::separator() + jidPart + ".hash";
    QFileInfo info(imageHashPath);

    if (info.exists() && info.isFile())
    {
        QFile file(imageHashPath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QString line = file.readLine();
            if (line.compare(newHash) == 0)
            {
                // same hash
                returnValue = false;
            }
            else
            {
                // hash differs
                returnValue = true;
            }

            //qDebug() << "old hash: " << line << ". new: " << newHash << "bool: " << returnValue;

            file.close();
        }
        else
        {
            qDebug() << "cant save hash to: " << imageHashPath;
        }

    }
    else
    {
        // no hash file. hash differs!
        returnValue = true;
    }

    return returnValue;
}

void RosterController::bindJidUpdateMethodes()
{
    if(rosterManager_)
    {
        connect(rosterManager_, &QXmppRosterManager::itemAdded, this, &RosterController::handleJidAdded);
        connect(rosterManager_, &QXmppRosterManager::itemRemoved, this, &RosterController::handleJidRemoved);
        connect(rosterManager_, &QXmppRosterManager::itemChanged, this, &RosterController::handleJidUpdated);
    }
}

void RosterController::addContact(const QString& jid, const QString& name)
{
    //qDebug() << "addContact: " << jid << ", name: " << name;

    //TODO Check jid is valid

    if (isJidInRoster(jid) == false && rosterManager_ != nullptr)
    {
        rosterManager_->addRosterItem(jid, name);
        qDebug() << "  stanza sent";
    }
    else
    {
        emit signalShowMessage("Add Contact", "JID not valid or already in Roster!");
        qDebug() << "  already in roster";
    }
}

void RosterController::removeContact(const QString& jid)
{
    if(isGroup(jid))
    {
        //Manage internal errors
        qWarning() << "group considered as individual contact";
        removeGroupFromContacts(jid);
    }
    else
    {
        sendUnavailableAndUnsubscribeToJid(jid);

        if(rosterManager_ != nullptr)
            rosterManager_->removeRosterItem(jid);
    }
}

void RosterController::sendUnavailableAndUnsubscribeToJid(const QString& jid)
{
    //TODO Check Jid is valid

    if (rosterManager_ != nullptr)
    {
        rosterManager_->unsubscribeFrom(jid);
    }
}

QQmlListProperty<RosterItem> RosterController::getRosterList()
{
    return QQmlListProperty<RosterItem>(this, rosterList_);
}

bool RosterController::isJidInRoster(const QString& bareJid)
{
    bool returnValue = false;

    for(auto item: rosterList_)
    {
        if (item->getJid().compare(bareJid, Qt::CaseInsensitive) == 0)
        {
            returnValue = true;
            break;
        }
    }

    return returnValue;
}

void RosterController::addGroupAsContact(QString groupJid, QString groupName)
{
    //qDebug() << "addGroupAsContact";
    dumpRosterList();

    if (isJidInRoster(groupJid) == false)
    {
        rosterList_.append(new RosterItem(groupJid, groupName, RosterItem::SUBSCRIPTION_NONE, true, this));

        sortRosterList();
        emit rosterListChanged();
    }
    else
    {
        //qDebug() << "############ group already in roster gui!" << groupJid;
    }

    //qDebug() << "#####################addGroupAsContact: rL_.size: " << rosterList_.size();
}

void RosterController::removeGroupFromContacts(QString groupJid)
{
    bool somethingChanged = false;
    QList<RosterItem*>::iterator it = rosterList_.begin();

    for (; it != rosterList_.end(); ++it)
    {
        if (groupJid.compare( (*it)->getJid(), Qt::CaseInsensitive ) == 0)
        {
            qDebug() << "remove group:" << groupJid;

            it = rosterList_.erase(it);
            // TODO what about free the RosterItem?!

            somethingChanged = true;
            break;
        }
    }

    if (somethingChanged)
    {
        sortRosterList();
        emit rosterListChanged();
    }
}

bool RosterController::isGroup(QString const &jid)
{
    bool returnValue = true;

    QList<RosterItem*>::iterator it = rosterList_.begin();
    for (; it != rosterList_.end(); ++it)
    {
        if (jid.compare((*it)->getJid(), Qt::CaseInsensitive) == 0)
        {
            returnValue = (*it)->isGroup();
            break;
        }
    }

    return returnValue;
}

QString RosterController::getAvatarImagePathForJid(QString const &jid)
{
    return getTypeForJid(attributePicturePath, jid);
}

QString RosterController::getNameForJid(QString const &jid)
{
    return getTypeForJid(attributeName, jid);
}



QString RosterController::getTypeForJid(itemAttribute const &attribute, QString const &jid)
{
    QString returnValue = "";

    QList<RosterItem*>::iterator it = rosterList_.begin();
    for (; it != rosterList_.end(); ++it)
    {
        if (jid.compare((*it)->getJid()) == 0)
        {
            if (attribute == attributePicturePath)
            {
                returnValue = (*it)->getImagePath();
            }
            else if(attribute == attributeName)
            {
                returnValue = (*it)->getName();
            }
            break;
        }
    }

    return returnValue;
}

bool RosterController::appendToRosterIfNotAlreadyIn(const QString& jid)
{
    //qDebug() << "----------------- appendToRosterIfNotAlreadyIn ----------  " << jid;
    if (isJidInRoster(jid) == false // not already in
            &&
            (! jid.compare( qXmppClient_->configuration().jidBare(), Qt::CaseInsensitive) == 0)) // not the user of this client instance
    {
        QXmppRosterIq::Item item = rosterManager_->getRosterEntry(jid);

        rosterList_.append(new RosterItem(jid, item.name(), RosterItem::SUBSCRIPTION_NONE, false, this));
        return true; // entry added
    }
    else
        return false; // already present or current
}

void RosterController::sortRosterList()
{
    struct
    {
        bool operator()(RosterItem* a, RosterItem* b) const
        {
            return a->getName().compare(b->getName(), Qt::CaseInsensitive) < 0;
        }
    } customSort;

    std::sort(rosterList_.begin(), rosterList_.end(), customSort);
}

#ifdef DBUS
QList<RosterItem*> RosterController::fetchRosterList()
{
    return rosterList_;
}
#endif

void RosterController::dumpRosterList()
{
    for (auto item: rosterList_)
    {
        qDebug() << "rl: " << item->getJid() << ", name: " << item->getName() << ", isGroup? " << item->isGroup();
    }
}
