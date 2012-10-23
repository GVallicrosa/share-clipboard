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
#include <QHostInfo>

#include "avahi/bonjourrecord.h"

class BonjourServiceRegister;
class BonjourServiceBrowser;
class BonjourServiceResolver;

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

    void on_connectButtonZeroconf_clicked();

    void on_becomeServerBtnZeroconf_clicked();

    void updateRecords(const QList<BonjourRecord> &list);
    void connectToServer(const QHostInfo & hostInfo, int portNumber);

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

    BonjourServiceRegister *bonjourRegister;
    BonjourServiceBrowser  *bonjourBrowser;
    BonjourServiceResolver *bonjourResolver;
};

#endif // MAINWINDOW_H
