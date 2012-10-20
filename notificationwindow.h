#ifndef NOTIFICATIONWINDOW_H
#define NOTIFICATIONWINDOW_H

#include <QObject>
#include <QUrl>
#include <QDeclarativeView>
#include <QVariant>
#include <QTimer>
#include <QApplication>
#include <QDesktopWidget>

class NotificationWindow : public QObject
{
    Q_OBJECT
public:
    explicit NotificationWindow(QObject *parent = 0);
    
signals:
    
public slots:
    void destroyMessage();
    void showMessage(QString title, QString description, quint32 msecs=2000);
    
private:
    QDeclarativeView *mDecView;
    QTimer *mTimer;
    
};

#endif // NOTIFICATIONWINDOW_H
