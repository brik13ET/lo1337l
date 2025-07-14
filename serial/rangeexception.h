#ifndef RANGEEXCEPTION_H
#define RANGEEXCEPTION_H

#include "serial_global.h"
#include <QException>

class RangeException : public QException
{
public:
    RangeException(int highValue, int lowValue);
    RangeException(QString msg);
};

#endif // RANGEEXCEPTION_H
