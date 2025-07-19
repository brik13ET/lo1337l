#include "actor.h"
#include "rangeexception.h"

#include <array>

#include <QDebug>

SERIAL_EXPORT
const std::array<Serial::Message (Actor::*)(Serial::Message), 3>
    Actor::lookupCmd = {{
        &Actor::handshakeMsg,
        &Actor::getStateMsg ,
        &Actor::setSettingsMsg ,
}};

SERIAL_EXPORT Actor::Actor(Serial* con, uint8_t addr)
    : addr(addr)
    , con(con)
{
    connect(
        con, &Serial::rxd,
        this,&Actor::rxd
    );
}

Actor::Actor(QSerialPort * sport, uint8_t addr)
    : addr(addr)
    , con (new Serial(sport))
{ }

Actor::Actor(QString nport, uint8_t addr)
    : addr(addr)
    , con (new Serial(nport))
{ }

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
    Serial::Message ret = (this->*method)(msg);
    con->transmit(ret);
}

SERIAL_EXPORT Serial::Message Actor::handshakeMsg(Serial::Message msg)
{

    return Serial::Message (
        msg.getAddress(),
        Serial::Message::handshake().cmd,
        Serial::Message::handshake().op,
        QByteArray {}
    );
}

SERIAL_EXPORT Serial::Message Actor::getStateMsg(Serial::Message msg)
{
    // Build `State` package
    auto state = static_cast<QByteArray>(this->state);
    msg.setData(state);
    msg.setMeta(0);
    msg.setCmdNo(Serial::Message::getState().cmd);
    msg.setMeta (Serial::Message::getState().op );
    return msg;
}

SERIAL_EXPORT Serial::Message Actor::setSettingsMsg(Serial::Message msg)
{
    auto pkg = msg.getData();

    memcpy(
        settings.output,
        pkg.mid(0, sizeof(settings.output)).constData(),
        sizeof(settings.output)
    );


    memcpy(
        settings.attenuator,
        pkg.mid(2, sizeof(settings.attenuator)).constData(),
        sizeof(settings.attenuator)
    );

    msg.setMeta (Serial::Message::setSettings().op );
    msg.setCmdNo(Serial::Message::setSettings().cmd);
    return msg;
}

uint8_t Actor::getAddress()
{
    return addr;
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
    ret.append(partsBit);
    ret.append(temp);
    ret.append(voltage);
    ret.append(ver.major);
    ret.append(ver.minor);
    return ret;
}


SERIAL_EXPORT Actor::State::State(QByteArray raw)
{
    parts.Module   = ((raw.at(0) << 0) & 1) == 1;
    parts.PSU      = ((raw.at(0) << 1) & 1) == 1;
    parts.ROM      = ((raw.at(0) << 2) & 1) == 1;
    parts.TempSens = ((raw.at(0) << 3) & 1) == 1;
    parts.VSens    = ((raw.at(0) << 4) & 1) == 1;
    parts.Atten1   = ((raw.at(0) << 5) & 1) == 1;
    parts.Atten2   = ((raw.at(0) << 6) & 1) == 1;
    parts.Res      = ((raw.at(0) << 7) & 1) == 1;
    temp      = raw.at(1);
    voltage   = raw.at(2);
    ver.major = raw.at(3);
    ver.minor = raw.at(4);
}

SERIAL_EXPORT Actor::State::State()
{
    parts.Module   = 1;
    parts.PSU      = 1;
    parts.ROM      = 1;
    parts.TempSens = 1;
    parts.VSens    = 1;
    parts.Atten1   = 1;
    parts.Atten2   = 1;
    parts.Res      = 1;
    temp      = 27*2;
    voltage   = 50;
    ver.major = 1;
    ver.minor = 0;
}

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
