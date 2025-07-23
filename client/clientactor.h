#pragma once
#include <actor.h>

class ClientActor : public Actor
{
    Q_OBJECT
public:
    explicit ClientActor(Serial*,uint8_t);
    explicit ClientActor(QSerialPort*,uint8_t);
    explicit ClientActor(QString,uint8_t);

    // RX: request actual state
    Serial::Message handshakeMsg  (Serial::Message msg) override;
    // TX: send handshake
    Serial::Message handshake     ();

    // RX: just emits after update
    Serial::Message getStateMsg   (Serial::Message msg) override;
    // TX: send getState
    Serial::Message getState      ();

    // RX: does nothing
    Serial::Message setSettingsMsg(Serial::Message msg) override;
    // TX: send setSettings with actual settings in actor
    Serial::Message setSettings   ();

    Actor::Settings& getSettings  ();

    void msgEmit(Serial::Message);

signals:
    void state(Actor::State);
    void msg(Serial::Message);

};

