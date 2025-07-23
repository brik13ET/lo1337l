#ifndef SERVERACTOR_H
#define SERVERACTOR_H

#include <QObject>
#include "actor.h"

class ServerActor : public Actor
{
    Q_OBJECT
public:
    void setAddress(uint8_t);
    Actor::State& getState();
    Actor::Settings getSettings();
    QStringList getComs();
    ServerActor(Serial *serial, uint8_t addr);
    void setState(Actor::State);
    Serial::Message setSettingsMsg(Serial::Message msg) override;
    Serial::Message getStateMsg   (Serial::Message msg) override;
    Serial::Message handshakeMsg  (Serial::Message msg) override;
protected:

    void messagedEmit(Serial::Message);
signals:
    void messaged(Serial::Message);
    void newState(QByteArray);
};

#endif // SERVERACTOR_H
