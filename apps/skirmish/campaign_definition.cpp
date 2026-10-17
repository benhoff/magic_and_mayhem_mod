#include "campaign_definition.hpp"
#include "objective_definition.hpp"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include <map>
#include <set>
#include <stdexcept>
namespace mnm::skirmish {
namespace {
void need(bool ok,const char* why) {if(!ok) throw std::invalid_argument(why);}
void keys(const QJsonObject& o,QStringList names) {names.sort();need(o.keys()==names,"unknown/missing campaign field");}
std::string text(const QJsonValue& v,bool empty=false) {
    need(v.isString(),"campaign string expected");const auto s=v.toString().toStdString();
    need((empty || !s.empty()) && s.size()<=255 && s.find('\0')==std::string::npos,"invalid campaign string");return s;
}
QJsonArray array(const QJsonValue& v,unsigned limit) {need(v.isArray() && unsigned(v.toArray().size())<=limit,"campaign array expected/budget");return v.toArray();}
QJsonObject object(const QJsonValue& v) {need(v.isObject(),"campaign object expected");return v.toObject();}
}
game::CampaignDefinition decodeCampaignDefinition(std::string_view bytes,const game::SkirmishMission& plains,const std::string& arenaMap) {
    need(bytes.size()<=256*1024,"campaign JSON byte budget");
    QJsonParseError error;const auto doc=QJsonDocument::fromJson(QByteArray(bytes.data(),int(bytes.size())),&error);
    need(error.error==QJsonParseError::NoError && doc.isObject(),"invalid campaign JSON");const auto o=doc.object();
    keys(o,{"version","id","realm","entry","finale","initial_spells","missions"});
    need(o["version"].isDouble() && o["version"].toDouble()==1,"unsupported campaign definition version");
    const auto nodes=array(o["missions"],16);need(nodes.size()>=2,"campaign mission roster empty");
    std::map<std::string,unsigned> ids;
    for(const auto value:nodes) {const auto node=object(value);need(ids.emplace(text(node["id"]),unsigned(ids.size())).second,"duplicate campaign mission ID");}
    need(text(object(nodes[0])["id"])==text(o["entry"]) && text(object(nodes.last())["id"])==text(o["finale"]),"campaign entry/finale order");
    game::CampaignDefinition d;d.graph=game::CampaignGraph{};d.spellLoadout=true;
    auto& p=d.initial;p.identity={text(o["id"]),"binding-pending"};p.player="player";p.realm=text(o["realm"]);p.region=text(o["entry"]);p.graph=game::CampaignGraphProgress{};
    game::CampaignWizard player;player.id=p.player;
    for(const auto value:array(o["initial_spells"],4)) {const auto spell=text(value);need(game::campaignSpell(spell),"unsupported initial spell");player.spells.push_back(spell);}
    game::CampaignWizard rival;rival.id="rival";p.wizards={player,rival};p.realms={{p.realm,{}}};
    const auto dependencies=[&](const QJsonValue& value) {
        std::vector<std::uint32_t> result;
        for(const auto v:array(value,16)) {const auto it=ids.find(text(v));need(it!=ids.end(),"unknown campaign prerequisite");result.push_back(it->second);}
        return result;
    };
    for(const auto value:nodes) {
        const auto n=object(value);keys(n,{"id","map","requires_all","requires_any","choice","rewards","plan"});
        const auto id=text(n["id"]),map=text(n["map"]);need(map=="plains" || map=="arena","unknown campaign map");
        d.missions.push_back({p.realm,id,map=="plains"?plains.map:arenaMap,"rival"});
        p.realms[0].regions.push_back({id,"rival",false});
        game::CampaignNode node;node.all=dependencies(n["requires_all"]);node.any=dependencies(n["requires_any"]);node.choice=text(n["choice"],true);
        for(const auto spell:array(n["rewards"],4)) node.rewards.push_back(text(spell));
        d.graph->nodes.push_back(std::move(node));
        if(n["plan"].isNull()) d.plans.push_back(std::nullopt);
        else {
            const auto plan=object(n["plan"]);auto parsed=decodeObjectiveDefinition(QJsonDocument(plan).toJson().toStdString());
            need(parsed.rewardSpell.empty(),"campaign rewards must be declared on mission nodes");
            game::validateMissionPlan(parsed,map=="plains"?plains:game::arenaMission());d.plans.push_back(std::move(parsed));
        }
    }
    game::validateCampaignGraph(*d.graph);game::encodeCampaign(p);return d;
}
}
