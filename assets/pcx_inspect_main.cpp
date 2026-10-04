#include "pcx.hpp"
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <iostream>
#include <stdexcept>
using namespace mnm::assets;
namespace {
QString hash(const std::vector<std::uint8_t>& bytes) {
    return QString::fromLatin1(QCryptographicHash::hash(QByteArray(reinterpret_cast<const char*>(bytes.data()),static_cast<qsizetype>(bytes.size())),QCryptographicHash::Sha256).toHex());
}
}
int main(int argc,char** argv) try {
    QCoreApplication app(argc,argv);
    if(argc!=3) throw std::runtime_error("Usage: mnm-pcx-inspect ROOT PATH.pcx");
    auto configured=AssetStore::create(argv[1]);
    if(const auto* error=std::get_if<Error>(&configured)) throw std::runtime_error(error->detail);
    auto store=std::get<AssetStore>(std::move(configured));auto opened=store.open(argv[2]);
    if(const auto* error=std::get_if<Error>(&opened)) throw std::runtime_error(error->detail);
    auto file=std::get<std::unique_ptr<AssetFile>>(std::move(opened));auto decoded=loadPcx(*file);file.reset();
    if(const auto* error=std::get_if<PcxError>(&decoded)) throw std::runtime_error(error->detail+" at byte "+std::to_string(error->offset));
    const auto& image=std::get<PcxImage>(decoded);
    std::vector<std::uint8_t> palette;
    for(const auto& rgb:image.palette) palette.insert(palette.end(),rgb.begin(),rgb.end());
    std::cout<<QJsonDocument(QJsonObject{{"source_bytes",qint64(image.sourceBytes)},
        {"width",int(image.width)},{"height",int(image.height)},
        {"origin_x",int(image.originX)},{"origin_y",int(image.originY)},
        {"horizontal_dpi",int(image.horizontalDpi)},{"vertical_dpi",int(image.verticalDpi)},
        {"bytes_per_line",int(image.bytesPerLine)},{"palette_info",int(image.paletteInfo)},
        {"palette_sha256",hash(palette)},{"indices_sha256",hash(image.indices)}}).toJson().constData();return 0;
} catch(const std::exception& error) {std::cerr<<error.what()<<'\n';return 2;}
