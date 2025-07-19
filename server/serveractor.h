#ifndef SERVERACTOR_H
#define SERVERACTOR_H

#include <QObject>
#include "actor.h"

class ServerActor : public Actor
{
    Q_OBJECT
public:
    void run() override;
    void setAddress(uint8_t);
    void setPort(Serial *);
    void setPort(QString);
    Actor::State getState();
    Actor::Settings getSettings();
    QStringList getComs();
    ServerActor(Serial *serial, uint8_t addr);

protected:
    void rxd(Serial::Message);

signals:
    void recived(Serial::Message);
    void newState(QByteArray);
};

#endif // SERVERACTOR_H
