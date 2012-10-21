#include "filemessage.h"

FileMessage::FileMessage()
{
    setType(MESSAGE_FILE);
}


QByteArray FileMessage::serialize() {

    // Before serializing, read the file.
    if (mFilePaths.isEmpty()) {
        qWarning() << "File url is empty. Are you sure you should serialize it?";
    }
    
    // Reading the files
    QList<QByteArray> fileContents;
    int length = mFilePaths.length();
    for (int i = 0; i < length; i++) {
        QString filePath = mFilePaths[i].toString();
        if (filePath.contains("file://")) {
            filePath = filePath.remove("file://");
        }
        QFile file(filePath);
        if (!file.open(QFile::ReadOnly)) {
            qWarning() << "File could not be sent!";
        } else {
            fileContents.append(file.readAll());
        }
        file.close();
    }
    
    // We are now ready to serialize all the files we have!
    QByteArray message = BaseMessage::serialize();
    QDataStream dataStream(&message, QIODevice::Append);
    dataStream << fileContents << mMimeContent;
    
    return message;
}

void FileMessage::deserialize(const QByteArray &message) {
    QList<QByteArray> fileContents;
    QByteArray msg(message);
    QDataStream dataStream(&msg, QIODevice::ReadOnly);
    dataStream.skipRawData(sizeof(mType));
    dataStream >> fileContents >> mMimeContent;

    // Now we have the file(s), so go ahead, save it in the .temp folder

    // Check if .temp exists
    QDir dir = QDir();
    if (!dir.exists(TEMP_FOLDER)) {
        dir.mkdir(TEMP_FOLDER);
    } else {
        // Clean inside the folder
        dir.cd(TEMP_FOLDER);
        QFileInfoList files = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files);
        foreach (QFileInfo filename, files) {
            dir.remove(filename.fileName());
        }
    }

    // Get the file name(s)
    QMimeData *mimeData = new QMimeData();
    QMap<QString, QByteArray>::iterator itr;
    for (itr = mMimeContent.begin(); itr != mMimeContent.end(); itr++) {
        mimeData->setData(itr.key(),itr.value());
    }
    
    // Here we have the paths
    QList<QUrl> filePathList = mimeData->urls();
    QStringList fileNames;
    int length = filePathList.length();
    for (int i = 0; i < length; i++) {
        QString filePath = filePathList[i].toString();
        QStringList filePathParts = filePath.split("/",QString::SkipEmptyParts);
        fileNames.append(filePathParts[filePathParts.length()-1]);
    }
    
    // Now we have the file names, we can save the files
    length = fileContents.length();
    for (int i = 0; i < length; i++) {
        QFile file(TEMP_FOLDER + "/" + fileNames[i]);
        file.open(QFile::WriteOnly);
        file.write(fileContents[i]);
        file.close();
#ifdef Q_WS_X11
        mFilePaths.append(QUrl("file://" + qApp->applicationDirPath() + "/" + TEMP_FOLDER + "/" + fileNames[i]));
#else
        mFilePaths.append(qApp->applicationDirPath() + "/" + QUrl(TEMP_FOLDER + "/" + fileNames[i]));
#endif
    }
    
    // First we go through all the data in the mimecontent
    // and simplify it
    for (itr = mMimeContent.begin(); itr != mMimeContent.end(); itr++) {
        // Remove \0 and \r from the values
        itr.value() = itr.value().replace("\0","");
        itr.value() = itr.value().replace("\r","");
        // And now replace the old paths with the new ones
        for (int i = 0; i < filePathList.length(); i++) {
            itr.value() = itr.value().replace(filePathList[i].toString().toAscii(), mFilePaths[i].toString().toAscii());
        }
    }
    
}

QList<QUrl> FileMessage::getFilePaths() const
{
    return mFilePaths;
}


void FileMessage::setFilePath(QUrl filePath) {
    mFilePaths.append(QUrl(filePath));
}

void FileMessage::setFilePaths(QList<QUrl> filePaths) {
    mFilePaths = QList<QUrl>(filePaths);
}

