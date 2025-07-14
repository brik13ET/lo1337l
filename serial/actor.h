#ifndef ACTOR_H
#define ACTOR_H

#include <QThread>
#include <serial.h>

#include <array>
#include <functional>

class SERIAL_EXPORT Actor : public QThread
{
public:
    // Connection, self-address
    explicit Actor(Serial*,uint8_t);
    ~Actor();

    class SERIAL_EXPORT State {
    public:
        enum Mode {
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
        explicit State();
        operator QByteArray();
    };

    class SERIAL_EXPORT Settings {
    public:
        uint8_t output[2];
        uint16_t attenuator[2];

        explicit Settings(QByteArray);
        explicit Settings();

        operator QByteArray();
    };

    Serial::Message handshake(Serial::Message);
    Serial::Message getState (Serial::Message);
    Serial::Message setSettings (Serial::Message);

protected:

    const uint8_t addr;
    Serial* const con;
    Actor::State state;
    Actor::Settings settings;

    void run() = 0;
    void rxd(Serial::Message);


    // to add cmds modify lookup table (index is cmdnumber)
    static const
    std::array<
        Serial::Message (Actor::*)(Serial::Message),
        3
    > lookupCmd ;
};

#endif // ACTOR_H
