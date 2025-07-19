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
        this  , &ServerActor::rxd
    );
}

void ServerActor::rxd(Serial::Message msg)
{
    Actor::rxd(msg);
    emit recived(msg);
}

// Emulate changing state
void ServerActor::run()
{
    //TODO: add custom logic for changind state
//    emit newState(static_cast<QByteArray>(this->state));
}

void ServerActor::setAddress(uint8_t addr)
{
    this->addr = addr;
}

void ServerActor::setPort(Serial * serial)
{
    if (con->getPort().portName().compare(serial->getPort().portName()) == 0)
        return;
    if (con != nullptr)
    {
        con->getPort().close();
        delete con;
        con = nullptr;
    }
    if (!serial->getPort().isOpen())
    {
        throw new NullException("New serial is not open");
    }
    con = serial;
}

void ServerActor::setPort(QString name)
{
    if (con->getPort().portName() == name)
        return;
    setPort(new Serial(name));
}

Actor::State ServerActor::getState()
{
    return this->state;
}

Actor::Settings ServerActor::getSettings()
{
    return this->settings;
}

