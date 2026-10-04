#include "fp.hpp"
#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <iostream>
#include <stdexcept>
using namespace mnm::assets;
int main(int argc,char** argv) try {
    QCoreApplication app(argc,argv);
    if(argc!=3) throw std::runtime_error("Usage: mnm-fp-inspect ROOT PATH.fp");
    auto configured=AssetStore::create(argv[1]);
    if(const auto* error=std::get_if<Error>(&configured)) throw std::runtime_error(error->detail);
    auto store=std::get<AssetStore>(std::move(configured));auto opened=store.open(argv[2]);
    if(const auto* error=std::get_if<Error>(&opened)) throw std::runtime_error(error->detail);
    auto file=std::get<std::unique_ptr<AssetFile>>(std::move(opened));auto decoded=loadFp(*file);file.reset();
    if(const auto* error=std::get_if<FpError>(&decoded)) throw std::runtime_error(error->detail+" at byte "+std::to_string(error->offset));
    const auto& asset=std::get<FpAsset>(decoded);QJsonArray counts,offsets,flags,points;
    for(auto count:asset.pointCounts) counts.append(qint64(count));
    for(auto offset:asset.pointOffsets) offsets.append(qint64(offset));
    for(const auto& point:asset.flagPositions) flags.append(QJsonArray{point.x,point.y});
    for(const auto& point:asset.points) points.append(QJsonArray{point.x,point.y});
    std::cout<<QJsonDocument(QJsonObject{{"source_bytes",qint64(asset.sourceBytes)},
        {"version",qint64(asset.version)},{"path_count",qint64(asset.pathCount)},
        {"header_point",QJsonArray{asset.headerPoint.x,asset.headerPoint.y}},
        {"point_counts",counts},{"point_offsets",offsets},{"flag_positions",flags},
        {"point_count",qint64(asset.points.size())},{"points",points}}).toJson().constData();return 0;
} catch(const std::exception& error) {std::cerr<<error.what()<<'\n';return 2;}
