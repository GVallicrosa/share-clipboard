#include "protocolhandler.h"

ProtocolHandler::ProtocolHandler(QObject *parent) :
    QObject(parent)
{
    // Initialize the message transceiver as null
    mMessageTransceiver = 0;
}

void ProtocolHandler::receiveMessage(const QByteArray &message) {
    
    // Check the type of the message!
    BaseMessage baseMessage;
    baseMessage.deserialize(message);
    int messageType = baseMessage.getType();
    
    switch(messageType) {
    case MESSAGE_IMAGE: {
        ImageMessage imageMessage;
        imageMessage.deserialize(message);
        emit receiveImageMessage(imageMessage); }
        break;
    case MESSAGE_FILE: {
        FileMessage fileMessage;
        fileMessage.deserialize(message);
        emit receiveFileMessage(fileMessage); }
        break;
    case MESSAGE_CUSTOM: {
        CustomMessage customMessage;
        customMessage.deserialize(message);
        emit receiveCustomMessage(customMessage); }
        break;
    default:
        qWarning() << "A message of unrecognized type has been received";
    }
}

void ProtocolHandler::sendCustomMessage(const QMap<QString, QByteArray> &customContent) {
    CustomMessage customMessage;
    customMessage.setMimeContent(customContent);
    emit sendMessage(customMessage.serialize());
}

void ProtocolHandler::sendImageMessage(const QImage &image) {
    ImageMessage imageMessage;
    imageMessage.setImage(image);
    emit sendMessage(imageMessage.serialize());
}

void ProtocolHandler::sendFileMessage(const QList<QUrl> filePaths, const QMap<QString,QByteArray> &mimeContent) {
    FileMessage fileMessage;
    fileMessage.setFilePaths(filePaths);
    fileMessage.setMimeContent(mimeContent);
    emit sendMessage(fileMessage.serialize());
}
