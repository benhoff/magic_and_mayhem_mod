#include "jpeg.hpp"
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <iostream>
#include <stdexcept>
using namespace mnm::assets;
int main(int argc,char** argv) try {
    QCoreApplication app(argc,argv);
    if(argc!=4)throw std::runtime_error("Usage: mnm-jpeg-word-cache ROOT INPUT OUTPUT");
    auto configured=AssetStore::create(argv[1]);
    if(const auto* error=std::get_if<Error>(&configured))throw std::runtime_error(error->detail);
    auto store=std::get<AssetStore>(std::move(configured));auto opened=store.open(argv[2]);
    if(const auto* error=std::get_if<Error>(&opened))throw std::runtime_error(error->detail);
    auto file=std::get<std::unique_ptr<AssetFile>>(std::move(opened));auto decoded=loadJpeg(*file);file.reset();
    if(const auto* error=std::get_if<JpegError>(&decoded))throw std::runtime_error(error->detail);
    const auto& image=std::get<JpegImage>(decoded);
    QFile output(QString::fromLocal8Bit(argv[3]));
    if(!output.open(QIODevice::WriteOnly|QIODevice::NewOnly))throw std::runtime_error("Cache output exists or cannot be created");
    if(output.write(reinterpret_cast<const char*>(image.rgb.data()),qsizetype(image.rgb.size()))!=qsizetype(image.rgb.size()) || !output.flush())
        throw std::runtime_error("Incomplete RGB output");
    const auto hash=QCryptographicHash::hash(QByteArray::fromRawData(reinterpret_cast<const char*>(image.rgb.data()),qsizetype(image.rgb.size())),QCryptographicHash::Sha256).toHex();
    std::cout<<QJsonDocument(QJsonObject{{"width",int(image.width)},{"height",int(image.height)},
        {"source_bytes",qint64(image.sourceBytes)},{"rgb_sha256",QString::fromLatin1(hash)}}).toJson().constData();
    return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 2;}
