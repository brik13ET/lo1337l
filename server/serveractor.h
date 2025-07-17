#ifndef SERVERACTOR_H
#define SERVERACTOR_H

#include <QObject>
#include "actor.h"

class ServerActor : public Actor
{
    Q_OBJECT
private:
    Serial* port;
    uint8_t* address;

public:
    void run() override;
    void setAddress();
    void setPort();
    QStringList getComs();
    ServerActor(Serial *serial, uint8_t addr);

protected:
    void rxd(Serial::Message);

signals:
    void recived(Serial::Message);
};

#endif // SERVERACTOR_H
