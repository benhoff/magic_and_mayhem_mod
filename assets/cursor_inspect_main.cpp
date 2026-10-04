#include "cursor.hpp"
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
    if(argc!=3) throw std::runtime_error("Usage: mnm-cursor-inspect ROOT PATH.cur");
    auto configured=AssetStore::create(argv[1]);
    if(const auto* error=std::get_if<Error>(&configured)) throw std::runtime_error(error->detail);
    auto store=std::get<AssetStore>(std::move(configured));auto opened=store.open(argv[2]);
    if(const auto* error=std::get_if<Error>(&opened)) throw std::runtime_error(error->detail);
    auto file=std::get<std::unique_ptr<AssetFile>>(std::move(opened));auto decoded=loadCursor(*file);file.reset();
    if(const auto* error=std::get_if<CursorError>(&decoded)) throw std::runtime_error(error->detail+" at byte "+std::to_string(error->offset));
    const auto& cursor=std::get<CursorAsset>(decoded);QJsonArray images;
    for(const auto& image:cursor.images) {
        std::vector<std::uint8_t> paletteBytes;std::uint32_t altered=0;
        for(const auto& color:image.palette) paletteBytes.insert(paletteBytes.end(),color.begin(),color.end());
        for(std::size_t p=0;p<image.xorIndices.size();++p) {
            const auto& color=image.palette[image.xorIndices[p]];
            if(image.andMask[p] && (color[0] || color[1] || color[2])) ++altered;
        }
        images.append(QJsonObject{{"width",int(image.width)},{"height",int(image.height)},
            {"hotspot_x",int(image.hotspotX)},{"hotspot_y",int(image.hotspotY)},{"bit_depth",int(image.bitDepth)},
            {"directory_color_count",int(image.directoryColorCount)},{"palette_entries",qint64(image.palette.size())},
            {"source_offset",qint64(image.sourceOffset)},{"encoded_bytes",qint64(image.encodedSize)},
            {"palette_sha256",hash(paletteBytes)},{"xor_indices_sha256",hash(image.xorIndices)},
            {"and_mask_sha256",hash(image.andMask)},{"and_one_nonzero_xor_pixels",int(altered)}});
    }
    std::cout<<QJsonDocument(QJsonObject{{"source_bytes",qint64(cursor.sourceBytes)},{"images",images}}).toJson().constData();return 0;
} catch(const std::exception& error) {std::cerr<<error.what()<<'\n';return 2;}
