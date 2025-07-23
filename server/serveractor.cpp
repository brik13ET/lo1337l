#include "serveractor.h"
#include <QSerialPort>
#include <QSerialPortInfo>
#include <cmath>
#include <QDebug>
#include <nullexception.h>

ServerActor::ServerActor(Serial* serial, uint8_t addr)
    : Actor(serial, addr)
{
    connect(
        serial, &Serial::rxd,
        this  , [this](Serial::Message msg){
            msg.dir = Serial::Message::Direction::RX;
            this->messagedEmit(msg);
    });
    connect(
        serial, &Serial::txd,
        this  , [this](Serial::Message msg){
            msg.dir = Serial::Message::Direction::TX;
            this->messagedEmit(msg);
    });

}

void ServerActor::setState(Actor::State st)
{
    this->state = new Actor::State (
        st.mode,
        st.parts,
        st.temp,
        st.voltage,
        st.ver
    );
}

void ServerActor::messagedEmit(Serial::Message msg)
{
    emit messaged(msg);
}

void ServerActor::setAddress(uint8_t addr)
{
    this->addr = addr;
}


Actor::State& ServerActor::getState()
{
    return *this->state;
}

Actor::Settings ServerActor::getSettings()
{
    return *this->settings;
}



Serial::Message ServerActor::handshakeMsg(Serial::Message msg)
{

    return Serial::Message (
        msg.getAddress(),
        Serial::Message::handshake.cmd,
        Serial::Message::handshake.op,
        QByteArray {}
    );
}

Serial::Message ServerActor::getStateMsg(Serial::Message msg)
{
    // Build `State` package
    auto state = static_cast<QByteArray>(*this->state);
    msg.setData(state);
    msg.setCmdNo(Serial::Message::getState.cmd);
    msg.setMeta (Serial::Message::getState.op );
    qDebug() << msg.toString();
    return msg;
}

Serial::Message ServerActor::setSettingsMsg(Serial::Message msg)
{
    auto pkg = msg.getData();

    memcpy(
        settings->output,
        pkg.mid(0, sizeof(settings->output)).constData(),
        sizeof(settings->output)
    );


    memcpy(
        settings->attenuator,
        pkg.mid(2, sizeof(settings->attenuator)).constData(),
        sizeof(settings->attenuator)
    );

    msg.setMeta (Serial::Message::setSettings.op );
    msg.setCmdNo(Serial::Message::setSettings.cmd);
    return msg;
}

