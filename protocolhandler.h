#ifndef PROTOCOLHANDLER_H
#define PROTOCOLHANDLER_H

#include <QString>
#include <QByteArray>
#include <QImage>
#include <QMap>
#include "messagetransceiver.h"

#include "basemessage.h"
#include "imagemessage.h"
#include "filemessage.h"
#include "custommessage.h"

// An interface for sending and receiving messages

class ProtocolHandler: public QObject
{
    Q_OBJECT
public:
    explicit ProtocolHandler(QObject *parent = 0);

    inline void setMessageTransceiver(MessageTransceiver *messageTransceiver) {
        if (mMessageTransceiver) {
            disconnect(this, SIGNAL(sendMessage(QByteArray)), mMessageTransceiver, SLOT(sendMessage(QByteArray)));
            disconnect(mMessageTransceiver, SIGNAL(receiveMessage(QByteArray)), this, SLOT(receiveMessage(QByteArray)));
            delete mMessageTransceiver;
            mMessageTransceiver = 0;
        }
        mMessageTransceiver = messageTransceiver;
        connect(this, SIGNAL(sendMessage(QByteArray)), mMessageTransceiver, SLOT(sendMessage(QByteArray)));
        connect(mMessageTransceiver, SIGNAL(receiveMessage(QByteArray)), this, SLOT(receiveMessage(QByteArray)));
    }

    inline MessageTransceiver * getMessageTransceiver() {
        return mMessageTransceiver;
    }

public slots:
    void receiveMessage(const QByteArray &message);
    void sendCustomMessage(const QMap<QString, QByteArray> &customContent);
    void sendImageMessage(const QImage &image);
    void sendFileMessage(const QList<QUrl> filePaths, const QMap<QString,QByteArray> &mimeContent);
signals:
    void sendMessage(const QByteArray &message);
    void receiveCustomMessage(CustomMessage customMessage);
    void receiveImageMessage(ImageMessage imageMessage);
    void receiveFileMessage(FileMessage fileMessage);
    
private:
    MessageTransceiver *mMessageTransceiver;
};

#endif // PROTOCOLHANDLER_H
