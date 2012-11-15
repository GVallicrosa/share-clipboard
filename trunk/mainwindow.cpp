#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QClipboard>
#include <QDebug>
#include "messagetransceiver.h"
#include <QFile>
#include <QProcess>
#include <QDir>
#include <QUrl>
#include <QDeclarativeView>

#include "imagemessage.h"
#include "filemessage.h"
#include "custommessage.h"

#ifdef ZEROCONF
#include "avahi/bonjourserviceregister.h"
#include "avahi/bonjourservicebrowser.h"
#include "avahi/bonjourserviceresolver.h"
#endif // ZEROCONF


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    
    mMessageTransceiver = new MessageTransceiver(this,true);
    mProtocolHandler = new ProtocolHandler();
    mClipboardClient = new ClipboardClient();
    mNotify = new NotificationWindow();
    
    mProtocolHandler->setMessageTransceiver(mMessageTransceiver);
    mClipboardClient->setProtocolHandler(mProtocolHandler);
    mClipboardClient->setClipboard(qApp->clipboard());
    
    // UI to MessageTransceiver connection
    connect(this, SIGNAL(connectTo(QString,QString)), mMessageTransceiver, SLOT(connectTo(QString,QString)));
    connect(mMessageTransceiver, SIGNAL(connected()), this, SLOT(connected()));
    
    connect(mClipboardClient, SIGNAL(showMessage(QString,QString,quint32)), mNotify, SLOT(showMessage(QString,QString,quint32)));
    connect(this, SIGNAL(showMessage(QString,QString,quint32)), mNotify, SLOT(showMessage(QString,QString,quint32)));
    
    // Get the application icon from resources
    mIcon = new QIcon(":/icons/clipshare.svg");

    // Create a system tray icon
    mTray = new QSystemTrayIcon(*mIcon,this);
    mTray->show();
    setWindowIcon(*mIcon);

    mAppPath = qApp->applicationDirPath();

    
    // Set the shortcuts
    mShareShortcut = new QxtGlobalShortcut(this);
    connect(mShareShortcut, SIGNAL(activated()), mClipboardClient, SLOT(sendClipboard()));
    mShareShortcut->setShortcut(QKeySequence("Ctrl+Shift+C"));

    mRecoverShortcut = new QxtGlobalShortcut(this);
    connect(mRecoverShortcut, SIGNAL(activated()), mClipboardClient, SLOT(recoverClipboard()));
    mRecoverShortcut->setShortcut(QKeySequence("Ctrl+Shift+Z"));
    
    mUnhideShortcut = new QxtGlobalShortcut(this);
    connect(mUnhideShortcut, SIGNAL(activated()), this, SLOT(toggleVisibility()));
    mUnhideShortcut->setShortcut(QKeySequence("Ctrl+Shift+H"));

#ifdef ZEROCONF
    qDebug("ZEROCONF");
//    ui->tabWidget->removeTab(0);
    ui->tabWidget->setCurrentIndex(1);

    bonjourBrowser = new BonjourServiceBrowser(this);

    connect( bonjourBrowser, SIGNAL(currentBonjourRecordsChanged(const QList<BonjourRecord> &)),
             this, SLOT(updateRecords(const QList<BonjourRecord> &)) );

    bonjourBrowser->browseForServiceType(QLatin1String("_shareClipboard._tcp"));

    ui->serverList->setFocus();

    bonjourResolver = new BonjourServiceResolver(this);
    connect( bonjourResolver, SIGNAL(bonjourRecordResolved(const QHostInfo &, int)),
             this, SLOT(connectToServer(const QHostInfo &, int)) );

#else
    qDebug("NO ZEROCONF");
    ui->tabWidget->removeTab(1);
//    ui->tabWidget->setCurrentIndex(0);
#endif
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::dataChanged() {
    const QMimeData *mData = mClipboard->mimeData();

    if (mData->hasImage()) {
        QImage image = mClipboard->image();
        QByteArray msg;
        QDataStream dataStream(&msg, QIODevice::WriteOnly);
        dataStream << QString("image") <<  image;
        emit sendMessage(msg);
    } else if (mData->hasFormat("text/uri-list") && mData->data("text/uri-list").contains(QByteArray("file://"))) {
        // This is a list of files
        // Read the files first! Send the content
        QByteArray fileNameList = mData->data("text/uri-list");
        QList<QString> list = QString(fileNameList).split(QRegExp("\r\n"), QString::SkipEmptyParts);
        QByteArray msg;
        QDataStream dataStream(&msg, QIODevice::WriteOnly);
        dataStream << QString("files");
        QList<QByteArray> fileList;
        for (int i = 0; i < list.length(); ++i) {
            list[i].remove(0, QString("file://").length());
            list[i] = QUrl::fromPercentEncoding(list[i].toAscii());
            QFile file(list[i]);
            file.open(QIODevice::ReadOnly);
            QByteArray fileContent = file.readAll();
            fileList.append(fileContent);
            file.close();
        }
        dataStream << fileList;
        // Now send whole clipboard
        QStringList mTypes = mData->formats();
        QMap<QString, QByteArray> msgContent;
        for (int i = 0; i < mTypes.length(); i++) {
            QByteArray data = mData->data(mTypes.at(i));
            msgContent.insert(mTypes[i], data);
        }
        dataStream << msgContent;
        emit sendMessage(msg);
    } else {
        QStringList mTypes = mData->formats();
        QMap<QString, QByteArray> msgContent;
        for (int i = 0; i < mTypes.length(); i++) {
            QByteArray data = mData->data(mTypes.at(i));
            msgContent.insert(mTypes[i], data);
        }

        qWarning() << msgContent;

        // Send the text!
        QByteArray msg;
        QDataStream *dataStream = new QDataStream(&msg, QIODevice::WriteOnly);
        (*dataStream) << QString("custom") << msgContent;
        emit sendMessage(msg);
        delete dataStream;
    }

    QString cmd = QString("notify-send \"Share Clipboard\" \"Clipboard has been sent!\" -i " + qApp->applicationDirPath() + "/icon.png");
    system(cmd.toStdString().c_str());
}

//void MainWindow::on_pushButton_clicked()
//{
//    QMimeData *mData = new QMimeData();
//    mData->setData("x-special/gnome-copied-files",QByteArray("copy\nfile:///home/ozan/Desktop/cagri.jpg"));
//    mClipboard->setMimeData(mData);
//}

void MainWindow::on_connectButton_clicked()
{
    QString ipAddress = ui->ipAddressEdit->text();
    QString portNumber = ui->portNumberEdit->text();
    emit connectTo(ipAddress, portNumber);

}

void MainWindow::on_becomeServerBtn_clicked()
{
    if (mMessageTransceiver) {
        disconnect(this, SIGNAL(connectTo(QString,QString)), mMessageTransceiver, SLOT(connectTo(QString,QString)));
        disconnect(mMessageTransceiver, SIGNAL(receiveMessage(QByteArray)), mProtocolHandler, SLOT(receiveMessage(QByteArray)));
        disconnect(mMessageTransceiver, SIGNAL(connected()), this, SLOT(connected()));
        disconnect(mProtocolHandler, SIGNAL(sendMessage(QByteArray)), mMessageTransceiver, SLOT(sendMessage(QByteArray)));
    }
    delete mMessageTransceiver;
    mMessageTransceiver = NULL;
    if (ui->becomeServerBtn->text() == "Become Server") {
        mMessageTransceiver = new MessageTransceiver(this, false);
        ui->ipAddressEdit->setEnabled(false);
        ui->portNumberEdit->setEnabled(false);
        ui->becomeServerBtn->setText("Become Client");
        setHidden(true);
        emit showMessage("Share Clipboard", "You have successfully become server", 2000);
//    #ifdef Q_WS_X11
//        QString cmd = QString("notify-send \"Share Clipboard\" \"You have successfully become server\" -i " + qApp->applicationDirPath() + "/icon.png");
//        system(cmd.toStdString().c_str());
//    #endif
//    #ifdef Q_WS_WIN
//        mTray->showMessage("Share Clipboard", "You have successfully become server!");
//    #endif
    } else {
        mMessageTransceiver = new MessageTransceiver(this, true);
        ui->ipAddressEdit->setEnabled(true);
        ui->portNumberEdit->setEnabled(true);
        ui->becomeServerBtn->setText("Become Server");
    }
    connect(this, SIGNAL(connectTo(QString,QString)), mMessageTransceiver, SLOT(connectTo(QString,QString)));
    connect(mMessageTransceiver, SIGNAL(receiveMessage(QByteArray)), mProtocolHandler, SLOT(receiveMessage(QByteArray)));
    connect(mMessageTransceiver, SIGNAL(connected()), this, SLOT(connected()));
    connect(mProtocolHandler, SIGNAL(sendMessage(QByteArray)), mMessageTransceiver, SLOT(sendMessage(QByteArray)));
}

void MainWindow::receiveMessage(QByteArray msg) {
//    QDataStream dataStream(&msg, QIODevice::ReadOnly);

//    // Read the content
//    QString type;
//    dataStream >> type;

//    if (type == QString("image")) {
//        QImage img;
//        dataStream >> img;
//        mIsClient = true;
//        mClipboard->clear();
//        mIsClient = true;
//        mClipboard->setImage(img);
//    } else if(type == QString("files")) {
//        QList<QByteArray> fileContents;
//        QMap<QString, QByteArray> content;
//        dataStream >> fileContents >> content;
//        QByteArray fileNameList = content["text/uri-list"];
//        //Check if linux
//#ifdef Q_WS_X11
//        QRegExp sep("\r\n");
//        QList<QString> fileNameListSplitted = QString(QUrl::fromPercentEncoding(fileNameList)).split(sep, QString::SkipEmptyParts);
//        QList<QString> fileNames;
//        for (int i = 0; i < fileNameListSplitted.length(); ++i) {
//            QList<QString> pathBits = fileNameListSplitted[i].split('/');
//            fileNames.append(pathBits[pathBits.length()-1]);
//        }
//#endif
//#ifdef Q_WS_WIN
//        return ;
//#endif
//        // Create a file saver
//        QDir dir;
//        if (!dir.exists(".temp")) {
//            dir.mkdir(".temp");
//        } else {
//            dir.cd(".temp");
//            QFileInfoList files = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files);
//            foreach (QFileInfo filename, files) {
//                dir.remove(filename.fileName());
//            }
//        }

//        // Have a list of file paths
//        QStringList filePaths;
//        for (int i = 0; i < fileContents.length(); i++) {
//            QFile file(qApp->applicationDirPath() + "/.temp/" + QString(fileNames[i]));
//            filePaths.append("file://" + qApp->applicationDirPath() + "/.temp/" + QString(fileNames[i]));
//            file.open(QIODevice::WriteOnly);
//            file.write(fileContents[i]);
//            file.close();
//        }
//        mMimeData = new QMimeData();
//        // traverse through it and fill the clipboard in!
//        // We also want to replace all the (without file:// and \r\n) paths that are found in
//        QStringList fileNameListTruncated = QString(fileNameList).split("\r\n", QString::SkipEmptyParts);
//        QByteArray purePath;
//        // If more than one file copied, combine them with \n
//        if (fileNameListTruncated.length() > 1) {
//            // Don't forget to remove the last new line
//            purePath = fileNameListTruncated.join("\n").toAscii();
//        } else {
//            purePath = fileNameListTruncated.join("").toAscii();
//        }
//        QMap<QString, QByteArray>::Iterator itr = content.begin();
//        for (; itr != content.end(); itr++) {
//            QString todoRemove = itr.key();
//            QByteArray todoRemove2 = itr.value();
//            if (itr.key() == QString("text/uri-list")) {
//                // Append files!
//#ifdef Q_WS_X11
//                mMimeData->setData(itr.key(), filePaths.join("\r\n").toAscii());
//#endif
//            } else if (((QByteArray) itr.value()).contains(purePath)) {
//#ifdef Q_WS_X11
//                // Replace the path and put it into clipboard
//                QByteArray tempByteArray = itr.value();
//                tempByteArray.replace(purePath, filePaths.join("\n").toAscii());
//                mMimeData->setData(itr.key(), tempByteArray);
//#endif
//            } else {
//                mMimeData->setData(itr.key(), itr.value());
//            }
//        }
//        mClipboard->setMimeData(mMimeData);
//    } else {
//        QMap<QString, QByteArray> content;
//        dataStream >> content;

//        // traverse through it and fill the clipboard in!
//        QMap<QString, QByteArray>::Iterator itr = content.begin();
//        mIsClient = true;

//        mClipboard->clear();
//        mMimeData = new QMimeData();
//        // Qmimedata cannot be freed!
//        mMimeData->clear();
//        for (; itr != content.end(); itr++) {
//            mMimeData->setData(itr.key(),itr.value());
//        }
//        mIsClient = true;
//        mClipboard->setMimeData(mMimeData);

//        qWarning() << content;
//    }

//#ifdef Q_WS_X11
//    QString cmd = QString("notify-send \"New Clipboard\" \"New data arrived\" -i " + qApp->applicationDirPath() + "/icon.png");
//    system(cmd.toStdString().c_str());
//#endif
//#ifdef Q_WS_WIN
//    mTray->showMessage("New Clipboard", "New data arrived");
//#endif

}

void MainWindow::connected() {
    #ifdef Q_WS_X11
        QString cmd = QString("notify-send \"New Connection\" \"Connection has been set\" -i " + qApp->applicationDirPath() + "/icon.png");
        system(cmd.toStdString().c_str());
    #endif
    #ifdef Q_WS_WIN
        mTray->showMessage("New Connection", "Connection has been set");
    #endif
    setHidden(true);
}



#ifdef ZEROCONF

void MainWindow::on_connectButtonZeroconf_clicked()
{
    QList<QTreeWidgetItem *> selectedItems = ui->serverList->selectedItems();
    if (selectedItems.isEmpty())
        return;

    QTreeWidgetItem *item = selectedItems.at(0);
    QVariant variant = item->data(0, Qt::UserRole);
    bonjourResolver->resolveBonjourRecord(variant.value<BonjourRecord>());
}

void MainWindow::on_becomeServerBtnZeroconf_clicked()
{    
    on_becomeServerBtn_clicked();

    bonjourRegister = new BonjourServiceRegister(this);
    if (!bonjourRegister) {
        QMessageBox::critical(this, tr("shareClipboard Server"), tr("Unable to start the server"));
        close();
        return;
    }

    BonjourRecord record( tr("shareClipboard on %1 - port: %2").arg(QHostInfo::localHostName()).arg(mMessageTransceiver->getPort()),
                          QLatin1String("_shareClipboard._tcp"), QString() );

    bonjourRegister->registerService( record, TCP_PORT );
}


void MainWindow::updateRecords(const QList<BonjourRecord> &list)
{
    ui->serverList->clear();

    //qDebug("list %d\n", list.size());  // FixMe: it enters two times!

    foreach (BonjourRecord record, list) {
        QVariant variant;
        variant.setValue(record);
        QTreeWidgetItem *processItem = new QTreeWidgetItem( ui->serverList,
                                                            QStringList() << record.serviceName );
        processItem->setData(0, Qt::UserRole, variant);
    }

    if( ui->serverList->invisibleRootItem()->childCount() > 0 ) {
        ui->serverList->invisibleRootItem()->child(0)->setSelected(true);
        ui->connectButtonZeroconf->setEnabled(true);
        ui->becomeServerBtnZeroconf->setEnabled(false);
    }
    else {
        ui->connectButtonZeroconf->setEnabled(false);
        ui->becomeServerBtnZeroconf->setEnabled(true);
    }
}


void MainWindow::connectToServer(const QHostInfo &hostInfo, int portNumber)
{
    const QList<QHostAddress> &ipAddress = hostInfo.addresses();
    if (!ipAddress.isEmpty())
        emit connectTo(ipAddress.first().toString(), QString::number(portNumber));

//    ui->ipAddressEdit->setText(ipAddress);
//    ui->portNumberEdit->setText(portNumber);
}

void MainWindow::toggleVisibility() {
    if (isHidden()) {
        show();
    } else {
        hide();
    }
}

#endif // ZEROCONF
