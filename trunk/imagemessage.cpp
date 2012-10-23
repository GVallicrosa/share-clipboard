#include "imagemessage.h"

ImageMessage::ImageMessage()
{
    // The type of the image is set here!
    setType(MESSAGE_IMAGE);
}

QByteArray ImageMessage::serialize() {
    QByteArray message = BaseMessage::serialize();
    QDataStream dataStream(&message, QIODevice::Append);
    dataStream << mImage;
    // After updating the message, go back to the beginning and update the size
//    mLength = (quint64)(message.length() - (quint64) sizeof(mLength));
//    dataStream.device()->seek(0);
//    dataStream << mLength;
    return message;
}

void ImageMessage::deserialize(const QByteArray & message) {
    QByteArray msg(message);
    QDataStream dataStream(&msg, QIODevice::ReadOnly);
    dataStream.skipRawData(sizeof(mType));
    dataStream >> mImage;
}

void ImageMessage::setImage(const QImage &img) {
    // Copy the image
    mImage = QImage(img.size());
    mImage.fill(Qt::white);
    QPainter painter(&mImage);
    painter.drawImage(0,0,img);
}

const QImage &ImageMessage::getImage() const{
    return mImage;
}
