#include "sprite.hpp"
#include <QCommandLineParser>
#include <QCryptographicHash>
#include <QFile>
#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <iostream>
#include <limits>
#include <stdexcept>

namespace {
QString hash(const QByteArray& bytes){return QCryptographicHash::hash(bytes,QCryptographicHash::Sha256).toHex();}
void writeNew(const QString& path,const QByteArray& bytes){
    QFile file(path);
    if(!file.open(QIODevice::WriteOnly|QIODevice::NewOnly))throw std::runtime_error(file.errorString().toStdString());
    if(file.write(bytes)!=bytes.size() || !file.flush()){file.remove();throw std::runtime_error("Failed to write native output");}
}
}
int main(int argc,char** argv)try{
    QGuiApplication app(argc,argv);
    QCommandLineParser parser;parser.setApplicationDescription("Offline SPR preview: owned RGB565/mask upload, unshaded embedded palette");
    parser.addHelpOption();
    parser.addOption({"root","Installed asset root","directory"});
    parser.addOption({"prefix","Explicit Windows installation alias (repeatable)","path"});
    parser.addOption({"path","One SPR request","path"});
    parser.addOption({"frame","Frame index (default 0)","index","0"});
    parser.addOption({"output","New little-endian RGB565 destination file","file"});
    parser.addOption({"preview","New PNG presentation file","file"});
    parser.addOption({"manifest","JSON array of {path,frame,output,preview}; maximum 128 entries","file"});
    parser.process(app);
    if(!parser.isSet("root") || parser.isSet("path")==parser.isSet("manifest"))
        throw std::runtime_error("Specify --root and exactly one of --path or --manifest");
    if(parser.isSet("manifest") && (parser.isSet("frame") || parser.isSet("output") || parser.isSet("preview")))
        throw std::runtime_error("Manifest contains its own frame/output/preview selections");
    QJsonArray entries;
    if(parser.isSet("manifest")){
        QFile input(parser.value("manifest"));
        if(!input.open(QIODevice::ReadOnly) || input.size()>1024*1024)throw std::runtime_error("Cannot read manifest within 1 MiB limit");
        QJsonParseError error;const auto document=QJsonDocument::fromJson(input.read(1024*1024+1),&error);
        if(error.error!=QJsonParseError::NoError || !document.isArray())throw std::runtime_error("Expected manifest array");
        entries=document.array();
    }else{
        bool ok=false;const auto index=parser.value("frame").toUInt(&ok);
        if(!ok || !parser.isSet("output") || !parser.isSet("preview"))throw std::runtime_error("Specify frame index, --output and --preview");
        entries.append(QJsonObject{{"path",parser.value("path")},{"frame",qint64(index)},
                                  {"output",parser.value("output")},{"preview",parser.value("preview")}});
    }
    if(entries.empty() || entries.size()>128)throw std::runtime_error("Manifest must contain 1..128 entries");
    std::vector<std::string> prefixes;for(const auto& value:parser.values("prefix"))prefixes.push_back(value.toStdString());
    auto configured=mnm::assets::AssetStore::create(parser.value("root").toStdString(),prefixes);
    if(const auto* error=std::get_if<mnm::assets::Error>(&configured))throw std::runtime_error(error->detail);
    auto store=std::get<mnm::assets::AssetStore>(std::move(configured));
    mnm::render::GlBlitter renderer;QJsonArray results;std::uint64_t outputBytes=0;
    for(const auto& value:entries){
        if(!value.isObject())throw std::runtime_error("Expected manifest entry object");
        const auto entry=value.toObject();
        for(const auto* key:{"path","output","preview"})
            if(!entry[key].isString() || entry[key].toString().isEmpty())throw std::runtime_error("Manifest requires path/output/preview strings");
        const auto number=entry["frame"].toDouble(-1);
        if(number<0 || number>UINT32_MAX || number!=double(std::uint32_t(number)))throw std::runtime_error("Invalid manifest frame index");
        const auto index=std::uint32_t(number);const auto path=entry["path"].toString();
        auto opened=store.open(path.toStdString());
        if(const auto* error=std::get_if<mnm::assets::Error>(&opened))throw std::runtime_error(error->detail);
        auto file=std::get<std::unique_ptr<mnm::assets::AssetFile>>(std::move(opened));
        auto loaded=mnm::assets::loadSprite(*file);file.reset();
        if(const auto* error=std::get_if<mnm::assets::SpriteError>(&loaded))throw std::runtime_error(error->detail);
        auto sprite=std::get<mnm::assets::Sprite>(std::move(loaded));
        if(index>=sprite.frames.size())throw std::runtime_error("Frame index out of range");
        const auto& f=sprite.frames[index];
        const auto ax=std::int64_t(f.originX)+1,ay=std::int64_t(f.originY)+1;
        if(ax>std::numeric_limits<int>::max() || ay>std::numeric_limits<int>::max() || f.width>2046 || f.height>2046)
            throw std::runtime_error("Frame cannot fit preview border/anchor");
        const int w=int(f.width)+2,h=int(f.height)+2;
        outputBytes+=std::uint64_t(w)*h*6;
        if(outputBytes>32*1024*1024)throw std::runtime_error("Preview output exceeds 32 MiB pixel budget");
        const auto originX=f.originX,originY=f.originY;
        mnm::render::UploadedSpriteFrame uploaded(renderer,sprite,index);
        sprite={}; // GPU upload remains valid without CPU decoded buffers.
        const auto destination=renderer.create({w,h,std::vector<std::uint32_t>(std::size_t(w)*h,0x1234)},mnm::render::spriteFormat);
        const auto before=renderer.stats();uploaded.draw(destination,int(ax),int(ay));const auto after=renderer.stats();
        if(before.uploads!=after.uploads || before.nativeReadbacks!=after.nativeReadbacks)throw std::runtime_error("Sprite draw uploaded/read back native pixels");
        const auto native=renderer.read(destination);const auto image=renderer.present(destination);renderer.destroy(destination);
        QByteArray bytes;bytes.reserve(w*h*2);for(auto pixel:native.pixels){bytes.append(char(pixel&255));bytes.append(char(pixel>>8));}
        writeNew(entry["output"].toString(),bytes);
        QFile png(entry["preview"].toString());
        if(!png.open(QIODevice::WriteOnly|QIODevice::NewOnly))throw std::runtime_error(png.errorString().toStdString());
        if(!image.save(&png,"PNG") || !png.flush()){png.remove();throw std::runtime_error("Failed to write preview");}
        results.append(QJsonObject{{"path",path},{"frame",qint64(index)},{"width",w},{"height",h},
            {"origin_x",originX},{"origin_y",originY},{"anchor_x",qint64(ax)},{"anchor_y",qint64(ay)},
            {"empty",uploaded.empty()},{"draw_copies",qint64(after.copies-before.copies)},
            {"output_sha256",hash(bytes)},
            {"presentation_rgba_sha256",hash(QByteArray(reinterpret_cast<const char*>(image.constBits()),image.sizeInBytes()))}});
    }
    const auto driver=renderer.driver();const auto stats=renderer.stats();
    if(stats.surfaces || stats.pixels)throw std::runtime_error("Sprite preview resources leaked");
    std::cout<<QJsonDocument(QJsonObject{{"rendered",true},{"files",results},{"vendor",QString::fromStdString(driver.vendor)},
        {"renderer",QString::fromStdString(driver.renderer)},{"version",QString::fromStdString(driver.version)},
        {"uploads",qint64(stats.uploads)},{"copies",qint64(stats.copies)},{"surfaces",int(stats.surfaces)}}).toJson().constData();
    return 0;
}catch(const std::exception& error){
    std::cerr<<error.what()<<'\n';return 2;
}
