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
    
    inline QMap<QString, QString> getPeerIps() {
        return mPeers;
    }

protected:
    void startBroadcasting();
    void stopBroadcasting();
    void sessionOpened();

signals:
    void receiveMessage(QByteArray data);
    void connected();

public slots:
    void sendMessage(QByteArray data);
    void sendMessageExcept(QByteArray data, const QHostAddress &client);
    void newConnection();
    void connectTo(QString ipAddress, QString portNumber);
    quint16 getPort();
    void readyRead();
    void disconnected();

private:
    QTcpServer *mTcpServer;
    QTcpSocket *mTcpSocket;
    QNetworkSession *mNetworkSession;
    QByteArray mReceivedData;
    quint64 mBlockSize;
    bool mIsClient;
    
    // Needed if we are the server
    QVector<QTcpSocket *> mSockets;

    QMap<QString, QString> mPeers;
};

#endif // MESSAGETRANSCEIVER_H
