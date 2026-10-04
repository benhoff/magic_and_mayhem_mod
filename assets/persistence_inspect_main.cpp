#include "persistence.hpp"
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <iostream>
#include <stdexcept>
using namespace mnm::assets;
namespace {
QString string(const std::string& s) {return QString::fromLatin1(s.data(),static_cast<qsizetype>(s.size()));}
QString hash(const Bytes& b) {return QString::fromLatin1(QCryptographicHash::hash(QByteArray(reinterpret_cast<const char*>(b.data()),static_cast<qsizetype>(b.size())),QCryptographicHash::Sha256).toHex());}
template<class T> T checked(PersistenceResult<T> r) {
    if(auto* e=std::get_if<PersistenceError>(&r)) throw std::runtime_error(e->detail+" at byte "+std::to_string(e->offset));
    return std::get<T>(std::move(r));
}
}
int main(int argc,char** argv) try {
    QCoreApplication app(argc,argv);
    if(argc!=4) throw std::runtime_error("Usage: mnm-persistence-inspect ROOT TYPE PATH\nTypes: cfg packed-cfg container save-container realm regions realm-state sav vas");
    auto configured=AssetStore::create(argv[1]);if(auto* e=std::get_if<Error>(&configured)) throw std::runtime_error(e->detail);
    auto store=std::get<AssetStore>(std::move(configured));auto opened=store.open(argv[3]);
    if(auto* e=std::get_if<Error>(&opened)) throw std::runtime_error(e->detail);
    auto file=std::get<std::unique_ptr<AssetFile>>(std::move(opened));std::string type=argv[2];
    QJsonObject out{{"type",string(type)},{"path",QString::fromLocal8Bit(argv[3])}};
    if(type=="container"||type=="save-container") {
        auto c=checked(loadPackedContainer(*file,{},type=="save-container"?ContainerTransform::noCdSave:ContainerTransform::cfgBytes));out["mode"]=int(c.mode);out["decoded_bytes"]=qint64(c.decoded.size());out["decoded_sha256"]=hash(c.decoded);
    } else if(type=="cfg"||type=="packed-cfg") {
        auto c=checked(loadConfig(*file,type=="packed-cfg"));QJsonObject sections;
        for(const auto& s:c.sections) {QJsonObject keys;for(const auto& k:s.second) keys[string(k.first)]=string(k.second);sections[string(s.first)]=keys;}
        out["sections"]=sections;out["annotation_lines"]=qint64(c.annotations.size());
    } else if(type=="realm") {
        auto r=checked(loadRealmConfig(*file));out["name"]=string(r.name);out["next_realm"]=string(r.nextRealm);out["wizards"]=qint64(r.wizards.size());out["regions"]=int(r.regionCount);out["last_region"]=r.lastRegion;
        int owners=0;for(const auto& owner:r.regionOwners) if(owner) ++owners;out["specified_owners"]=owners;
    } else if(type=="regions") {
        auto r=checked(loadRegionNames(*file));out["lines"]=qint64(r.size());
    } else if(type=="realm-state") {
        auto r=checked(loadRealmState(*file));out["name"]=string(r.name);out["wizard_count"]=r.wizardCount;out["raw_sha256"]=hash(r.raw);
    } else if(type=="sav"||type=="vas") {
        auto s=checked(loadSavedGame(*file,type=="sav"));out["version"]=int(s.version);out["accounting_value"]=qint64(s.accountingValue);out["realm"]=string(s.realm.name);out["wizard_records"]=80;out["world_marker"]=int(s.worldMarker);out["opaque_world_bytes"]=qint64(s.worldTail.size());out["world_sha256"]=hash(s.worldTail);
    } else throw std::runtime_error("unknown type");
    file.reset();std::cout<<QJsonDocument(out).toJson().constData();return 0;
} catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 2;}
