#include "clipboardclient.h"

ClipboardClient::ClipboardClient(QObject *parent):
    QObject(parent) {
    mClipboard = 0;
    mProtocolHandler = 0;
}

void ClipboardClient::receiveCustomMessage(const CustomMessage &customMessage) {
    // TODO
    // Form a mimedata and store it in clipboard
    
    if (mClipboard) {
        emit showMessage("New message arrived", "Received new clipboard content", 2000);
    }
}

void ClipboardClient::receiveImageMessage(const ImageMessage &imageMessage) {
    if (mClipboard) {
        mClipboard->setImage(imageMessage.getImage());
        emit showMessage("New message arrived", "Received new image", 2000);
    }
}

void ClipboardClient::receiveFileMessage(const FileMessage &fileMessage) {

    if (mClipboard) {
        emit showMessage("New message arrived", "Received new file(s)", 2000);
    }
}

void ClipboardClient::sendClipboard() {    
    // TODO
    // 1 - Detect which kind of data have been copied
    // 2 - Send it to protocol handler
    // Maybe extend qmimedata for file 
    
    const QMimeData *mData = mClipboard->mimeData();
    if (mData->hasImage()) {
        QImage imageData = qvariant_cast<QImage>(mData->imageData());
        emit sendImageMessage(imageData);
        emit showMessage("Shared Clipboard", "Image has been sent.", 2000);
    } else {
        emit sendCustomMessage(convertMimetoMap(mData));
        emit showMessage("Shared Clipboard", "Clipboard has been sent.", 2000);
    }
    
}

QMap<QString, QByteArray> ClipboardClient::convertMimetoMap(const QMimeData *mimeData) {
    QMap<QString, QByteArray> mimeContent;
    QStringList formats = mimeData->formats();
    int length = formats.length();
    
    for (int i = 0; i < length; i++) {
        mimeContent.insert(formats[i], mimeData->data(formats[i]));
    }
    
    return mimeContent;
}

void ClipboardClient::connectProtocolHandlerSignals() {
    if (mProtocolHandler) {
        connect(mProtocolHandler, SIGNAL(receiveCustomMessage(CustomMessage)), this, SLOT(receiveCustomMessage(CustomMessage)));
        connect(mProtocolHandler, SIGNAL(receiveFileMessage(FileMessage)), this, SLOT(receiveFileMessage(FileMessage)));
        connect(mProtocolHandler, SIGNAL(receiveImageMessage(ImageMessage)), this, SLOT(receiveImageMessage(ImageMessage)));
        connect(this, SIGNAL(sendCustomMessage(QMap<QString,QByteArray>)), mProtocolHandler, SLOT(sendCustomMessage(QMap<QString,QByteArray>)));
        connect(this, SIGNAL(sendFileMessage(QList<QUrl>,QMap<QString,QByteArray>)), mProtocolHandler, SLOT(sendFileMessage(QList<QUrl>,QMap<QString,QByteArray>)));
        connect(this, SIGNAL(sendImageMessage(const QImage&)), mProtocolHandler, SLOT(sendImageMessage(const QImage&)));
    } else {
        qWarning() << "Trying to use null protocol handler";
    }
}

void ClipboardClient::disconnectProtocolHandlerSignals() {
    if (mProtocolHandler) {
        disconnect(mProtocolHandler, SIGNAL(receiveCustomMessage(CustomMessage)), this, SLOT(receiveCustomMessage(CustomMessage)));
        disconnect(mProtocolHandler, SIGNAL(receiveFileMessage(FileMessage)), this, SLOT(receiveFileMessage(FileMessage)));
        disconnect(mProtocolHandler, SIGNAL(receiveImageMessage(ImageMessage)), this, SLOT(receiveImageMessage(ImageMessage)));
        disconnect(this, SIGNAL(sendCustomMessage(QMap<QString,QByteArray>)), mProtocolHandler, SLOT(sendCustomMessage(QMap<QString,QByteArray>)));
        disconnect(this, SIGNAL(sendFileMessage(QList<QUrl>,QMap<QString,QByteArray>)), mProtocolHandler, SLOT(sendFileMessage(QList<QUrl>,QMap<QString,QByteArray>)));
        disconnect(this, SIGNAL(sendImageMessage(const QImage*)), mProtocolHandler, SLOT(sendImageMessage(QImage)));
    } else {
        qWarning() << "Trying to use null protocol handler";
    }
}

void ClipboardClient::setProtocolHandler(ProtocolHandler *protocolHandler) {
    // Delete if there already is
    if (mProtocolHandler) {
        disconnectProtocolHandlerSignals();
        delete mProtocolHandler;
        mProtocolHandler = 0;
    }
    mProtocolHandler = protocolHandler;
    connectProtocolHandlerSignals();
}

