#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QClipboard>
#include <QDebug>
#include "messagetransceiver.h"
#include <QFile>
#include <QProcess>
#include <QxtGlobalShortcut>
#include <QDir>
#include <QUrl>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    mLastContent = "";
    mMimeData = new QMimeData();

    mIsClient = false;
    mMessageTransceiver = new MessageTransceiver(this, true);
    connect(this, SIGNAL(connectTo(QString,QString)), mMessageTransceiver, SLOT(connectTo(QString,QString)));
    connect(mMessageTransceiver, SIGNAL(receiveMessage(QByteArray)), this, SLOT(receiveMessage(QByteArray)));
    connect(mMessageTransceiver, SIGNAL(connected()), this, SLOT(connected()));
    connect(this, SIGNAL(sendMessage(QByteArray)), mMessageTransceiver, SLOT(sendMessage(QByteArray)));


    mClipboard = qApp->clipboard();

    mIcon = new QIcon(":/icons/icon_partners-325.png");
    if (!(QFile::exists("icon.png"))) {
        qWarning() << "The file does not exist!" ;
        QImage img(":/icons/icon_partners-325.png");
        if (img.isNull()) {
            qWarning() << "iamge could not be loaded";
        }
        img.save("icon.png");
    }

    #ifdef Q_WS_WIN
        mTray = new QSystemTrayIcon(this);
        mTray->show();
        mTray->setIcon(*mIcon);
    #endif

        mAppPath = qApp->applicationDirPath();

        QxtGlobalShortcut* shortcut = new QxtGlobalShortcut(this);
        connect(shortcut, SIGNAL(activated()), this, SLOT(dataChanged()));
        shortcut->setShortcut(QKeySequence("Ctrl+Shift+C"));

}

MainWindow::~MainWindow()
{
    delete ui;
    delete mMimeData;
}

void MainWindow::dataChanged() {
    const QMimeData *mData = mClipboard->mimeData();

    if (mData->hasImage()) {
        QImage image = mClipboard->image();
        QByteArray msg;
        QDataStream dataStream(&msg, QIODevice::WriteOnly);
        dataStream << QString("image") <<  image;
        emit sendMessage(msg);
    } else if (mData->hasFormat("text/uri-list")) {
        // This is a list of files
        // Read the files first! Send the content
        QByteArray fileNameList = mData->data("text/uri-list");
        QList<QString> list = QString(fileNameList).split(QRegExp("\r\n"), QString::SkipEmptyParts);
        QByteArray msg;
        QDataStream dataStream(&msg, QIODevice::WriteOnly);
        dataStream << "files";
        QList<QByteArray> fileList;
        for (int i = 0; i < list.length(); ++i) {
            list[i].remove(0, QString("file://").length());
            list[i] = QUrl::fromPercentEncoding(list[i].toAscii());
            QFile file(list[i]);
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
        disconnect(mMessageTransceiver, SIGNAL(receiveMessage(QByteArray)), this, SLOT(receiveMessage(QByteArray)));
        disconnect(mMessageTransceiver, SIGNAL(connected()), this, SLOT(connected()));
        disconnect(this, SIGNAL(sendMessage(QByteArray)), mMessageTransceiver, SLOT(sendMessage(QByteArray)));
    }
    delete mMessageTransceiver;
    mMessageTransceiver = NULL;
    if (ui->becomeServerBtn->text() == "Become Server") {
        mMessageTransceiver = new MessageTransceiver(this, false);
        ui->ipAddressEdit->setEnabled(false);
        ui->portNumberEdit->setEnabled(false);
        ui->becomeServerBtn->setText("Become Client");
        setHidden(true);
    #ifdef Q_WS_X11
        QString cmd = QString("notify-send \"Share Clipboard\" \"You have successfully become server\" -i " + qApp->applicationDirPath() + "/icon.png");
        system(cmd.toStdString().c_str());
    #endif
    #ifdef Q_WS_WIN
        mTray->showMessage("Share Clipboard", "You have successfully become server!");
    #endif
    } else {
        mMessageTransceiver = new MessageTransceiver(this, true);
        ui->ipAddressEdit->setEnabled(true);
        ui->portNumberEdit->setEnabled(true);
        ui->becomeServerBtn->setText("Become Server");
    }
    connect(this, SIGNAL(connectTo(QString,QString)), mMessageTransceiver, SLOT(connectTo(QString,QString)));
    connect(mMessageTransceiver, SIGNAL(receiveMessage(QByteArray)), this, SLOT(receiveMessage(QByteArray)));
    connect(mMessageTransceiver, SIGNAL(connected()), this, SLOT(connected()));
    connect(this, SIGNAL(sendMessage(QByteArray)), mMessageTransceiver, SLOT(sendMessage(QByteArray)));
}

void MainWindow::receiveMessage(QByteArray msg) {
    QDataStream dataStream(&msg, QIODevice::ReadOnly);

    // Read the content
    QString type;
    dataStream >> type;

    if (type == QString("image")) {
        QImage img;
        dataStream >> img;
        mIsClient = true;
        mClipboard->clear();
        mIsClient = true;
        mClipboard->setImage(img);
    } else if(type == QString("files")) {
        QList<QByteArray> fileContents;
        QMap<QString, QByteArray> content;
        dataStream >> fileContents >> content;
        QByteArray fileNameList = content["text/uri-list"];
        //Check if linux
#ifdef Q_WS_X11
        QRegExp sep("\r\n");
        QList<QString> fileNameListSplitted = QString(QUrl::fromPercentEncoding(fileNameList)).split(sep, QString::SkipEmptyParts);
        QList<QString> fileNames;
        for (int i = 0; i < fileNameListSplitted.length(); ++i) {
            QList<QString> pathBits = fileNameListSplitted[i].split('/');
            fileNames.append(pathBits[pathBits.length()-1]);
        }
#endif
#ifdef Q_WS_WIN
        return ;
#endif
        // Create a file saver
        QDir dir;
        if (!dir.exists(".temp")) {
            dir.mkdir(".temp");
        }

        // Have a list of file paths
        QStringList filePaths;
        for (int i = 0; i < fileContents.length(); i++) {
            QFile file(qApp->applicationDirPath() + "/.temp/" + QString(fileNames[i]));
            filePaths.append(QUrl::toPercentEncoding("file://" + qApp->applicationDirPath() + "/.temp/" + QString(fileNames[i])));
            file.write(fileContents[i]);
            file.close();
        }
        mMimeData = new QMimeData();
        // traverse through it and fill the clipboard in!
        QMap<QString, QByteArray>::Iterator itr = content.begin();
        for (; itr != content.end(); itr++) {
            if (itr.key() == QString("text/uri-list")) {
                // Append files!
#ifdef Q_WS_X11
                mMimeData->setData(itr.key(), filePaths.join("\r\n").toAscii());
#endif
            } else {
                mMimeData->setData(itr.key(), itr.value());
            }
        }
        mClipboard->setMimeData(mMimeData);
    } else {
        QMap<QString, QByteArray> content;
        dataStream >> content;

        // traverse through it and fill the clipboard in!
        QMap<QString, QByteArray>::Iterator itr = content.begin();
        mIsClient = true;

        mClipboard->clear();
        mMimeData = new QMimeData();
        // Qmimedata cannot be freed!
        mMimeData->clear();
        for (; itr != content.end(); itr++) {
            mMimeData->setData(itr.key(),itr.value());
        }
        mIsClient = true;
        mClipboard->setMimeData(mMimeData);

        qWarning() << content;
    }

#ifdef Q_WS_X11
    QString cmd = QString("notify-send \"New Clipboard\" \"New data arrived\" -i " + qApp->applicationDirPath() + "/icon.png");
    system(cmd.toStdString().c_str());
#endif
#ifdef Q_WS_WIN
    mTray->showMessage("New Clipboard", "New data arrived");
#endif

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

