#ifndef IMAGEMESSAGE_H
#define IMAGEMESSAGE_H

#include "basemessage.h"

#include <QImage>

class ImageMessage: public BaseMessage
{
public:
    ImageMessage();

    QByteArray serialize();
    void deserialize(const QByteArray & message);

    void setImage(const QImage &img);
    const QImage & getImage();
protected:
    QImage mImage;

};

#endif // IMAGEMESSAGE_H
