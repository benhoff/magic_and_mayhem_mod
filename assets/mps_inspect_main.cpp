#include "mps.hpp"
#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <iostream>
#include <stdexcept>
using namespace mnm::assets;
int main(int argc,char** argv) try {
    QCoreApplication app(argc,argv);
    if(argc!=3) throw std::runtime_error("Usage: mnm-mps-inspect ROOT PATH.mps");
    auto configured=AssetStore::create(argv[1]);
    if(const auto* error=std::get_if<Error>(&configured)) throw std::runtime_error(error->detail);
    auto store=std::get<AssetStore>(std::move(configured));auto opened=store.open(argv[2]);
    if(const auto* error=std::get_if<Error>(&opened)) throw std::runtime_error(error->detail);
    auto file=std::get<std::unique_ptr<AssetFile>>(std::move(opened));auto decoded=loadMps(*file);file.reset();
    if(const auto* error=std::get_if<MpsError>(&decoded)) throw std::runtime_error(error->detail+" at byte "+std::to_string(error->offset));
    const auto& asset=std::get<MpsAsset>(decoded);QJsonArray placements;
    for(const auto& item:asset.placements) {
        QJsonArray position,parameters;
        for(auto word:item.position) position.append(word);
        for(auto word:item.parameters) parameters.append(word);
        placements.append(QJsonObject{{"position",position},{"kind",static_cast<int>(item.kind)},
            {"kind_name",QString::fromLatin1(mpsKindName(item.kind))},{"parameters",parameters}});
    }
    std::cout<<QJsonDocument(QJsonObject{{"source_bytes",qint64(asset.sourceBytes)},
        {"header_size_word",qint64(asset.headerSizeWord)},{"version",qint64(asset.version)},
        {"record_count",qint64(asset.placements.size())},{"placements",placements}}).toJson().constData();return 0;
} catch(const std::exception& error) {std::cerr<<error.what()<<'\n';return 2;}
