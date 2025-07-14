#include "rangeexception.h"
#include <QDebug>

RangeException::RangeException(int highValue, int lowValue)
{
    qDebug() << "QRangeException: "
             << "Value out of range [" << highValue << ".." << lowValue << "]";
}

RangeException::RangeException(QString msg)
{
    qDebug() << msg;
}
