#ifndef CLIPBOARDCLIENT_H
#define CLIPBOARDCLIENT_H

#include <QObject>
#include <QClipboard>
#include <QCoreApplication>
#include <QByteArray>

#include "custommessage.h"
#include "imagemessage.h"
#include "filemessage.h"
#include "protocolhandler.h"

class ClipboardClient:public QObject
{
    Q_OBJECT
public:
    explicit ClipboardClient(QObject *parent=0);
    
    void setProtocolHandler(ProtocolHandler *protocolHandler);
    void getProtocolHandler();
    
    inline void setClipboard(QClipboard * clipboard) {
        mClipboard = clipboard;
    }

    QClipboard * getClipboard() {
        return mClipboard;
    }
    
protected:
    void connectProtocolHandlerSignals();
    void disconnectProtocolHandlerSignals();
    void publishNotification(QString title, QString description);
    QMap<QString, QByteArray> convertMimetoMap(const QMimeData *mimeData);
    
signals:
    void sendCustomMessage(const QMap<QString,QByteArray> &customContent);
    void sendImageMessage(const QImage &image);
    void sendFileMessage(const QList<QUrl> &filePaths, const QMap<QString, QByteArray> &mimeContent);
    void showMessage(QString title, QString description, quint32 msecs);
    
public slots:
    
    // NETWORK RELATED
    void receiveCustomMessage(const CustomMessage &customMessage);
    void receiveImageMessage(const ImageMessage &imageMessage);
    void receiveFileMessage(const FileMessage &fileMessage);
    void sendClipboard();
    
private:
    ProtocolHandler *mProtocolHandler;
    QClipboard *mClipboard;
};

#endif // CLIPBOARDCLIENT_H
