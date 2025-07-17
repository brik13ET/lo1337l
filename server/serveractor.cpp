#include "serveractor.h"
#include <QSerialPort>
#include <QSerialPortInfo>
#include <cmath>

ServerActor::ServerActor(Serial* serial, uint8_t addr)
    : Actor(serial, addr)
{
}

void ServerActor::rxd(Serial::Message msg)
{
    Actor::rxd(msg);
    emit recived(msg);
}

// Emulate changing state
void ServerActor::run()
{
    uint16_t deltaT = 1, T = 0;
    while(!this->isInterruptionRequested()) {
        state.temp = (sinf(T) + 1) * 180 - 110; // Temp in -110..250
        state.voltage = (cosf(T) + 1) * 1000;
        T += deltaT;
        msleep(1000);
    }
}

