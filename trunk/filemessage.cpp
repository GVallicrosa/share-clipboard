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


//    mFile = new QFile(mFilePath);
//    if (!mFile.open(QFile::ReadOnly)) {
//        qWarning() << "File could not be read. It does not exist.";
//    }
//    QByteArray fileContent = mFile.readAll();
//    mFile.close();

    QByteArray message = BaseMessage::serialize();
    QDataStream dataStream(&message, QIODevice::WriteOnly);
    // Serialize the mime content
//    dataStream << fileContent << mMimeContent;
    // After updating the message, go back to the beginning and update the size
    mLength = (quint64)(message.length() - (quint64) sizeof(mLength));
    dataStream.device()->seek(0);
    dataStream << mLength;
    return message;
}

void FileMessage::deserialize(const QByteArray &message) {
    QByteArray fileContent;
    QByteArray msg(message);
    QDataStream dataStream(&msg, QIODevice::ReadOnly);
    dataStream >> fileContent >> mMimeContent;

    // Now we have the file(s), so go ahead, save it in the .temp folder

    // Check if .temp exists
    QDir dir = QDir();
    if (!dir.exists(TEMP_FOLDER)) {
        dir.mkdir(TEMP_FOLDER);
    }

    // Get the file name(s)


}

QList<QUrl> FileMessage::getFilePaths()
{
    return mFilePaths;
}


void FileMessage::setFilePath(QUrl filePath) {
    mFilePaths.append(QUrl(filePath));
}

void FileMessage::setFilePaths(QList<QUrl> filePaths) {
    mFilePaths = QList<QUrl>(filePaths);
}
