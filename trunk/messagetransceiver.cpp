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
}

Q_WS_X11
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

    //! [0] //! [1]
        mTcpServer = new QTcpServer(this);
        if (!mTcpServer->listen(QHostAddress::Any, TCP_PORT)) {
            qWarning() << "it ain't working";
            return;
        }
    //! [0]
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
    mTcpSocket = mTcpServer->nextPendingConnection();
    emit connected();
    connect(mTcpSocket, SIGNAL(readyRead()), this, SLOT(readyRead()));
}

void MessageTransceiver::connectTo(QString ipAddress, QString portNumber) {
    if (mTcpSocket) {
        mTcpSocket->connectToHost(ipAddress,portNumber.toInt());
//        emit connected();
    }
}

void MessageTransceiver::sendMessage(QByteArray data) {
    QByteArray msg;
    QDataStream out(&msg, QIODevice::WriteOnly);
    out << (quint64)data.size() << data;
    if (mTcpSocket)
        mTcpSocket->write(msg);
}

void MessageTransceiver::readyRead() {
    QDataStream in(mTcpSocket);
    // We are reading the clipboard of the other end!
    if (mBlockSize == 0) {
        if (mTcpSocket->bytesAvailable() < (int)sizeof(quint64)) {
            return ;
        }
        in >> mBlockSize;
    }

    if (mTcpSocket->bytesAvailable() < mBlockSize) {
        return;
    }

    QByteArray msg;
    in >> msg;
    mBlockSize = 0;
    emit receiveMessage(msg);
}
