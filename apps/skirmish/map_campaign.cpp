#include "map_campaign.hpp"
#include "creature_runtime.hpp"
#include "arena.hpp"
#include "cure_runtime.hpp"
#include "health_rules.hpp"
#include "frozen_navigation.hpp"
#include <QCryptographicHash>
#include <algorithm>
#include <stdexcept>
#include <tuple>
namespace mnm::skirmish {
namespace {
constexpr const char *mapName="native-plains-crop-v1";
std::string sha(const game::Bytes &b) {
    return QCryptographicHash::hash(QByteArrayView(reinterpret_cast<const char*>(b.data()),b.size()),QCryptographicHash::Sha256).toHex().toStdString();
}
template<class T,class E> T take(std::variant<T,E> result) {
    if(auto *value=std::get_if<T>(&result)) return std::move(*value);
    throw std::invalid_argument("Installed Plains campaign resource refused");
}
game::Bytes read(const assets::AssetStore &store,const std::string &name) {
    auto file=take(store.open(name));return take(assets::readWhole(*file,32*1024*1024));
}
void append(game::Bytes &to,const game::Bytes &value) {
    for(unsigned i=0;i<4;++i) to.push_back(std::uint8_t(value.size()>>(8*i)));
    to.insert(to.end(),value.begin(),value.end());
}
void append(game::Bytes &to,const std::string &value) {append(to,game::Bytes(value.begin(),value.end()));}
}
MapCampaignResources::MapCampaignResources(const std::filesystem::path &root,bool enableCure,bool enableSphere,bool installedCreatures):cure_(enableCure || enableSphere),spheres_(enableSphere),creatures_(installedCreatures) {
    auto store=take(assets::AssetStore::create(root));
    const auto mapBytes=read(store,"Realms/Celtic/Plains/CPsec01.map"),
               ttdBytes=read(store,"Realms/Celtic/Plains/Terrain.ttd"),
               sprBytes=read(store,"Realms/Celtic/Plains/Terrain.spr");
    hashes_={sha(mapBytes),sha(ttdBytes),sha(sprBytes)};
    catalog_=take(assets::decodeTerrainCatalog(ttdBytes));sprites_=take(assets::decodeSprite(sprBytes));
    if(sprites_.frames.empty()) throw std::invalid_argument("Empty campaign terrain sprites");
    map_=scene::projectMapNavigation(take(assets::decodeMap(mapBytes)),catalog_,creatures_?scene::MapCrop{4,4,12,12}:scene::MapCrop{0,0,8,8});
    std::vector<std::byte> frozen;for(auto b:map_.frozen) frozen.push_back(std::byte(b));
    navigation_=sandbox::loadFrozenNavigationBytes(std::move(frozen),{},true,true);
    std::optional<TerrainBattle> terrain;
    if(creatures_) {
        mapDriver_=installedTerrainCreatureRuntime(root);arenaDriver_=installedCreatureRuntime(root);
        terrain=creatureTerrain(*mapDriver_);navigation_=terrain->navigation;
        map_.frozen=terrain->frozen;map_.standing.clear();
        for(auto p:terrain->standing)map_.standing.push_back({p.x,p.y,p.z});
    }
    // Navigation extends its hash with occupancy policy; geometry validation
    // compares the exact owned raw MAP/TTD/frozen bytes before that extension.
    scene::validateMapGeometry(map_.geometry,catalog_,map_.frozen);
    mission_={mapName,navigation_->binding().dimensions,{{{1,6,3},{6,1,3}}},
              {{5,2,3},{5,1,3},{6,2,3},{4,2,3}}, {}};
    for(const auto &p:map_.standing) mission_.standing.push_back({p[0],p[1],p[2]});
    std::sort(mission_.standing.begin(),mission_.standing.end(),[](auto a,auto b){return std::tie(a.x,a.y,a.z)<std::tie(b.x,b.y,b.z);});
    if(terrain) {
        mission_.map=terrainBattleMap;mission_.dimensions=navigation_->binding().dimensions;
        mission_.wizards=terrain->wizards;mission_.aiSummons.clear();
        const auto distance=[](auto a,auto b){return std::max(std::abs(a.x-b.x),std::abs(a.y-b.y));};
        const auto overlaps=[](auto a,auto b){return std::abs(a.x-b.x)<2 && std::abs(a.y-b.y)<2 && a.z==b.z;};
        for(auto p:mission_.standing)if(distance(p,mission_.wizards[1])<=2 && !overlaps(p,mission_.wizards[0]) && !overlaps(p,mission_.wizards[1]))mission_.aiSummons.push_back(p);
        std::stable_sort(mission_.aiSummons.begin(),mission_.aiSummons.end(),[&](auto a,auto b){return distance(a,mission_.wizards[0])<distance(b,mission_.wizards[0]);});
    }
    game::validateMission(mission_);
    if(spheres_) {
        sphereEnvironment_.dimensions=mission_.dimensions;
        if(creatures_){sphereEnvironment_.creatureWidth=2;sphereEnvironment_.creatureHeight=2;}
        std::vector<std::array<std::uint8_t,32>> palette(1);
        for(const auto &cell:map_.geometry.cells) {
            std::array<std::uint8_t,32> bitmap{};
            const auto &record=catalog_.records.at(cell.definition);
            std::copy_n(record.begin(),32,bitmap.begin());
            auto it=std::find(palette.begin(),palette.end(),bitmap);
            if(it==palette.end()) {
                if(palette.size()==4096) throw std::invalid_argument("Campaign Sphere terrain palette bound");
                palette.push_back(bitmap);it=palette.end()-1;
            }
            sphereEnvironment_.cells.push_back({unsigned(it-palette.begin()),unsigned(cell.flags8)|(unsigned(cell.flags10)<<16)});
        }
        sphereEnvironment_.occupancy=std::move(palette);
    }
    auto &p=definition_.initial;p.identity={creatures_?"native-plains-creature-campaign":"native-plains-campaign","binding-pending"};
    p.player="player";p.realm="celtic";p.region="plains-crossing";
    game::CampaignWizard player;player.id="player";player.spells={creatures_?"summon-crocodile":"summon-sentinel","arc-bolt"};
    if(cure_) player.spells.push_back("cure");
    if(spheres_) player.spells.push_back("magic-sphere");
    game::CampaignWizard rival;rival.id="rival";p.wizards={player,rival};
    p.realms={{"celtic",{{"plains-crossing","rival",false},{"arena-finale","rival",false}}}};
    definition_.missions={{"celtic","plains-crossing",mission_.map,"rival"},
                          {"celtic","arena-finale",game::arenaMission().map,"rival"}};
    auto binding=game::encodeCampaign(p);append(binding,std::string(creatures_?"linear-plains-creature-v1;4,4,12,12":"linear-plains-crop-v1;0,0,8,8"));
    for(const auto &hash:hashes_) append(binding,hash);
    for(const auto &m:definition_.missions) {
        append(binding,m.realm);append(binding,m.region);append(binding,m.map);append(binding,m.opponent);
        append(binding,game::encodeSnapshot(create(m).world().state()));
    }
    p.identity.contentRevision="plains-v1-"+sha(binding);
}
game::SkirmishSession MapCampaignResources::create(const game::CampaignMission &m) const {
    if(m.map==mission_.map) return {mission_,navigation_,ordinaryHealthRules(),spheres_?battleRuntime(sphereEnvironment_):cure_?cureRuntime(mission_.dimensions):nullptr,mapDriver_};
    if(m.map==game::arenaMission().map) return game::SkirmishSession(arenaNavigation(),ordinaryHealthRules(),spheres_?battleRuntime():cure_?cureRuntime():nullptr,arenaDriver_);
    throw std::invalid_argument("Unknown installed campaign mission");
}
game::SkirmishSession MapCampaignResources::restore(const game::CampaignMission &m,game::State state) const {
    const auto expected=create(m);
    if(state.map!=m.map) throw std::invalid_argument("Campaign mission map mismatch");
    game::World world(0);world.restore(std::move(state));
    game::SkirmishSession restored(std::move(world),m.map==mission_.map ? navigation_ : arenaNavigation(),ordinaryHealthRules(),expected.effectsRuntime(),expected.combat().driver());
    if(!game::sameMission(restored.mission(),expected.mission()))
        throw std::invalid_argument("Campaign mission policy mismatch");
    return restored;
}
std::shared_ptr<const MapCampaignResources> mapCampaignResources(const std::filesystem::path &root,bool enableCure,bool enableSphere,bool installedCreatures) {
    return std::make_shared<const MapCampaignResources>(root,enableCure,enableSphere,installedCreatures);
}
game::CampaignSession newMapCampaign(std::shared_ptr<const MapCampaignResources> resources) {
    if(!resources) throw std::invalid_argument("Select installed campaign resources first");
    return {resources->definition(),resources};
}
game::CampaignSession loadMapCampaign(std::shared_ptr<const MapCampaignResources> resources,const std::filesystem::path &path) {
    auto s=newMapCampaign(std::move(resources));s.restore(path);return s;
}
game::CampaignSession loadMapCampaign(const std::filesystem::path &root,const std::filesystem::path &path) {
    const auto progress=game::decodeCampaign(game::readSnapshot(path).campaign);
    const bool enabled=std::any_of(progress.wizards.begin(),progress.wizards.end(),[&](const auto &w){return w.id==progress.player && std::find(w.spells.begin(),w.spells.end(),"cure")!=w.spells.end();});
    const bool spheres=std::any_of(progress.wizards.begin(),progress.wizards.end(),[&](const auto &w){return w.id==progress.player && std::find(w.spells.begin(),w.spells.end(),"magic-sphere")!=w.spells.end();});
    const bool creatures=progress.identity.id=="native-plains-creature-campaign";
    return loadMapCampaign(mapCampaignResources(root,enabled,spheres,creatures),path);
}
} // namespace mnm::skirmish
