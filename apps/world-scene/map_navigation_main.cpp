#include "map_navigation.hpp"
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <iostream>
namespace {
void writeNew(QString path,const mnm::assets::Bytes& bytes){QFile f(path);if(!f.open(QIODevice::WriteOnly|QIODevice::NewOnly) || f.write(reinterpret_cast<const char*>(bytes.data()),bytes.size())!=qint64(bytes.size()) || !f.flush())throw std::runtime_error("Cannot create new MAP navigation output");}
QString hash(const mnm::assets::Bytes& b){return QString::fromLatin1(QCryptographicHash::hash(QByteArray(reinterpret_cast<const char*>(b.data()),b.size()),QCryptographicHash::Sha256).toHex());}
}
int main(int argc,char** argv)try{
    QCoreApplication app(argc,argv);
    if(argc!=9)throw std::invalid_argument("Usage: mnm-map-navigation ROOT MAP REALM X Y WIDTH HEIGHT NEW_OUTPUT_PREFIX");
    auto configured=mnm::assets::AssetStore::create(argv[1]);
    if(auto* e=std::get_if<mnm::assets::Error>(&configured))throw std::runtime_error(e->detail);
    auto store=std::get<mnm::assets::AssetStore>(std::move(configured));
    const auto bytes=[&](std::string path){auto opened=store.open(path);if(auto* e=std::get_if<mnm::assets::Error>(&opened))throw std::runtime_error(e->detail);auto file=std::get<std::unique_ptr<mnm::assets::AssetFile>>(std::move(opened));auto loaded=mnm::assets::readWhole(*file,24*1024*1024);if(auto* e=std::get_if<mnm::assets::Error>(&loaded))throw std::runtime_error(e->detail);return std::get<mnm::assets::Bytes>(std::move(loaded));};
    auto mapBytes=bytes(argv[2]),ttdBytes=bytes(std::string(argv[3])+"/Terrain.ttd");
    auto map=mnm::assets::decodeMap(mapBytes);auto catalog=mnm::assets::decodeTerrainCatalog(ttdBytes);
    if(auto* e=std::get_if<mnm::assets::PersistenceError>(&map))throw std::runtime_error(e->detail);
    if(auto* e=std::get_if<mnm::assets::TerrainCatalogError>(&catalog))throw std::runtime_error(e->detail);
    const auto number=[&](int i){bool ok=false;auto n=QString::fromUtf8(argv[i]).toUInt(&ok);if(!ok)throw std::invalid_argument("Crop requires unsigned integers");return n;};
    const mnm::scene::MapCrop crop{number(4),number(5),number(6),number(7)};
    const auto result=mnm::scene::projectMapNavigation(std::get<mnm::assets::MapAsset>(map),std::get<mnm::assets::TerrainCatalog>(catalog),crop);
    const auto payload=mnm::scene::mapPayload(result.geometry);QJsonArray standing;
    for(const auto& at:result.standing)standing.append(QJsonArray{at[0],at[1],at[2]});
    QJsonObject report{{"policy","ordinary terrain projection; sealed crop; synthetic one-cell profile"},{"source_map",QString::fromUtf8(argv[2])},{"realm",QString::fromUtf8(argv[3])},{"source_map_sha256",hash(mapBytes)},{"ttd_sha256",hash(ttdBytes)},{"crop",QJsonArray{int(crop.x),int(crop.y),int(crop.width),int(crop.height)}},{"layers",int(result.geometry.layers)},{"projected_source_objects",int(result.projectedObjects)},{"projected_source_references",int(result.projectedReferences)},{"sealed_cells",int(result.sealedCells)},{"standing",standing},{"geometry_sha256",hash(payload)},{"frozen_sha256",hash(result.frozen)},{"live_validated",false}};
    const auto prefix=QString::fromUtf8(argv[8]);writeNew(prefix+".frozen",result.frozen);writeNew(prefix+".geometry",payload);
    const auto json=QJsonDocument(report).toJson();writeNew(prefix+".json",mnm::assets::Bytes(json.begin(),json.end()));
    std::cout<<json.constData();return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
