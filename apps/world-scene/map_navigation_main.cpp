#include "map_navigation.hpp"
#include "creature_profile.hpp"
#include "frozen_navigation.hpp"
#include "persistence/snapshot.hpp"
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <iostream>
#include <algorithm>
#include <cstring>
namespace {
void writeNew(QString path,const mnm::assets::Bytes& bytes){QFile f(path);if(!f.open(QIODevice::WriteOnly|QIODevice::NewOnly) || f.write(reinterpret_cast<const char*>(bytes.data()),bytes.size())!=qint64(bytes.size()) || !f.flush())throw std::runtime_error("Cannot create new MAP navigation output");}
QString hash(const mnm::assets::Bytes& b){return QString::fromLatin1(QCryptographicHash::hash(QByteArray(reinterpret_cast<const char*>(b.data()),b.size()),QCryptographicHash::Sha256).toHex());}
}
int main(int argc,char** argv)try{
    QCoreApplication app(argc,argv);
    if(argc!=9 && argc!=10 && argc!=17)throw std::invalid_argument("Usage: mnm-map-navigation ROOT MAP REALM X Y WIDTH HEIGHT NEW_OUTPUT_PREFIX [CREATURE_TYPE [SX SY SZ TX TY TZ TICKS]]");
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
    auto result=mnm::scene::projectMapNavigation(std::get<mnm::assets::MapAsset>(map),std::get<mnm::assets::TerrainCatalog>(catalog),crop);
    QJsonObject configuredProfile;std::optional<mnm::game::AnimationBinding> movementAnimation;
    if(argc>=10){
        const auto type=number(9);if(type!=10)throw std::invalid_argument("Only configured Redcap type 10 is admitted");
        auto cfgBytes=bytes("CFG/Encrypted/creature.cfg");auto packed=mnm::assets::decodePackedContainer(cfgBytes);
        if(auto* e=std::get_if<mnm::assets::PersistenceError>(&packed))throw std::runtime_error(e->detail);
        auto cfg=mnm::assets::decodeConfig(std::get<mnm::assets::PackedContainer>(packed).decoded);
        if(auto* e=std::get_if<mnm::assets::PersistenceError>(&cfg))throw std::runtime_error(e->detail);
        const auto profile=mnm::reconstruction::normalizeCreatureMovementConfig(mnm::assets::creatureMovementConfig(std::get<mnm::assets::Config>(cfg),type));
        const std::string aniRequest="Creatures/redcap.ani";auto aniBytes=bytes(aniRequest);
        auto decoded=mnm::assets::decodeAnimation(aniBytes);
        if(auto* e=std::get_if<mnm::assets::AnimationError>(&decoded))throw std::runtime_error(e->detail);
        const auto& ani=std::get<mnm::assets::Animation>(decoded);
        auto nameEnd=std::find(ani.spriteName.begin(),ani.spriteName.end(),0);
        if(nameEnd==ani.spriteName.end())throw std::invalid_argument("ANI sprite name lacks terminator");
        const auto spriteRequest="Creatures/"+std::string(ani.spriteName.begin(),nameEnd);
        if(spriteRequest!="Creatures/redcap.spr")throw std::invalid_argument("Configured Redcap ANI sprite binding differs");
        const auto spriteBytes=bytes(spriteRequest);
        result=mnm::scene::projectCreatureMapNavigation(std::get<mnm::assets::MapAsset>(map),std::get<mnm::assets::TerrainCatalog>(catalog),crop,profile,ani);
        QJsonArray samples;for(auto n:mnm::reconstruction::groundMovementSamples(ani))samples.append(int(n));
        configuredProfile={{"type",int(type)},{"height",profile.height},{"width",profile.width},{"acceleration",profile.acceleration},{"swimming",profile.swimming},{"can_fly",profile.canFly},{"ground_speed",profile.groundSpeed},{"flying_speed",profile.flyingSpeed},{"animation_request",QString::fromStdString(aniRequest)},{"sprite_request",QString::fromStdString(spriteRequest)},{"sequence_base",0},{"cfg_sha256",hash(cfgBytes)},{"ani_sha256",hash(aniBytes)},{"sprite_sha256",hash(spriteBytes)},{"samples",samples},{"generator_maximum",int(mnm::reconstruction::groundMovementMaximum(mnm::reconstruction::groundMovementSamples(ani)))}};
        mnm::game::AnimationBinding binding;binding.sequenceBase=0;binding.data.resize(aniBytes.size());std::memcpy(binding.data.data(),aniBytes.data(),aniBytes.size());movementAnimation=std::move(binding);
    }
    const auto payload=mnm::scene::mapPayload(result.geometry);QJsonArray standing;
    for(const auto& at:result.standing)standing.append(QJsonArray{at[0],at[1],at[2]});
    QJsonObject report{{"policy","ordinary terrain projection; sealed crop; synthetic one-cell profile"},{"source_map",QString::fromUtf8(argv[2])},{"realm",QString::fromUtf8(argv[3])},{"source_map_sha256",hash(mapBytes)},{"ttd_sha256",hash(ttdBytes)},{"crop",QJsonArray{int(crop.x),int(crop.y),int(crop.width),int(crop.height)}},{"layers",int(result.geometry.layers)},{"projected_source_objects",int(result.projectedObjects)},{"projected_source_references",int(result.projectedReferences)},{"sealed_cells",int(result.sealedCells)},{"standing",standing},{"geometry_sha256",hash(payload)},{"frozen_sha256",hash(result.frozen)},{"live_validated",false}};
    if(movementAnimation){report["policy"]="ordinary terrain projection; sealed crop; configured ground Redcap";report["creature_profile"]=configuredProfile;}
    const auto prefix=QString::fromUtf8(argv[8]);
    std::optional<mnm::game::State> checkpoint;
    if(argc==17){
        std::vector<std::byte> frozen(result.frozen.size());std::memcpy(frozen.data(),result.frozen.data(),result.frozen.size());
        auto navigation=mnm::sandbox::loadFrozenNavigationBytes(frozen,movementAnimation);
        mnm::game::World world(8);auto state=world.state();state.map=std::filesystem::absolute((prefix+".frozen").toStdString()).lexically_normal().string();state.navigation=navigation->binding();state.animation=movementAnimation;world.restore(std::move(state));
        mnm::game::MovementSession session(std::move(world),navigation);
        mnm::game::Entity e;e.type=navigation->creatureType();e.x=int(number(10));e.y=int(number(11));e.z=int(number(12));
        const auto handle=session.spawn(e,true,true,true);session.move(handle,{int(number(13)),int(number(14)),int(number(15))});
        const auto ticks=number(16);if(ticks>10000)throw std::invalid_argument("Configured preview tick limit");
        for(unsigned i=0;i<ticks;++i)session.step();
        checkpoint=session.world().state();
        report["checkpoint"] = prefix+".mnms";report["checkpoint_ticks"]=int(ticks);
    }
    writeNew(prefix+".frozen",result.frozen);writeNew(prefix+".geometry",payload);
    if(checkpoint){const auto result=mnm::game::writeSnapshot((prefix+".mnms").toStdString(),*checkpoint);if(!result.durable)throw std::runtime_error(result.detail);}
    const auto json=QJsonDocument(report).toJson();writeNew(prefix+".json",mnm::assets::Bytes(json.begin(),json.end()));
    std::cout<<json.constData();return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
