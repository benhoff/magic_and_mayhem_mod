#pragma once
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
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
