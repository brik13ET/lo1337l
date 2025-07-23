#ifndef ACTOR_H
#define ACTOR_H

#include <QThread>
#include <serial.h>

#include <array>
#include <functional>

class SERIAL_EXPORT Actor : public QObject
{
public:
    // Connection, self-address
    explicit Actor(Serial*,uint8_t);
    explicit Actor(QSerialPort*,uint8_t);
    explicit Actor(QString,uint8_t);
    ~Actor();

    class SERIAL_EXPORT State {
    public:
        enum Mode : uint8_t {
            Standard    = 0,
            Control     = 1,
            Calibration = 2,
            Loader      = 3
        };
        Mode mode;
        struct Parts {
            bool
                Module : 1,
                PSU    : 1,
                ROM    : 1,
                TempSens:1,
                VSens  : 1,
                Atten1 : 1,
                Atten2 : 1,
                Res    : 1;
        };
        struct Parts parts;
        struct Version {
            uint8_t
                major,
                minor;
        };
        struct Version ver;

        uint16_t temp;
        uint16_t voltage;

        explicit State(QByteArray);
        operator QByteArray();
        explicit State(Mode _mode, Parts _parts, uint16_t _temp, uint16_t _voltage, Version _ver);
        explicit State();
    };

    class SERIAL_EXPORT Settings {
    public:
        uint8_t output[2];
        uint16_t attenuator[2];

        explicit Settings(QByteArray);
        explicit Settings();

        operator QByteArray();
    };

    virtual Serial::Message handshakeMsg  (Serial::Message) = 0;
    virtual Serial::Message getStateMsg   (Serial::Message) = 0;
    virtual Serial::Message setSettingsMsg(Serial::Message) = 0;

    uint8_t getAddress();

    void setPort(Serial *serial);
    void setPort(QString name);
protected:

    uint8_t addr;
    Serial* con;
    Actor::State* state;
    Actor::Settings* settings;
    void skipMsg();

private:
    void rxd(Serial::Message);
    using worker = Serial::Message (Actor::*)(Serial::Message);
    static std::vector<worker> lookupCmd;
    bool skipMsgFlag;
};

#endif // ACTOR_H
