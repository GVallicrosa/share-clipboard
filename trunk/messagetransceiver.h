#ifndef MESSAGETRANSCEIVER_H
#define MESSAGETRANSCEIVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QNetworkConfigurationManager>
#include <QNetworkSession>
#include <QSettings>
#include <QHash>
#include <QMessageBox>

#define UDP_BROADCAST_PORT    4548
#define TCP_PORT    4547
#define TIMEOUT             1000 //Milliseconds

class MessageTransceiver : public QObject
{
    Q_OBJECT
public:
    explicit MessageTransceiver(QObject *parent = 0, bool isClient=false);
    ~MessageTransceiver();

protected:
    void startBroadcasting();
    void stopBroadcasting();
    void sessionOpened();

signals:
    void receiveMessage(QByteArray data);
    void connected();

public slots:
    void sendMessage(QByteArray data);
    void newConnection();
    void connectTo(QString ipAddress, QString portNumber);
    void readyRead();

private:
    QTcpServer *mTcpServer;
    QTcpSocket *mTcpSocket;
    QNetworkSession *mNetworkSession;
    QByteArray mReceivedData;
    quint64 mBlockSize;
    bool mIsClient;
};

#endif // MESSAGETRANSCEIVER_H
