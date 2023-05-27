#include "Settings.h"

#include <QSettings>
#include <QString>
#include <QStringList>
#include <QStandardPaths>
#include <QCoreApplication>

Settings::Settings(QObject *parent) : QObject(parent)
{
    const QString settingsPath =
      QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/" + QCoreApplication::applicationName() + ".conf";

    settings_ = new QSettings(settingsPath, QSettings::NativeFormat);
}

Settings::~Settings()
{
    delete settings_;
}


QString Settings::getJid() const
{
    QString returnValue = "";

    if(settings_->value("authentication/jid").toString() != "NOT_SET")
    {
        returnValue = settings_->value("authentication/jid").toString();
    }

    return returnValue;
}

void Settings::setJid(QString Jid)
{
    settings_->setValue("authentication/jid", Jid);
    emit jidChanged(Jid);
}

QString Settings::getPassword() const
{
    QString returnValue = "";

    if(settings_->value("authentication/password").toString() != "NOT_SET")
    {
        returnValue = settings_->value("authentication/password").toString();
    }

    return returnValue;
}

void Settings::setPassword(QString Password)
{
    settings_->setValue("authentication/password", Password);
    emit passwordChanged(Password);
}

bool Settings::getSaveCredentials() const
{
    bool save = false;

    save = settings_->value("authentication/saveCredentials", false).toBool();

    return save;
}

void Settings::setSaveCredentials(bool SaveCredentials)
{
    settings_->setValue("authentication/saveCredentials", SaveCredentials);
    emit saveCredentialsChanged(SaveCredentials);
}

bool Settings::getDisplayChatNotifications() const
{
    bool save = true;

    save = settings_->value("notifications/displayChatNotifications", true).toBool();

    return save;
}

void Settings::setDisplayChatNotifications(bool DisplayChatNotifications)
{
    settings_->setValue("notifications/displayChatNotifications", DisplayChatNotifications);
    emit saveCredentialsChanged(DisplayChatNotifications);
}

bool Settings::getDisplayGroupchatNotifications() const
{
    bool save = true;

    save = settings_->value("notifications/displayGroupchatNotifications", false).toBool();

    return save;
}

void Settings::setDisplayGroupchatNotifications(bool DisplayGroupchatNotifications)
{
    settings_->setValue("notifications/displayGroupchatNotifications", DisplayGroupchatNotifications);
    emit saveCredentialsChanged(DisplayGroupchatNotifications);
}

QStringList Settings::getForceOnNotifications() const
{
    QStringList jids;

    jids = settings_->value("notifications/forceOnNotifications").toStringList();

    return jids;
}

void Settings::setForceOnNotifications(QStringList const & ForceOnNotifications)
{

    if (ForceOnNotifications.length() > 0)
    {
        settings_->setValue("notifications/forceOnNotifications", QVariant::fromValue(ForceOnNotifications));
    }
    else
    {
        settings_->remove("notifications/forceOnNotifications");
    }

    emit forceOnNotificationsChanged(ForceOnNotifications);
}

void Settings::removeForceOnNotifications(QString const & Jid)
{
    QStringList jids;

    if(settings_->contains("notifications/forceOnNotifications"))
    {
        jids = settings_->value("notifications/forceOnNotifications").toStringList();

        int idx = jids.indexOf(Jid);
        if(idx >= 0)
        {
            jids.removeAt(idx);
            setForceOnNotifications(jids);
        }
    }
}

void Settings::addForceOnNotifications(QString const & Jid)
{
    QStringList jids;

    if(settings_->contains("notifications/forceOnNotifications"))
    {
        jids = settings_->value("notifications/forceOnNotifications").toStringList();
    }

    if(!jids.contains(Jid))
    {
        jids.append(Jid);
        setForceOnNotifications(jids);
    }
}

QStringList Settings::getForceOffNotifications() const
{
    QStringList jids;

    jids = settings_->value("notifications/forceOffNotifications").toStringList();

    return jids;
}

void Settings::setForceOffNotifications(QStringList const & ForceOffNotifications)
{
    if (ForceOffNotifications.length() > 0)
    {
        settings_->setValue("notifications/forceOffNotifications", QVariant::fromValue(ForceOffNotifications));
    }
    else
    {
        settings_->remove("notifications/forceOffNotifications");
    }

    emit forceOffNotificationsChanged(ForceOffNotifications);
}

void Settings::removeForceOffNotifications(QString const & Jid)
{
    QStringList jids;

    if(settings_->contains("notifications/forceOffNotifications"))
    {
        jids = settings_->value("notifications/forceOffNotifications").toStringList();

        int idx = jids.indexOf(Jid);
        if(idx >= 0)
        {
            jids.removeAt(idx);
            setForceOffNotifications(jids);
        }
    }
}

void Settings::addForceOffNotifications(QString const & Jid)
{
    QStringList jids;

    if(settings_->contains("notifications/forceOffNotifications"))
    {
        jids = settings_->value("notifications/forceOffNotifications").toStringList();
    }

    if(!jids.contains(Jid))
    {
        jids.append(Jid);
        setForceOffNotifications(jids);
    }
}

void Settings::addForcePlainTextSending(const QString& jid)
{
    QStringList jids;

    if(settings_->contains("omeo/sendPlainText"))
    {
        jids = settings_->value("omeo/sendPlainText").toStringList();
    }

    if(!jids.contains(jid))
    {
        jids.append(jid);
        setSendPlainText(jids);
    }
}

void Settings::removeForcePlainTextSending(const QString& jid)
{
    QStringList jids;

    if(settings_->contains("omeo/sendPlainText"))
    {
        jids = settings_->value("omeo/sendPlainText").toStringList();

        int idx = jids.indexOf(jid);
        if(idx >= 0)
        {
            jids.removeAt(idx);
            setSendPlainText(jids);
        }
    }
}

void Settings::setSendPlainText(const QStringList& sendPlainText)
{
    if (sendPlainText.length() > 0)
    {
        settings_->setValue("omeo/sendPlainText", QVariant::fromValue(sendPlainText));
    }
    else
    {
        settings_->remove("omeo/sendPlainText");
    }

    emit sendPlainTextChanged(sendPlainText);
}

QStringList Settings::getSendPlainText() const
{
    return settings_->value("omeo/sendPlainText").toStringList();
}

bool Settings::getSendReadNotifications() const
{
    bool save = true;

    save = settings_->value("privacy/sendReadNotifications", true).toBool();

    return save;
}

void Settings::setSendReadNotifications(bool SendReadNotifications)
{
    settings_->setValue("privacy/sendReadNotifications", SendReadNotifications);
    emit saveCredentialsChanged(SendReadNotifications);
}

void Settings::setOmemoForSendingOff(bool omemoForSendingOff)
{
    settings_->setValue("omemo/omemoForSendingOff", omemoForSendingOff);
}

bool Settings::isOmemoForSendingOff()
{
    return settings_->value("omemo/omemoForSendingOff", false).toBool();
}

QStringList Settings::getImagePaths()
{
    QStringList searchPaths;

    if(!settings_->contains("storage/imagePaths"))
    {
        searchPaths.append(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));
        setImagePaths(searchPaths);
    }
    else
    {
        searchPaths = settings_->value("storage/imagePaths").toStringList();
    }

    return searchPaths;
}

void Settings::setImagePaths(QStringList const & ImagePaths)
{
    settings_->setValue("storage/imagePaths", QVariant::fromValue(ImagePaths));

    emit imagePathsChanged(ImagePaths);
}

void Settings::removeImagePath(QString const & Path)
{
    QStringList searchPaths;

    if(settings_->contains("storage/imagePaths"))
    {
        searchPaths = settings_->value("storage/imagePaths").toStringList();

        int idx = searchPaths.indexOf(Path);
        if(idx >= 0)
        {
            searchPaths.removeAt(idx);
            setImagePaths(searchPaths);
        }
    }
}

void Settings::addImagePath(QUrl const & Path)
{
    QStringList searchPaths;

    if(settings_->contains("storage/imagePaths"))
    {
        searchPaths = settings_->value("storage/imagePaths").toStringList();
    }

    if(!searchPaths.contains(Path.path()))
    {
        searchPaths.append(Path.path());

        setImagePaths(searchPaths);
    }
}

bool Settings::getCompressImages() const
{
    bool save;

    save = settings_->value("attachments/compressImages", false).toBool();

    return save;
}

void Settings::setCompressImages(bool CompressImages)
{
    settings_->setValue("attachments/compressImages", CompressImages);
    emit compressImagesChanged(CompressImages);
}

bool Settings::getSendOnlyImages() const
{
    bool save;

    save = settings_->value("attachments/sendOnlyImages", true).toBool();

    return save;
}

void Settings::setSendOnlyImages(bool SendOnlyImages)
{
    settings_->setValue("attachments/sendOnlyImages", SendOnlyImages);
    emit sendOnlyImagesChanged(SendOnlyImages);
}

unsigned int Settings::getLimitCompression() const
{
    unsigned int save;

    save = settings_->value("attachments/limitCompression", 400000u).toUInt();

    return save;
}

void Settings::setLimitCompression(unsigned int LimitCompression)
{
    settings_->setValue("attachments/limitCompression", LimitCompression);
    emit limitCompressionChanged(LimitCompression);
}

bool Settings::getSoftwareFeatureOmemoEnabled() const
{
    bool enabled{false};

    enabled = settings_->value("swfeatures/omemo", false).toBool();

    return enabled;
}

void Settings::setSoftwareFeatureOmemoEnabled(bool enableSoftwareFeatureOmemo)
{
    settings_->setValue("swfeatures/omemo", enableSoftwareFeatureOmemo);
    emit softwareFeatureOmemoEnabledChanged(enableSoftwareFeatureOmemo);
}

QString Settings::getResourceId() const
{
   QString returnValue = "";

    if(settings_->value("authentication/resourceId").toString() != "NOT_SET")
    {
        returnValue = settings_->value("authentication/resourceId").toString();
    }

    return returnValue;
}

void Settings::setResourceId(QString resourceId)
{
    settings_->setValue("authentication/resourceId", resourceId);
    emit resourceIdChanged(resourceId);
}

bool Settings::getAskBeforeDownloading() const
{
    bool enabled{false};

    enabled = settings_->value("attachments/askBeforeDownloading", false).toBool();

    return enabled;
}

void Settings::setAskBeforeDownloading(bool askBeforeDownloading)
{
    settings_->setValue("attachments/askBeforeDownloading", askBeforeDownloading);
    emit askBeforeDownloadingChanged(askBeforeDownloading);
}

QString Settings::getMamLastMsgId(QString mamJid) const
{
    QString key("mam/" + mamJid.replace("@", "-at-"));
    return settings_->value(key).toString();
}

void Settings::setMamLastMsgId(QString mamJid, QString lastMsgId)
{
    QString key("mam/" + mamJid.replace("@", "-at-"));
    settings_->setValue(key, lastMsgId);
}

bool Settings::getMsgToConsole() const
{
    bool enabled{false};

    enabled = settings_->value("debug/msgToConsole", false).toBool();

    return enabled;
}

void Settings::setMsgToConsole(bool msgToConsole)
{
    settings_->setValue("debug/msgToConsole", msgToConsole);
    emit msgToConsoleChanged(msgToConsole);
}
