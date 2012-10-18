#ifndef PROTOCOLHANDLER_H
#define PROTOCOLHANDLER_H

#include <QString>
#include <QByteArray>
#include <QImage>
#include <QMap>
#include "messagetransceiver.h"

// An interface for sending and receiving messages

class ProtocolHandler: public QObject
{
    Q_OBJECT
public:
    explicit ProtocolHandler(QObject *parent = 0);

public slots:
    void receiveMessage(const QByteArray &message);
    void sendCustomMessage(QString userName, QByteArray customMessage);
    void sendImageMessage(QString userName, const QImage &image);
    void sendFileMessage(QString userName, const QByteArray &fileContent, const QMap<QString,QByteArray> &mimeContent);

signals:
    void sendMessage(const QByteArray &message);
    void receiveCustomMessage(QString userName, QByteArray customMessage);
    void receiveImageMessage(QString userName, const QImage &image);
    void receiveFileMessage(QString userName, const QByteArray &fileContent, const QMap<QString, QByteArray> &mimeContent);

private:
    MessageTransceiver mMessageTransceiver;
};

#endif // PROTOCOLHANDLER_H
