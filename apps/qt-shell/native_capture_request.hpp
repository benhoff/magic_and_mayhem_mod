#pragma once
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QImageWriter>
#include <QTemporaryFile>
#include <cstdio>
#include <stdexcept>
// Worker-only file intake. Return an owned name; valid requests remain until
// the GUI has copied images and the encoder removes the request before its ACK.
inline QString readNativeCaptureRequest(const QString& path){
    QFile file(path);if(!file.open(QIODevice::ReadOnly))return {};
    const auto bytes=file.read(4097);const bool bounded=bytes.size()<=4096&&file.atEnd();file.close();
    QJsonParseError error;const auto doc=QJsonDocument::fromJson(bytes,&error);
    const auto value=doc.object().value("name");const auto name=value.toString();
    if(bounded&&error.error==QJsonParseError::NoError&&doc.isObject()&&value.isString()&&!name.isEmpty()&&name.size()<80&&!name.contains('/')&&!name.contains('\\')&&!name.contains(".."))return name;
    if(!QFile::remove(path))throw std::runtime_error("Invalid capture request cleanup failed");
    return {};
}
// Bounded diagnostic output: compression must not consume the camera helper's
// readiness deadline. Publish complete lossless pixels atomically before ACK.
// Capture readiness is a live file handshake, not a durable-storage barrier.
// Keep all writes private until flushed/closed, then replace in one rename.
template<class Write> inline bool publishNativeCaptureFile(const QString& path,Write write){
    QTemporaryFile file(path+QStringLiteral(".XXXXXX"));
    if(!file.open() || !write(file) || !file.flush())return false;
    file.close();if(file.error()!=QFileDevice::NoError)return false;
    const auto temporary=QFile::encodeName(file.fileName()),destination=QFile::encodeName(path);
    if(std::rename(temporary.constData(),destination.constData())!=0)return false;
    file.setAutoRemove(false);return true;
}
inline bool saveNativeCaptureImage(const QString& path,const QImage& image){
    if(image.isNull())return false;
    return publishNativeCaptureFile(path,[&](QIODevice& file){
        QImageWriter writer(&file,"png");writer.setCompression(0);
        return writer.write(image);
    });
}
inline bool saveNativeCaptureMetadata(const QString& path,const QJsonObject& metadata){
    const auto bytes=QJsonDocument(metadata).toJson();
    return publishNativeCaptureFile(path,[&](QIODevice& file){return file.write(bytes)==bytes.size();});
}
