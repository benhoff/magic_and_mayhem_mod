#include "nod.hpp"
#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <iostream>
#include <stdexcept>
using namespace mnm::assets;
int main(int argc,char** argv) try {
    QCoreApplication app(argc,argv);
    if(argc!=3) throw std::runtime_error("Usage: mnm-nod-inspect ROOT PATH.nod");
    auto configured=AssetStore::create(argv[1]);
    if(const auto* error=std::get_if<Error>(&configured)) throw std::runtime_error(error->detail);
    auto store=std::get<AssetStore>(std::move(configured));auto opened=store.open(argv[2]);
    if(const auto* error=std::get_if<Error>(&opened)) throw std::runtime_error(error->detail);
    auto file=std::get<std::unique_ptr<AssetFile>>(std::move(opened));auto result=loadNod(*file);file.reset();
    if(const auto* error=std::get_if<NodError>(&result)) throw std::runtime_error(error->detail+" at byte "+std::to_string(error->offset));
    const auto& asset=std::get<NodAsset>(result);QJsonArray nodes;
    for(const auto& node:asset.nodes) {
        QJsonArray connections,tail;
        for(auto value:node.tailWords) tail.append(qint64(value));
        for(const auto& connection:node.connections) {
            const auto hex=QByteArray(reinterpret_cast<const char*>(connection.metadata.data()),17).toHex();
            connections.append(QJsonObject{{"value",qint64(connection.value)},{"target",connection.target},
                {"metadata_hex",QString::fromLatin1(hex)}});
        }
        nodes.append(QJsonObject{{"state",qint64(node.state)},{"word4",qint64(node.word4)},
            {"position",QJsonArray{node.position[0],node.position[1],node.position[2]}},
            {"connections",connections},{"tail_words",tail}});
    }
    std::cout<<QJsonDocument(QJsonObject{{"version",qint64(asset.version)},{"source_bytes",qint64(asset.sourceBytes)},
        {"node_count",qint64(asset.nodes.size())},{"nodes",nodes},
        {"trailer_words",QJsonArray{qint64(asset.trailerWords[0]),qint64(asset.trailerWords[1])}}}).toJson().constData();return 0;
} catch(const std::exception& error) {std::cerr<<error.what()<<'\n';return 2;}
