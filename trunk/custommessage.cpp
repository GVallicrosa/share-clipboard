#include "custommessage.h"

CustomMessage::CustomMessage()
{
    setType(MESSAGE_CUSTOM);
}

QByteArray CustomMessage::serialize() {
    QByteArray message = BaseMessage::serialize();
    QDataStream dataStream(&message, QIODevice::WriteOnly);
    dataStream << mMimeContent;
    // After updating the message, go back to the beginning and update the size
    mLength = (quint64)(message.length() - (quint64) sizeof(mLength));
    dataStream.device()->seek(0);
    dataStream << mLength;
    return message;
}

void CustomMessage::deserialize(const QByteArray & message) {
    QByteArray msg(message);
    QDataStream dataStream(&msg, QIODevice::ReadOnly);
    dataStream.skipRawData(sizeof(mType));
    dataStream >> mMimeContent;
}
