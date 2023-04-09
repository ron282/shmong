#ifndef ROSTERCONTROLLER_H
#define ROSTERCONTROLLER_H

#include "RosterItem.h"

#include <QObject>
#include <QQmlListProperty>
#include "QXmppClient.h"
#include "QXmppVCardIq.h"
#include "QXmppRosterManager.h"
#include "QXmppVCardManager.h"
#include "QXmppRosterIq.h"

class RosterController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<RosterItem> rosterList READ getRosterList NOTIFY rosterListChanged)

    enum itemAttribute
    {
        attributePicturePath,
        attributeName
    };

public:
    RosterController(QObject *parent = 0);

    void setupWithClient(QXmppClient *qXmppClient);

    Q_INVOKABLE void addContact(const QString& jid, const QString& name);
    Q_INVOKABLE void removeContact(const QString& jid);
    Q_INVOKABLE bool isGroup(QString const &jid);

    Q_INVOKABLE QString getAvatarImagePathForJid(QString const &jid);
    Q_INVOKABLE QString getNameForJid(QString const &jid);

    Q_INVOKABLE int getAvailability(const QString& jid);

    void requestRoster();
    QQmlListProperty<RosterItem> getRosterList();

    bool updateSubscriptionForJid(const QString &bareJid, QXmppRosterIq::Item::SubscriptionType subscription);
    bool updateStatusForJid(const QString& jid, const QString& status);
    bool updateAvailabilityForJid(const QString &jid, RosterItem::Availability availability);


    bool updateNameForJid(const QString &jid, const QString &name);

#ifdef DBUS
    QList<RosterItem *> fetchRosterList();
#endif

signals:
    void rosterListChanged();
    void signalShowMessage(QString headline, QString body);
    void subscriptionUpdated(RosterItem::Subscription subs);

public slots:
    void addGroupAsContact(QString groupJid, QString groupName);
    void removeGroupFromContacts(QString groupJid);
    void handleRosterReceived();
    void handlePresenceReceived(const QXmppPresence &presence);
    void handlePresenceChanged(const QString &jid, const QString& resource);

private:
    void handleJidAdded(const QString &bareJid);
    void handleJidUpdated(const QString &bareJid);
    void handleJidRemoved(const QString &bareJid);

    void sendUnavailableAndUnsubscribeToJid(const QString& jid);

    void handleVCardChanged(const QXmppVCardIq &vCard);

    void bindJidUpdateMethodes();

    bool checkHashDiffers(QString const &jid, QString const &newHash);
    void sortRosterList();

    QString getTypeForJid(itemAttribute const &attribute, QString const &jid);
    bool isJidInRoster(const QString& bareJid);

    bool appendToRosterIfNotAlreadyIn(const QString& jid);
    void dumpRosterList();

    QXmppClient* qXmppClient_;
    QXmppRosterManager *rosterManager_;
    QXmppVCardManager *vCardManager_;

    QList<RosterItem*> rosterList_;
};

#endif // ROSTERCONTROLLER_H
