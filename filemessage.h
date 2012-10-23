#ifndef FILEMESSAGE_H
#define FILEMESSAGE_H

#include <QUrl>
#include <QFile>
#include <QList>
#include <QApplication>

#include "basemessage.h"

class FileMessage:public BaseMessage
{
public:
    FileMessage();

    QByteArray serialize();
    void deserialize(const QByteArray & message);

    QList<QUrl> getFilePaths() const;
    void setFilePaths(QList<QUrl> filePaths);
    void setFilePath(QUrl filePath);

protected:
    
    QByteArray removeOccurences(const QByteArray & byteArray, char c);
    
    
    QList<QUrl> mFilePaths;
};

#endif // FILEMESSAGE_H
