#ifndef BASEMESSAGE_H
#define BASEMESSAGE_H

#include <QByteArray>
#include <QDataStream>
#include <QMimeData>
#include <QMap>
#include <QStringList>
#include <QDebug>
#include <QDir>
#include "globals.h"

// This class forms the fundamental message which will be extended by specific messages
// - to be sent over network

class BaseMessage
{
public:
    BaseMessage();

    QByteArray serialize();
    void deserialize(const QByteArray & message);

    void setMimeContent(const QMimeData *mimeContent);
    void setMimeContent(const QMap<QString, QByteArray> &mimeContent);
    const QMap<QString, QByteArray> & getMimeContent() const;

    inline void setType(quint8 type) {
        mType = type;
    }

    inline quint8 getType() {
        return mType;
    }


protected:
    quint8 mType;
    quint64 mLength;
    QMimeData *mMimeData;
    QMap<QString, QByteArray> mMimeContent;

};

#endif // BASEMESSAGE_H
