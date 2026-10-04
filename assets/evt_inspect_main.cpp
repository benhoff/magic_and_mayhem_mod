#include "evt.hpp"
#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <iostream>
#include <stdexcept>
using namespace mnm::assets;
int main(int argc,char** argv) try {
    QCoreApplication app(argc,argv);
    if(argc!=3) throw std::runtime_error("Usage: mnm-evt-inspect ROOT PATH.evt");
    auto configured=AssetStore::create(argv[1]);
    if(const auto* error=std::get_if<Error>(&configured)) throw std::runtime_error(error->detail);
    auto store=std::get<AssetStore>(std::move(configured));auto opened=store.open(argv[2]);
    if(const auto* error=std::get_if<Error>(&opened)) throw std::runtime_error(error->detail);
    auto file=std::get<std::unique_ptr<AssetFile>>(std::move(opened));auto decoded=loadEvt(*file);file.reset();
    if(const auto* error=std::get_if<EvtError>(&decoded)) throw std::runtime_error(error->detail+" at byte "+std::to_string(error->offset));
    const auto& asset=std::get<EvtAsset>(decoded);QJsonArray areas;
    for(const auto& item:asset.areas) {
        QJsonArray first,second;
        for(auto word:item.first) first.append(word);
        for(auto word:item.second) second.append(word);
        const auto name=evtAreaName(item);
        const QByteArray raw(reinterpret_cast<const char*>(item.nameBytes.data()),48);
        areas.append(QJsonObject{{"first",first},{"second",second},
            {"name",QString::fromLatin1(name.data(),static_cast<qsizetype>(name.size()))},
            {"name_bytes_hex",QString::fromLatin1(raw.toHex())}});
    }
    std::cout<<QJsonDocument(QJsonObject{{"source_bytes",qint64(asset.sourceBytes)},
        {"header_size_word",qint64(asset.headerSizeWord)},{"version",qint64(asset.version)},
        {"record_count",qint64(asset.areas.size())},{"areas",areas}}).toJson().constData();return 0;
} catch(const std::exception& error) {std::cerr<<error.what()<<'\n';return 2;}
