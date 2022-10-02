#pragma once

#include "QXmppClient.h"

class XmppClient : public QXmppClient
{
public:
    XmppClient(QObject *parent = 0);
};
