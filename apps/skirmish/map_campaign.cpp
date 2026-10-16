#include "map_campaign.hpp"
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
MapCampaignResources::MapCampaignResources(const std::filesystem::path &root,bool enableCure):cure_(enableCure) {
    auto store=take(assets::AssetStore::create(root));
    const auto mapBytes=read(store,"Realms/Celtic/Plains/CPsec01.map"),
               ttdBytes=read(store,"Realms/Celtic/Plains/Terrain.ttd"),
               sprBytes=read(store,"Realms/Celtic/Plains/Terrain.spr");
    hashes_={sha(mapBytes),sha(ttdBytes),sha(sprBytes)};
    catalog_=take(assets::decodeTerrainCatalog(ttdBytes));sprites_=take(assets::decodeSprite(sprBytes));
    if(sprites_.frames.empty()) throw std::invalid_argument("Empty campaign terrain sprites");
    map_=scene::projectMapNavigation(take(assets::decodeMap(mapBytes)),catalog_,{0,0,8,8});
    std::vector<std::byte> frozen;for(auto b:map_.frozen) frozen.push_back(std::byte(b));
    navigation_=sandbox::loadFrozenNavigationBytes(std::move(frozen),{},true,true);
    // Navigation extends its hash with occupancy policy; geometry validation
    // compares the exact owned raw MAP/TTD/frozen bytes before that extension.
    scene::validateMapGeometry(map_.geometry,catalog_,map_.frozen);
    mission_={mapName,navigation_->binding().dimensions,{{{1,6,3},{6,1,3}}},
              {{5,2,3},{5,1,3},{6,2,3},{4,2,3}}, {}};
    for(const auto &p:map_.standing) mission_.standing.push_back({p[0],p[1],p[2]});
    std::sort(mission_.standing.begin(),mission_.standing.end(),[](auto a,auto b){return std::tie(a.x,a.y,a.z)<std::tie(b.x,b.y,b.z);});
    game::validateMission(mission_);
    auto &p=definition_.initial;p.identity={"native-plains-campaign","binding-pending"};
    p.player="player";p.realm="celtic";p.region="plains-crossing";
    game::CampaignWizard player;player.id="player";player.spells={"summon-sentinel","arc-bolt"};
    if(cure_) player.spells.push_back("cure");
    game::CampaignWizard rival;rival.id="rival";p.wizards={player,rival};
    p.realms={{"celtic",{{"plains-crossing","rival",false},{"arena-finale","rival",false}}}};
    definition_.missions={{"celtic","plains-crossing",mapName,"rival"},
                          {"celtic","arena-finale",game::arenaMission().map,"rival"}};
    auto binding=game::encodeCampaign(p);append(binding,std::string("linear-plains-crop-v1;0,0,8,8"));
    for(const auto &hash:hashes_) append(binding,hash);
    for(const auto &m:definition_.missions) {
        append(binding,m.realm);append(binding,m.region);append(binding,m.map);append(binding,m.opponent);
        append(binding,game::encodeSnapshot(create(m).world().state()));
    }
    p.identity.contentRevision="plains-v1-"+sha(binding);
}
game::SkirmishSession MapCampaignResources::create(const game::CampaignMission &m) const {
    if(m.map==mapName) return {mission_,navigation_,ordinaryHealthRules(),cure_?cureRuntime(mission_.dimensions):nullptr};
    if(m.map==game::arenaMission().map) return game::SkirmishSession(arenaNavigation(),ordinaryHealthRules(),cure_?cureRuntime():nullptr);
    throw std::invalid_argument("Unknown installed campaign mission");
}
game::SkirmishSession MapCampaignResources::restore(const game::CampaignMission &m,game::State state) const {
    const auto expected=create(m);
    if(state.map!=m.map) throw std::invalid_argument("Campaign mission map mismatch");
    game::World world(0);world.restore(std::move(state));
    game::SkirmishSession restored(std::move(world),m.map==mapName ? navigation_ : arenaNavigation(),ordinaryHealthRules(),expected.effectsRuntime());
    if(!game::sameMission(restored.mission(),expected.mission()))
        throw std::invalid_argument("Campaign mission policy mismatch");
    return restored;
}
std::shared_ptr<const MapCampaignResources> mapCampaignResources(const std::filesystem::path &root,bool enableCure) {
    return std::make_shared<const MapCampaignResources>(root,enableCure);
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
    return loadMapCampaign(mapCampaignResources(root,enabled),path);
}
} // namespace mnm::skirmish
