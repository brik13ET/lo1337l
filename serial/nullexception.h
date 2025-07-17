#ifndef NULLEXCEPTION_H
#define NULLEXCEPTION_H

#include <QException>

#include "serial_global.h"

class SERIAL_EXPORT NullException : public QException
{
public:
    explicit NullException(QString msg = "");
};

#endif // NULLEXCEPTION_H
