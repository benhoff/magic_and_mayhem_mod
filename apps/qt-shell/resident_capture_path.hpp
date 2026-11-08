#pragma once
#include <QCryptographicHash>
#include <QDir>
#include <QFileInfo>
#include <QString>
#include <stdexcept>

// Opt-in diagnostic naming. Each launch owns a different producer channel.
inline QString residentCapturePath(const QString& channel,const QString& file,const QString& directory){
    if(directory.isEmpty())return file;
    if(!file.isEmpty())throw std::invalid_argument("Choose resident capture file or directory, not both");
    if(!QDir::isAbsolutePath(directory)||!QDir(directory).exists())
        throw std::invalid_argument("Resident capture directory must exist and be absolute");
    if(!QDir::isAbsolutePath(channel))throw std::invalid_argument("Resident capture channel must be absolute");
    const auto identity=QCryptographicHash::hash(QFileInfo(channel).absoluteFilePath().toUtf8(),QCryptographicHash::Sha256).toHex();
    return QDir(directory).filePath("worlds-"+QString::fromLatin1(identity)+".bin");
}
