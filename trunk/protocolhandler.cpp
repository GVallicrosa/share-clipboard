#include "protocolhandler.h"

ProtocolHandler::ProtocolHandler(QObject *parent) :
    QObject(parent)
{
}

void ProtocolHandler::receiveMessage(const QByteArray &message) {

}

void ProtocolHandler::sendCustomMessage(QString userName, QByteArray customMessage) {

}

void ProtocolHandler::sendImageMessage(QString userName, const QImage &image) {

}

void ProtocolHandler::sendFileMessage(QString userName, const QByteArray &fileContent, const QMap<QString,QByteArray> &mimeContent) {

}
