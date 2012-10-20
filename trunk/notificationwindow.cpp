#include "notificationwindow.h"

NotificationWindow::NotificationWindow(QObject *parent) :
    QObject(parent)
{
    mDecView = 0;
    mTimer = 0;
}

void NotificationWindow::showMessage(QString title, QString description, quint32 msecs) {
    // Delete existing view
    if (mDecView) {
        delete mDecView;
        mDecView = 0;
    }
    
    if (mTimer) {
        disconnect(mTimer, SIGNAL(timeout()), this, SLOT(destroyMessage()));
        mTimer->stop();
        killTimer(mTimer->timerId());
        delete mTimer;
        mTimer = 0;
    }
    
    mDecView = new QDeclarativeView;
    mDecView->setSource(QUrl("qrc:/NotificationQml.qml"));
    mDecView->setWindowFlags(Qt::FramelessWindowHint);
    mDecView->setStyleSheet("background:transparent;");
    mDecView->setAttribute(Qt::WA_TranslucentBackground);
    QObject *obj = (QObject *)mDecView->rootObject();
    QObject *titleObj = obj->findChild<QObject *>("title");
    titleObj->setProperty("text", title);
    QObject *descriptionObj = obj->findChild<QObject *>("description");
    descriptionObj->setProperty("text", description);
    mDecView->setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
    
    QRect qRect(QApplication::desktop()->screenGeometry());
    int iXpos=qRect.width()-mDecView->width()-20;
    int iYpos=40;
    mDecView->move(iXpos,iYpos);
    
    mDecView->show();
    mTimer = new QTimer(this);
    mTimer->setInterval(msecs);
    mTimer->setSingleShot(true);
    connect(mTimer, SIGNAL(timeout()), this, SLOT(destroyMessage()));
    mTimer->start();
//    mTimer->singleShot(msecs, this, SLOT(destroyMessage()));
}

void NotificationWindow::destroyMessage() {
    if(mDecView) {
        mDecView->setHidden(true);
        delete mDecView;
        mDecView = 0;
    }
}
