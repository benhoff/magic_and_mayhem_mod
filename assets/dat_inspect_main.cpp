#include "dat.hpp"
#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <iostream>
#include <stdexcept>
using namespace mnm::assets;
template<class Range> QJsonArray words(const Range& range) {QJsonArray a; for(auto v:range) a.append(qint64(v)); return a;}
template<class T> T checked(std::variant<T,DatError> result) {if(auto* e=std::get_if<DatError>(&result)) throw std::runtime_error(e->detail+" at byte "+std::to_string(e->offset)); return std::get<T>(std::move(result));}
int main(int argc,char** argv) try {
    QCoreApplication app(argc,argv);
    if(argc!=4 || (std::string(argv[1])!="brain" && std::string(argv[1])!="experience")) throw std::runtime_error("Usage: mnm-dat-inspect brain|experience ROOT PATH.dat");
    auto configured=AssetStore::create(argv[2]); if(auto* e=std::get_if<Error>(&configured)) throw std::runtime_error(e->detail);
    auto store=std::get<AssetStore>(std::move(configured)); auto opened=store.open(argv[3]); if(auto* e=std::get_if<Error>(&opened)) throw std::runtime_error(e->detail);
    auto file=std::get<std::unique_ptr<AssetFile>>(std::move(opened)); QJsonObject output;
    if(std::string(argv[1])=="brain") {
        auto a=checked(loadBrainDat(*file)); file.reset(); QJsonArray models;
        for(const auto& m:a.models) {QJsonArray layers; for(const auto& l:m.layers) {QJsonArray nodes; for(const auto& n:l.nodes) nodes.append(words(n)); layers.append(QJsonObject{{"scalar_bits",qint64(l.scalarBits)},{"tail_bits",qint64(l.tailBits)},{"nodes",nodes},{"matrix",words(l.matrix)}});}
            models.append(QJsonObject{{"key",qint64(m.key)},{"dimension",qint64(m.dimension)},{"layers",layers}});}
        output={{"source_bytes",qint64(a.sourceBytes)},{"models",models}};
    } else {
        auto a=checked(loadExperienceDat(*file)); file.reset(); QJsonArray samples;
        for(const auto& s:a.samples) samples.append(QJsonObject{{"key",qint64(s.key)},{"values",words(s.values)},{"parameters",words(s.parameters)}});
        output={{"source_bytes",qint64(a.sourceBytes)},{"samples",samples}};
    }
    std::cout<<QJsonDocument(output).toJson(QJsonDocument::Compact).constData()<<'\n'; return 0;
} catch(const std::exception& e) {std::cerr<<e.what()<<'\n'; return 2;}
