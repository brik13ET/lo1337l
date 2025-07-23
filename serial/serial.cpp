#include "serial.h"
#include <QException>
#include "nullexception.h"
#include "rangeexception.h"
#include <QDebug>
#include <QSerialPortInfo>


SERIAL_EXPORT const Serial::Message Serial::Message::EMPTY = Message();

SERIAL_EXPORT void Serial::parse()
{
    // Recieved part of message - append it to whole buffer
    if ( (msgBuff.size() < 4 || msgBuff.size() - 4 < msgBuff.at(3)) && port->bytesAvailable() > 0)
        msgBuff += port->readAll();
    // Buffer contains full message - emit `rxd` with message
    if (msgBuff.size() - 4 >= msgBuff.at(3)) {
        emit rxd(Message(
                     msgBuff.at(0),
                     msgBuff.at(1),
                     msgBuff.at(2),
                     msgBuff.mid(4, msgBuff.at(3))
        ));
        msgBuff.clear();
    }
}

SERIAL_EXPORT Serial::Serial(QSerialPort* port, QObject* parent)
    : QObject(parent)
    , port(port)
    , msgBuff(*(new QByteArray))
{
    if (this->port == nullptr)
        throw new NullException("Argument null: Serial::Serial(null)");
    port->open(QSerialPort::ReadWrite);

    connect(
        this->port, &QSerialPort::readyRead,
        this,       &Serial::parse
    );
}

SERIAL_EXPORT Serial::Serial(QString name, QObject *parent)
    : QObject(parent)
    , msgBuff(*(new QByteArray))
{
    port = new QSerialPort(name);
    port->open(QSerialPort::ReadWrite);

    connect(
        this->port, &QSerialPort::readyRead,
        this,       &Serial::parse
    );
}

SERIAL_EXPORT Serial::~Serial()
{
    disconnect(
        this->port, &QSerialPort::readyRead,
        this,       &Serial::parse
    );
    this->port->close();
}

SERIAL_EXPORT void Serial::transmit(Serial::Message msg)
{
    if (!port->isOpen())
        throw new NullException("Port is closed");
    port->write(static_cast<QByteArray>(msg));
    emit txd(msg);
}

SERIAL_EXPORT QSerialPort& Serial::getPort()
{
    return *port;
}

SERIAL_EXPORT const Serial::Message::CmdTempl Serial::Message::handshake
{
    /*.op =  */Serial::Message::MetaOp::Read,
    /*.cmd = */Serial::Message::Cmd::Handshake
};

SERIAL_EXPORT const Serial::Message::CmdTempl Serial::Message::getState
{
    /*.op  = */Serial::Message::MetaOp::Read,
    /*.cmd = */Serial::Message::Cmd::GetState
};

SERIAL_EXPORT const Serial::Message::CmdTempl Serial::Message::setSettings
{
    /*.op = */Serial::Message::MetaOp::Write,
    /*.cmd = */Serial::Message::Cmd::SetState
};

SERIAL_EXPORT Serial::Message::Message()
    : Address(0xff)
    , Cmd(0xff)
    , Meta(0xff)
    , Data(QByteArray ())
{ }

Serial::Message::Message(uint8_t Address, uint8_t CmdNo, uint8_t Meta, QByteArray Data)
    : Address(Address)
    , Cmd(CmdNo)
    , Meta(Meta)
    , Data(Data)
{
    if (Address > 31       || Address < 0       ) throw new RangeException(31, 0);
    if (CmdNo > 3          || CmdNo < 0         ) throw new RangeException(3,  0);
    if (Meta > 1           || Meta < 0          ) throw new RangeException(1,  0);
    if (Data.length() > 64 || Data.length() < 0 ) throw new RangeException(64, 0);
}

SERIAL_EXPORT Serial::Message::operator QByteArray()
{
    QByteArray ret;
    ret.append(Address);
    ret.append(Cmd);
    ret.append(Meta);
    ret.append((uint8_t)Data.length());
    return ret + Data;
}

SERIAL_EXPORT uint8_t Serial::Message::getAddress()
{
        return Address;
}

SERIAL_EXPORT uint8_t Serial::Message::getCmdNo()
{
        return Cmd;
}

SERIAL_EXPORT uint8_t Serial::Message::getMeta()
{
        return Meta;
}

SERIAL_EXPORT QByteArray Serial::Message::getData()
{
        return Data;
}

SERIAL_EXPORT void Serial::Message::setAddress(uint8_t v)
{
        Address = v;
}

SERIAL_EXPORT void Serial::Message::setCmdNo(uint8_t v)
{
        Cmd = v;
}

SERIAL_EXPORT void Serial::Message::setMeta(uint8_t v)
{
        Meta = v;
}

SERIAL_EXPORT void Serial::Message::setData(QByteArray v)
{
    Data = v;
}

SERIAL_EXPORT QString Serial::Message::toString()
{
    return QString("Addr: %1, Cmd: %2, Meta: %3\n%4")
            .arg(Address)
            .arg(Cmd)
            .arg(Meta)
            .arg(QString(Data.toHex()));
}


SERIAL_EXPORT QStringList Serial::getComs()
{
    auto coms = QSerialPortInfo::availablePorts();
    QStringList l;
    foreach (auto com, coms)
        l.append(com.portName());
    return l;
}
