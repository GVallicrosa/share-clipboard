#include "basemessage.h"

BaseMessage::BaseMessage()
{
//    mLength = (quint64) sizeof(mType);
}

QByteArray BaseMessage::serialize() {

    // Write the type of the message on the bytearray
    QByteArray message;
    QDataStream dataStream(&message, QIODevice::WriteOnly);
    dataStream << mType;
    return message;
}

void BaseMessage::deserialize(const QByteArray &message) {

    // Read the type of the message from bytearray
    QByteArray msg(message);
    QDataStream dataStream(&msg, QIODevice::ReadOnly);
    dataStream >> mType;
}

void BaseMessage::setMimeContent(const QMimeData *mimeContent) {

    // Go through the mimeContent and fill the map in
    QStringList formats = mimeContent->formats();
    int mimeContentSize = formats.length();
    for (int i = 0; i < mimeContentSize; i++) {
        QString format = formats.at(i);
        mMimeContent.insert(format, mimeContent->data(format));
    }
}

void BaseMessage::setMimeContent(const QMap<QString, QByteArray> &mimeContent) {

    // Simply copy the mimeContent
    mMimeContent = QMap<QString,QByteArray>(mimeContent);
}

const QMap<QString, QByteArray> & BaseMessage::getMimeContent() const {
    return mMimeContent;
}

