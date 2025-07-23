#include "actor.h"
#include "nullexception.h"
#include "rangeexception.h"

#include <array>

#include <QDebug>


using worker = Serial::Message (Actor::*)(Serial::Message);
std::vector<worker> Actor::lookupCmd = {
    &Actor::handshakeMsg,
    &Actor::getStateMsg,
    &Actor::setSettingsMsg
};

SERIAL_EXPORT Actor::Actor(Serial* con, uint8_t addr)
    : addr(addr)
    , con(con)
    , state(new State())
    , settings(new Settings())
    , skipMsgFlag(false)
{
    connect(
        con, &Serial::rxd,
        this,&Actor::rxd
    );
}

SERIAL_EXPORT  Actor::Actor(QSerialPort * sport, uint8_t addr)
    : Actor(new Serial(sport), addr) { }

SERIAL_EXPORT Actor::Actor(QString nport, uint8_t addr)
    : Actor(new Serial(nport), addr) { }

SERIAL_EXPORT Actor::~Actor()
{
    disconnect(
        con, &Serial::rxd,
        this,&Actor::rxd
    );
}

SERIAL_EXPORT void Actor::rxd(Serial::Message msg)
{
    if (msg.getCmdNo() >= lookupCmd.size()) {
        qDebug() << QString("Unknown command: \"%1\"").arg(msg.getCmdNo());
        return;
    }
    if (msg.getAddress() != addr)
    {
        qDebug() << QString("Address unmatch: \"%1\" != \"%2\"")
                    .arg(msg.getAddress()).arg(addr);
        return;
    }

    auto method = lookupCmd[msg.getCmdNo()];
    if (method == nullptr)
        return;
    Serial::Message ret = (this->*method)(msg);
    if(!skipMsgFlag)
        con->transmit(ret);
    skipMsgFlag = false;
}


uint8_t Actor::getAddress()
{
    return addr;
}

void Actor::skipMsg()
{
    this->skipMsgFlag = true;
}

SERIAL_EXPORT Actor::State::operator QByteArray()
{
    QByteArray ret;
    uint8_t partsBit =
            (parts.Module << 0) |
            (parts.PSU    << 1) |
            (parts.ROM    << 2) |
            (parts.TempSens<<3) |
            (parts.VSens  << 4) |
            (parts.Atten1 << 5) |
            (parts.Atten2 << 6) |
            (parts.Res    << 7) ;
    ret.append(mode);
    ret.append(partsBit);
//    QByteArray tempArr((char*)&temp, sizeof(temp));
//    ret.append(tempArr);

    union shortBytes {
        uint16_t u16;
        uint8_t  u8[2];
    };
    shortBytes a, b;
    a.u16 = temp;
    ret.append(a.u8[0]);
    ret.append(a.u8[1]);
    a.u16 = voltage;
    ret.append(a.u8[0]);
    ret.append(a.u8[1]);

    ret.append(ver.major);
    ret.append(ver.minor);

    qDebug() << "asByte " << "rt: " << a.u16 << hex << a.u16 << dec;
    qDebug() << "asByte " << "rv: " << b.u16 << hex << b.u16 << dec;
    return ret;
}


SERIAL_EXPORT Actor::State::State(QByteArray raw)
{
    mode = (Actor::State::Mode)raw.at(0);
    parts.Module   = (raw.at(1) & (1 << 0)) != 0;
    parts.PSU      = (raw.at(1) & (1 << 1)) != 0;
    parts.ROM      = (raw.at(1) & (1 << 2)) != 0;
    parts.TempSens = (raw.at(1) & (1 << 3)) != 0;
    parts.VSens    = (raw.at(1) & (1 << 4)) != 0;
    parts.Atten1   = (raw.at(1) & (1 << 5)) != 0;
    parts.Atten2   = (raw.at(1) & (1 << 6)) != 0;
    parts.Res      = (raw.at(1) & (1 << 7)) != 0;
    union shortBytes {
        uint16_t u16;
        uint8_t  u8[2];
    };

    shortBytes a, b;
    a.u8[0] = raw.at(2);
    a.u8[1] = raw.at(3);
    b.u8[0] = raw.at(4);
    b.u8[1] = raw.at(5);
    temp      = a.u16;
    voltage   = b.u16;
    ver.major = raw.at(6);
    ver.minor = raw.at(7);


    qDebug() << "asObj " << "temp: " << temp;
    qDebug() << "asObj " << "voltage: " << voltage;
}

SERIAL_EXPORT Actor::State::State(
    Mode _mode       ,
    Parts _parts     ,
    uint16_t _temp   ,
    uint16_t _voltage,
    Version _ver
)
    : mode(_mode)
    , parts(_parts)
    , temp(_temp)
    , voltage(_voltage)
    , ver(_ver)
{ }

Actor::State::State()
    : State(
          Mode::Standard,
          {1,1,1,1,1,1,1,1},
          0,
          0,
          {0, 0}
    )
{ }

SERIAL_EXPORT Actor::Settings::operator QByteArray()
{
    QByteArray outputQ(const_cast<const char*>((char *)&output[0]    ) , sizeof(output    ));
    QByteArray attenQ (const_cast<const char*>((char *)&attenuator[0]) , sizeof(attenuator));
    return outputQ + attenQ;
}

SERIAL_EXPORT Actor::Settings::Settings(QByteArray msg)
{
    if (msg.size() != sizeof(output    ) + sizeof(attenuator) ) throw RangeException("Array size not match all `Settings` value");
    QByteArray outputQ(const_cast<const char*>((char *)&output[0]    ) , sizeof(output    ));
    outputQ.replace(0, outputQ.size(), msg.left ( outputQ.size() ));
    QByteArray attenQ (const_cast<const char*>((char *)&attenuator[0]) , sizeof(attenuator));
    attenQ .replace(0, attenQ .size(), msg.right( attenQ .size() ));
}

SERIAL_EXPORT Actor::Settings::Settings()
{

    for (int i = 0; i < sizeof(output)/sizeof(output[0]); ++i) {
        output[i] = 0;
    }


    for (int i = 0; i < sizeof(attenuator)/sizeof(attenuator[0]); ++i) {
        attenuator[i] = 0;
    }
}


SERIAL_EXPORT void Actor::setPort(Serial * serial)
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

SERIAL_EXPORT void Actor::setPort(QString name)
{
    if (con->getPort().portName() == name)
        return;
    setPort(new Serial(name));
}
