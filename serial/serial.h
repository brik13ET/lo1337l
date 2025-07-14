#ifndef SERIAL_H
#define SERIAL_H

#include "serial_global.h"
#include <QSerialPort>
#include <stdint.h>

class SERIAL_EXPORT Serial : public QObject
{
    Q_OBJECT
private:
    QSerialPort* port;
    QByteArray& msgBuff;

    void parse();

public:

    class Message {
    private:
        uint8_t Address;
        uint8_t Cmd;
        uint8_t Meta;
        QByteArray Data;
    public:

        enum Cmd : uint8_t {
            Handshake = 0,
            GetState  = 1,
            SetState  = 2,
        };

        enum MetaOp : uint8_t {
            Read  = 0,
            Write = 1
        };

        enum MetaStat : uint8_t {
            Success = 0,
            Error   = 1
        };

        struct CmdTempl {
            enum MetaOp op;
            enum Cmd cmd;
        };

        SERIAL_EXPORT static CmdTempl handshake  (void);
        SERIAL_EXPORT static CmdTempl getState   (void);
        SERIAL_EXPORT static CmdTempl setSettings(void);

        SERIAL_EXPORT explicit Message(
            uint8_t Address,
            uint8_t Cmd,
            uint8_t Meta,
            QByteArray Data
        );

        SERIAL_EXPORT explicit operator QByteArray();

        SERIAL_EXPORT uint8_t getAddress();
        SERIAL_EXPORT uint8_t getCmdNo();
        SERIAL_EXPORT uint8_t getMeta();
        SERIAL_EXPORT QByteArray getData();

        SERIAL_EXPORT void setAddress(uint8_t);
        SERIAL_EXPORT void setCmdNo(uint8_t);
        SERIAL_EXPORT void setMeta(uint8_t);
        SERIAL_EXPORT void setData(QByteArray);

        SERIAL_EXPORT QString toString();
    };

    explicit Serial(QSerialPort*, QObject* parent = nullptr);
    ~Serial();
    void txd(Message);
    explicit operator QSerialPort&();
    static QStringList getComs();

signals:
    void rxd(Message);

};

#endif // SERIAL_H
