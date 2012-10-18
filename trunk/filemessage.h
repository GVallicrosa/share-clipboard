#ifndef FILEMESSAGE_H
#define FILEMESSAGE_H

#include <QUrl>
#include <QFile>
#include <QList>

#include "basemessage.h"

class FileMessage:public BaseMessage
{
public:
    FileMessage();

    QByteArray serialize();
    void deserialize(const QByteArray & message);

    QList<QUrl> getFilePaths();
    void setFilePaths(QList<QUrl> filePaths);
    void setFilePath(QUrl filePath);


protected:
    QList<QUrl> mFilePaths;
    QFile mFile;
};

#endif // FILEMESSAGE_H
