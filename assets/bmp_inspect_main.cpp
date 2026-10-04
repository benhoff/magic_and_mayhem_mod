#include "bmp.hpp"
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
    if(argc!=3) throw std::runtime_error("Usage: mnm-bmp-inspect ROOT PATH.bmp");
    auto configured=AssetStore::create(argv[1]);
    if(const auto* error=std::get_if<Error>(&configured)) throw std::runtime_error(error->detail);
    auto store=std::get<AssetStore>(std::move(configured));auto opened=store.open(argv[2]);
    if(const auto* error=std::get_if<Error>(&opened)) throw std::runtime_error(error->detail);
    auto file=std::get<std::unique_ptr<AssetFile>>(std::move(opened));auto decoded=loadBmp(*file);file.reset();
    if(const auto* error=std::get_if<BmpError>(&decoded)) throw std::runtime_error(error->detail+" at byte "+std::to_string(error->offset));
    const auto& image=std::get<BmpImage>(decoded);
    std::cout<<QJsonDocument(QJsonObject{{"source_bytes",qint64(image.sourceBytes)},
        {"width",int(image.width)},{"height",int(image.height)},
        {"source_top_down",image.sourceTopDown},{"pixel_offset",qint64(image.pixelOffset)},
        {"row_stride",qint64(image.rowStride)},{"declared_image_bytes",qint64(image.declaredImageBytes)},
        {"horizontal_pixels_per_meter",image.horizontalPixelsPerMeter},
        {"vertical_pixels_per_meter",image.verticalPixelsPerMeter},
        {"colors_used",qint64(image.colorsUsed)},{"colors_important",qint64(image.colorsImportant)},
        {"rgb_sha256",hash(image.rgb)}}).toJson().constData();return 0;
} catch(const std::exception& error) {std::cerr<<error.what()<<'\n';return 2;}
