#include "nullexception.h"
#include <QDebug>

NullException::NullException(QString msg)
{
    qDebug() << "NullException: " << msg;
}
