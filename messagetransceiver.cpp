#include "messagetransceiver.h"

MessageTransceiver::MessageTransceiver(QObject *parent, bool isClient) :
    QObject(parent)
{
    mNetworkSession = 0;
    mTcpServer = 0;
    mBlockSize = 0;
    mIsClient = isClient;
    mTcpSocket = 0;

    if (!isClient) {
        QNetworkConfigurationManager manager;
        if (manager.capabilities() & QNetworkConfigurationManager::NetworkSessionRequired) {
            // Get saved network configuration
            QSettings settings(QSettings::UserScope, QLatin1String("Trolltech"));
            settings.beginGroup(QLatin1String("QtNetwork"));
            const QString id = settings.value(QLatin1String("DefaultNetworkConfiguration")).toString();
            settings.endGroup();

            // If the saved network configuration is not currently discovered use the system default
            QNetworkConfiguration config = manager.configurationFromIdentifier(id);
            if ((config.state() & QNetworkConfiguration::Discovered) !=
                QNetworkConfiguration::Discovered) {
                config = manager.defaultConfiguration();
            }

            mNetworkSession = new QNetworkSession(config, this);
            connect(mNetworkSession, SIGNAL(opened()), this, SLOT(sessionOpened()));


            mNetworkSession->open();
        } else {
            sessionOpened();
        }

        connect(mTcpServer, SIGNAL(newConnection()), this, SLOT(newConnection()));
    } else {

        mTcpSocket = new QTcpSocket(this);
        connect(mTcpSocket, SIGNAL(readyRead()), this, SLOT(readyRead()));
        connect(mTcpSocket, SIGNAL(connected()), this, SIGNAL(connected()));

        QNetworkConfigurationManager manager;
        if (manager.capabilities() & QNetworkConfigurationManager::NetworkSessionRequired) {
            // Get saved network configuration
            QSettings settings(QSettings::UserScope, QLatin1String("Trolltech"));
            settings.beginGroup(QLatin1String("QtNetwork"));
            const QString id = settings.value(QLatin1String("DefaultNetworkConfiguration")).toString();
            settings.endGroup();

            // If the saved network configuration is not currently discovered use the system default
            QNetworkConfiguration config = manager.configurationFromIdentifier(id);
            if ((config.state() & QNetworkConfiguration::Discovered) !=
                QNetworkConfiguration::Discovered) {
                config = manager.defaultConfiguration();
            }

            mNetworkSession = new QNetworkSession(config, this);
            connect(mNetworkSession, SIGNAL(opened()), this, SLOT(sessionOpened()));

            mNetworkSession->open();
        }
    }
}

MessageTransceiver::~MessageTransceiver() {
    if(mTcpServer) {
        mTcpServer->close();
    }
    if (mTcpSocket) {
        mTcpSocket->close();
    }
    
    // Remove the client sockets
    while(!mSockets.empty()) {
        delete mSockets.back();
        mSockets.pop_back();
    }
}

void MessageTransceiver::sessionOpened()
{
    if (!mIsClient) {
        // Save the used configuration
        if (mNetworkSession) {
            QNetworkConfiguration config = mNetworkSession->configuration();
            QString id;
            if (config.type() == QNetworkConfiguration::UserChoice)
                id = mNetworkSession->sessionProperty(QLatin1String("UserChoiceConfiguration")).toString();
            else
                id = config.identifier();

            QSettings settings(QSettings::UserScope, QLatin1String("Trolltech"));
            settings.beginGroup(QLatin1String("QtNetwork"));
            settings.setValue(QLatin1String("DefaultNetworkConfiguration"), id);
            settings.endGroup();
        }

        mTcpServer = new QTcpServer(this);
        if (!mTcpServer->listen(QHostAddress::Any, TCP_PORT)) {
            qWarning() << "it ain't working";
            return;
        }
        
        QString ipAddress;
        QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
        // use the first non-localhost IPv4 address
        for (int i = 0; i < ipAddressesList.size(); ++i) {
            if (ipAddressesList.at(i) != QHostAddress::LocalHost &&
                ipAddressesList.at(i).toIPv4Address()) {
                ipAddress = ipAddressesList.at(i).toString();
                break;
            }
        }
        // if we did not find one, use IPv4 localhost
        if (ipAddress.isEmpty())
            ipAddress = QHostAddress(QHostAddress::LocalHost).toString();
        qWarning() << ipAddress << mTcpServer->serverPort();
    //! [1]
    } else {
        // Save the used configuration
        QNetworkConfiguration config = mNetworkSession->configuration();
        QString id;
        if (config.type() == QNetworkConfiguration::UserChoice)
            id = mNetworkSession->sessionProperty(QLatin1String("UserChoiceConfiguration")).toString();
        else
            id = config.identifier();

        QSettings settings(QSettings::UserScope, QLatin1String("Trolltech"));
        settings.beginGroup(QLatin1String("QtNetwork"));
        settings.setValue(QLatin1String("DefaultNetworkConfiguration"), id);
        settings.endGroup();
    }
}

void MessageTransceiver::newConnection() {
    //We are the server, so we use the vector instead
    QTcpSocket *tcpSocket = (mTcpServer->nextPendingConnection());
    mSockets.push_back(tcpSocket);
//    mTcpSocket = mTcpServer->nextPendingConnection();
    emit connected();
    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(readyRead()));
    connect(tcpSocket, SIGNAL(disconnected()), this, SLOT(disconnected()));
}

void MessageTransceiver::connectTo(QString ipAddress, QString portNumber) {
    if (mTcpSocket) {
        mTcpSocket->connectToHost(ipAddress,portNumber.toInt());
//        emit connected();
    }
}

quint16 MessageTransceiver::getPort()
{
    return mTcpSocket ? mTcpSocket->peerPort() : -1;
}

void MessageTransceiver::sendMessage(QByteArray data) {
    QByteArray msg;
    QDataStream out(&msg, QIODevice::WriteOnly);
    out << (quint64)data.size() << data;
    
    // IF we are client, we send it simply to the server
    if (mIsClient) {
        if (mTcpSocket)
            mTcpSocket->write(msg);
    } else {
        // Server has to send to all clients
        QVector<QTcpSocket *>::iterator itr;
        for (itr = mSockets.begin(); itr != mSockets.end(); itr++) {
            (*itr)->write(msg);
        }
    }
}

void MessageTransceiver::sendMessageExcept(QByteArray data, const QHostAddress &client ) {
    QByteArray msg;
    QDataStream out(&msg, QIODevice::WriteOnly);
    out << (quint64)data.size() << data;
    
    // IF we are client, we send it simply to the server
    if (mIsClient) {
        if (mTcpSocket)
            mTcpSocket->write(msg);
    } else {
        // Server has to send to all clients
        QVector<QTcpSocket*>::iterator itr;
        for (itr = mSockets.begin(); itr != mSockets.end(); itr++) {
            if ((*itr)->peerAddress() != client)
                (*itr)->write(msg);
        }
    }
}

void MessageTransceiver::readyRead() {
    QTcpSocket *client = qobject_cast<QTcpSocket*>(sender());
    QDataStream in(client);
    // We are reading the clipboard of the other end!
    if (mBlockSize == 0) {
        if (client->bytesAvailable() < (int)sizeof(quint64)) {
            return ;
        }
        in >> mBlockSize;
    }

    if (client->bytesAvailable() < mBlockSize) {
        return;
    }

    QByteArray msg;
    in >> msg;
    mBlockSize = 0;
    // If we are the server, we have to send the received message to everyone else (except the one who sends it!)
    if (!mIsClient) {
        sendMessageExcept(msg,client->peerAddress());
    }
    emit receiveMessage(msg);
}

void MessageTransceiver::disconnected() {
    if (mTcpSocket) {
        disconnect(mTcpSocket, SIGNAL(readyRead()), this, SLOT(readyRead()));
        disconnect(mTcpSocket, SIGNAL(disconnected()), this, SLOT(disconnected()));
    } else if (!mIsClient) {
        QTcpSocket *client = qobject_cast<QTcpSocket*>(sender());
        QVector<QTcpSocket *>::iterator itr;
        for (itr = mSockets.begin(); itr != mSockets.end(); itr++) {
            if ((*itr)->peerAddress() == client->peerAddress()) {
                // Client is disconnected, remove it
                delete (*itr);
                (*itr) = NULL;
                mSockets.erase(itr);
                break;
            }
        }
    }
    // TODO emit a message that the user is disconnected
}
