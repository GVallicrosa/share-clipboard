#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMimeData>
#include "messagetransceiver.h"
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
    bool mIsClient;
    QString mLastContent;
    QMimeData *mMimeData;
    QMimeData *mMimeDumper;

    QSystemTrayIcon *mTray;
    QIcon *mIcon;
    QString mAppPath;

};

#endif // MAINWINDOW_H
