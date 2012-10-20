#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMimeData>
#include "messagetransceiver.h"
#include "clipboardclient.h"
#include "protocolhandler.h"
#include "notificationwindow.h"
#include <QSystemTrayIcon>
#include <QKeyEvent>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

signals:
    void connectTo(QString ipAddress, QString portNumber);
    void sendMessage(QByteArray msg);
    void showMessage(QString title, QString description, quint32 msecs);

public slots:
    void dataChanged();
    void receiveMessage(QByteArray msg);
    void connected();
    
private slots:

    void on_connectButton_clicked();

    void on_becomeServerBtn_clicked();

private:
    Ui::MainWindow *ui;
    QClipboard *mClipboard;
    MessageTransceiver *mMessageTransceiver;
    ProtocolHandler *mProtocolHandler;
    ClipboardClient *mClipboardClient;
    bool mIsClient;
    QString mLastContent;
    QMimeData *mMimeData;
    QMimeData *mMimeDumper;

    QSystemTrayIcon *mTray;
    QIcon *mIcon;
    QString mAppPath;
    NotificationWindow *mNotify;

};

#endif // MAINWINDOW_H
