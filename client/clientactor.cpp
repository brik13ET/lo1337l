#include "clientactor.h"


ClientActor::ClientActor(QSerialPort * port, uint8_t addr)
    : ClientActor(new Serial(port), addr) { }

ClientActor::ClientActor(QString name, uint8_t addr)
    : ClientActor(new Serial(name), addr) { }

ClientActor::ClientActor(Serial * con, uint8_t addr)
    : Actor(con, addr)
{

    connect(
        con, &Serial::txd,
        this,
       [this](Serial::Message msg){
            msg.dir = Serial::Message::Direction::TX;
            this->msgEmit(msg);
    });

    connect(
        con, &Serial::rxd,
        this,[this](Serial::Message msg){
        msg.dir = Serial::Message::Direction::RX;
        this->msgEmit(msg);
});

}

Serial::Message ClientActor::handshakeMsg(Serial::Message msg)
{
    Q_UNUSED(msg);
    getState();
    skipMsg();
    return Serial::Message::EMPTY;
}

Serial::Message ClientActor::handshake()
{
    Serial::Message msg (
        addr,
        Serial::Message::handshake.cmd,
        Serial::Message::handshake.op ,
        QByteArray ()
    );
    con->transmit(msg);
    return msg;
}

Serial::Message ClientActor::getStateMsg(Serial::Message msg)
{
    Q_UNUSED(msg);
    Actor::state = new Actor::State(msg.getData());
    emit state(*Actor::state);
    skipMsg();
    return Serial::Message::EMPTY;
}

Serial::Message ClientActor::getState()
{
    Serial::Message msg(
        addr,
        Serial::Message::getState.cmd,
        Serial::Message::getState.op ,
        QByteArray ()
    );

    con->transmit (msg);
    return msg;
}

Serial::Message ClientActor::setSettingsMsg(Serial::Message msg)
{
    Q_UNUSED(msg);
    skipMsg();
    return Serial::Message::EMPTY;
}

Serial::Message ClientActor::setSettings()
{
    Serial::Message msg(
        addr,
        Serial::Message::setSettings.cmd,
        Serial::Message::setSettings.op,
        static_cast<QByteArray>(*settings)
    );
    con->transmit(msg);
    return msg;
}

Actor::Settings &ClientActor::getSettings()
{
    return *this->settings;
}

void ClientActor::msgEmit(Serial::Message msg)
{
    emit this->msg(msg);
}

