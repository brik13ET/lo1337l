#ifndef NULLEXCEPTION_H
#define NULLEXCEPTION_H

#include <QException>

class NullException : public QException
{
public:
    explicit NullException(QString msg = "");
};

#endif // NULLEXCEPTION_H
