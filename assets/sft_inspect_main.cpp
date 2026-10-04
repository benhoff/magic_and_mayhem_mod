#include "sft.hpp"
#include <QCoreApplication>
#include <QImage>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <algorithm>
#include <iostream>
#include <stdexcept>
using namespace mnm::assets;
QString hex(const std::vector<std::uint8_t>& bytes) {
    return QString::fromLatin1(QByteArray(reinterpret_cast<const char*>(bytes.data()),static_cast<qsizetype>(bytes.size())).toHex());
}
void atlas(const Sprite& glyphs,const char* path) {
    std::uint32_t width=1,height=1;
    for(const auto& frame:glyphs.frames) {width=std::max(width,frame.width);height=std::max(height,frame.height);}
    const std::uint64_t columns=16,rows=std::max<std::uint64_t>(1,(glyphs.frames.size()+15)/16);
    const auto w=columns*(width+2),h=rows*(height+2);
    if(w*h>16*1024*1024) throw std::runtime_error("Atlas exceeds 16M pixel budget");
    QImage image(static_cast<int>(w),static_cast<int>(h),QImage::Format_RGBA8888);
    if(image.isNull()) throw std::runtime_error("Atlas allocation failed");
    image.fill(Qt::transparent);
    for(std::size_t i=0;i<glyphs.frames.size();++i) {
        const auto& f=glyphs.frames[i];
        for(std::uint32_t y=0;y<f.height;++y) for(std::uint32_t x=0;x<f.width;++x) {
            const auto at=std::size_t(y)*f.width+x;if(!f.opaqueMask[at]) continue;
            QColor colour;
            if(f.paletteIndex) {
                const auto rgb=glyphs.palettes[*f.paletteIndex][std::get<std::vector<std::uint8_t>>(f.pixels)[at]];
                colour=QColor(rgb.red,rgb.green,rgb.blue);
            } else {
                const auto value=std::get<std::vector<std::uint16_t>>(f.pixels)[at];
                colour=QColor(((value>>11)*255+15)/31,(((value>>5)&63)*255+31)/63,((value&31)*255+15)/31);
            }
            image.setPixelColor(static_cast<int>((i%16)*(width+2)+x+1),static_cast<int>((i/16)*(height+2)+y+1),colour);
        }
    }
    if(!image.save(QString::fromLocal8Bit(path),"PNG")) throw std::runtime_error("Atlas write failed");
}
int main(int argc,char** argv) try {
    QCoreApplication app(argc,argv);
    if(argc!=3 && argc!=4) throw std::runtime_error("Usage: mnm-sft-inspect ROOT PATH.sft [ATLAS.png]");
    auto configured=AssetStore::create(argv[1]);
    if(const auto* error=std::get_if<Error>(&configured)) throw std::runtime_error(error->detail);
    auto store=std::get<AssetStore>(std::move(configured));auto opened=store.open(argv[2]);
    if(const auto* error=std::get_if<Error>(&opened)) throw std::runtime_error(error->detail);
    auto file=std::get<std::unique_ptr<AssetFile>>(std::move(opened));auto decoded=loadSft(*file);file.reset();
    if(const auto* error=std::get_if<SftError>(&decoded)) throw std::runtime_error(error->detail+" at byte "+std::to_string(error->offset));
    const auto& font=std::get<SftFont>(decoded);const auto& glyphs=font.glyphs;QJsonArray metrics,palettes,frames;
    for(const auto& metric:font.rowMetrics) metrics.append(QJsonArray{metric.leading,metric.trailing});
    for(const auto& palette:glyphs.palettes) {
        std::vector<std::uint8_t> bytes;for(const auto& rgb:palette) {bytes.push_back(rgb.red);bytes.push_back(rgb.green);bytes.push_back(rgb.blue);}
        palettes.append(hex(bytes));
    }
    for(std::size_t i=0;i<glyphs.frames.size();++i) {
        const auto& f=glyphs.frames[i];std::vector<std::uint8_t> pixels;
        if(f.paletteIndex) pixels=std::get<std::vector<std::uint8_t>>(f.pixels);
        else for(auto value:std::get<std::vector<std::uint16_t>>(f.pixels)) {pixels.push_back(value&255);pixels.push_back(value>>8);}
        const std::vector<std::uint8_t> name(f.name.begin(),f.name.end());
        frames.append(QJsonObject{{"byte",i<223 ? QJsonValue(qint64(i+33)) : QJsonValue(QJsonValue::Null)},
            {"width",qint64(f.width)},{"height",qint64(f.height)},{"origin_x",f.originX},{"origin_y",f.originY},
            {"name_hex",hex(name)},{"source_offset",qint64(f.sourceOffset)},{"encoded_size",qint64(f.encodedSize)},
            {"palette_index",f.paletteIndex ? QJsonValue(qint64(*f.paletteIndex)) : QJsonValue(QJsonValue::Null)},
            {"auxiliary_offsets",QJsonArray{qint64(f.auxiliaryOffsets[0]),qint64(f.auxiliaryOffsets[1])}},
            {"auxiliary_hex",QJsonArray{hex(f.auxiliaryData[0]),hex(f.auxiliaryData[1])}},
            {"pixels_hex",hex(pixels)},{"mask_hex",hex(f.opaqueMask)}});
    }
    if(argc==4) atlas(glyphs,argv[3]);
    std::cout<<QJsonDocument(QJsonObject{{"source_bytes",qint64(glyphs.sourceBytes)},{"version",qint64(font.version)},
        {"row_count",qint64(font.rowCount)},{"ascent",font.ascent},{"descent",font.descent},
        {"header_word_28",qint64(font.headerWord28)},{"palette_flag",qint64(font.paletteFlag)},
        {"metric_glyph_count",qint64(font.metricGlyphCount)},{"row_metrics",metrics},{"palettes_hex",palettes},
        {"glyph_count",qint64(glyphs.frames.size())},{"glyphs",frames}}).toJson().constData();return 0;
} catch(const std::exception& error) {std::cerr<<error.what()<<'\n';return 2;}
