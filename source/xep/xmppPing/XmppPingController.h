#ifndef XMPPPINGCONTROLLER_H
#define XMPPPINGCONTROLLER_H

class XmppClient;

class XmppPingController
{
public:
    XmppPingController();

    void setupWithClient(XmppClient* client);
    void doPing();

private:
    void handlePingResponse();

    XmppClient *client_;

};

#endif // XMPPPINGCONTROLLER_H
