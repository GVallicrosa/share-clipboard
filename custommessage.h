#ifndef CUSTOMMESSAGE_H
#define CUSTOMMESSAGE_H

#include "basemessage.h"

// This message is a pure copy of clipboard with no modifications whatsoever

class CustomMessage : public BaseMessage
{
public:
    CustomMessage();

    QByteArray serialize();
    void deserialize(const QByteArray & message);

};

#endif // CUSTOMMESSAGE_H
