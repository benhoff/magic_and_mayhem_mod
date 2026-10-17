#include "map_campaign.hpp"
#include "campaign_definition.hpp"
#include "campaign-graph-pilot.hpp"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <fstream>
#include <iostream>
#include <stdexcept>
namespace g=mnm::game;
namespace app=mnm::skirmish;
void need(bool ok,const char* why) {if(!ok) throw std::runtime_error(why);}
template<class F> void refuses(F action) {bool rejected=false;try{action();}catch(const std::invalid_argument&){rejected=true;}need(rejected,"expected installed graph refusal");}
g::Bytes bytes(const g::CampaignSession& s) {return g::encodeCampaignSnapshot(s.checkpoint(),s.progress().identity);}
std::shared_ptr<const app::MapCampaignResources> resources;
std::filesystem::path output;
void save(const std::string& name,const g::CampaignSession& s) {if(!output.empty()) need(s.save(output/(name+".mnmc")).durable,"boundary save");}
unsigned play(g::CampaignSession& s,const std::string& name) {
    unsigned ticks=0;save(name+"-start",s);
    for(;s.battle() && ticks<1024;++ticks) {
        mnm::tests::campaignPilot(s);s.step();if(ticks==23) save(name+"-mid",s);
    }
    if(s.battle()) std::cerr<<"unfinished mission "<<s.progress().region<<" tick "<<ticks<<'\n';
    need(!s.battle() && s.progress().lastResult->outcome==g::CampaignOutcome::victory,"full-health mission strategy failed");
    save(name+"-win",s);return ticks;
}
void parserContract() {
    const auto file=std::filesystem::path(__FILE__).parent_path().parent_path()/"apps/skirmish/plains_campaign.json";
    std::ifstream in(file);const std::string text((std::istreambuf_iterator<char>(in)),{});need(!text.empty(),"campaign definition input");
    const auto doc=QJsonDocument::fromJson(QByteArray::fromStdString(text)).object();
    const auto bad=[&](QJsonObject o){refuses([&]{app::decodeCampaignDefinition(QJsonDocument(o).toJson().toStdString(),resources->mission(),g::arenaMission().map);});};
    auto j=doc;j["unknown"]=true;bad(j);j=doc;j["version"]=1.5;bad(j);j=doc;j["entry"]="watchpost";bad(j);
    j=doc;j["initial_spells"]=QJsonArray{"arc-bolt","arc-bolt"};bad(j);j=doc;j["initial_spells"]=QJsonArray{"unknown"};bad(j);
    j=doc;j["missions"]=false;bad(j);
    const auto nodes=doc["missions"].toArray();
    for(const auto& kind:{"missing","cycle","map","reward","duplicate","mixed-reward","dead-end"}) {
        auto a=nodes;auto n=a[1].toObject();
        if(std::string(kind)=="missing") n["requires_all"]=QJsonArray{"missing"};
        else if(std::string(kind)=="cycle") n["requires_all"]=QJsonArray{"last-stand"};
        else if(std::string(kind)=="map") n["map"]="unknown";
        else if(std::string(kind)=="reward") n["rewards"]=QJsonArray{"unknown"};
        else if(std::string(kind)=="duplicate") n["id"]="opening";
        else if(std::string(kind)=="mixed-reward") {auto p=n["plan"].toObject();p["reward_spell"]="cure";n["plan"]=p;}
        else {auto final=a[3].toObject();final["requires_any"]=QJsonArray{"watchpost"};a[3]=final;}
        a[1]=n;j=doc;j["missions"]=a;bad(j);
    }
    refuses([&]{app::decodeCampaignDefinition("{",resources->mission(),g::arenaMission().map);});
    refuses([&]{app::decodeCampaignDefinition(std::string(256*1024+1,' '),resources->mission(),g::arenaMission().map);});
}
bool earnedEffect(const g::CampaignCheckpoint& finale,unsigned route) {
    auto s=app::newMapCampaign(resources);s.restore(finale);
    auto c=s.battle()->combat();auto m=s.battle()->state();
    g::Entity e;e.type=c.navigation()->creatureType();e.owner=route==1?0:1;e.x=route==1?1:3;e.y=3;e.z=1;
    const auto target=c.spawn(e,c.driver()->creatureStats());m.units.push_back({target,g::UnitRole::sentinel});
    if(route==1) c.damage(target,20,m.wizards[1].actor);
    auto fixture=s.checkpoint();fixture.world=c.world().state();fixture.world.campaign=g::encodeSkirmish(m);s.restore(fixture);
    s.enqueue({route==1?g::SkirmishOrderKind::cure:g::SkirmishOrderKind::sphere,m.wizards[0].actor,target,{}});
    save("route-"+std::to_string(route)+"-earned-pending",s);
    bool admitted=false,delivered=false;
    for(unsigned i=0;i<48 && s.battle() && !delivered;++i) {
        const auto report=s.step();
        for(const auto& event:report.battle->events) admitted|=event.kind==g::SkirmishEventKind::casting && event.spell==(route==1?g::Spell::cure:g::Spell::sphere);
        for(const auto& event:report.battle->combat.events)
            delivered|=event.target==target && event.kind==(route==1?g::CombatEventKind::healed:g::CombatEventKind::hit) && (route==1 || event.actor==m.wizards[0].actor);
    }
    need(admitted && delivered,"earned spell did not deliver in finale");return delivered;
}
int main(int argc,char** argv) try {
    need(argc>=2,"campaign-graph-test ROOT [emit DIR | pilot INPUT OUTPUT TICKS]");
    resources=app::mapCampaignResources(argv[1],true,true,true,true,true,false,true);
    if(argc==6 && std::string(argv[2])=="pilot") {
        auto s=app::loadMapCampaign(resources,argv[3]);const auto ticks=std::stoul(argv[5]);need(ticks<=4096,"pilot tick bound");
        for(unsigned i=0;i<ticks && s.battle();++i) {mnm::tests::campaignPilot(s);s.step();}
        need(s.save(argv[4]).durable,"pilot save");std::cout<<"{\"success\":true}\n";return 0;
    }
    need(argc==2 || (argc==4 && std::string(argv[2])=="emit"),"unknown test mode");
    if(argc==4) {output=argv[3];std::filesystem::create_directory(output);}
    parserContract();auto opening=app::newMapCampaign(resources);save("initial",opening);
    need(opening.progress().identity.id=="native-plains-crossroads-campaign" && opening.availableMissions().size()==1 && !opening.spellAvailable(g::Spell::cure) && !opening.spellAvailable(g::Spell::sphere),"initial chapter/loadout");
    const auto idle=bytes(opening);refuses([&]{opening.selectMission("celtic","arena-finale");});need(bytes(opening)==idle,"locked finale selection published");
    opening.startMission();const auto openingTicks=play(opening,"opening");
    need(opening.availableMissions().size()==2 && opening.progress().graph->choices.empty(),"opening did not unlock alternatives");
    const auto fork=opening.checkpoint();save("fork",opening);
    std::vector<unsigned> lengths;unsigned rewarded=0;
    for(unsigned route:{1U,2U}) {
        auto s=app::newMapCampaign(resources);s.restore(fork);
        const std::string name="route-"+std::to_string(route),region=route==1?"last-stand":"watchpost",other=route==1?"watchpost":"last-stand";
        s.selectMission("celtic",region);save(name+"-selected",s);
        auto selected=app::newMapCampaign(resources);selected.restore(s.checkpoint());need(bytes(selected)==bytes(s),"idle route selection continuation");
        s.startMission();need(s.progress().graph->choices==std::vector<unsigned>{route},"installed route commitment");
        auto active=bytes(s);refuses([&]{s.selectMission("celtic",other);});need(bytes(s)==active,"active route switch");
        auto forged=s.checkpoint();forged.campaign.graph->choices={route==1?2U:1U};refuses([&]{s.restore(forged);});need(bytes(s)==active,"forged installed route published");
        lengths.push_back(play(s,name));
        need(s.progress().graph->completed==std::vector<unsigned>({0,route}) && s.availableMissions().size()==1 && s.progress().region=="arena-finale", "route did not rejoin finale");
        need(s.spellAvailable(g::Spell::cure)==(route==1) && s.spellAvailable(g::Spell::sphere)==(route==2),"installed route reward");
        const auto reward=bytes(s);s.step();s.step();need(bytes(s)==reward,"reward replay after idle ticks");
        refuses([&]{s.selectMission("celtic",other);});
        forged=s.checkpoint();forged.campaign.wizards[0].spells.push_back(route==1?"magic-sphere":"cure");refuses([&]{s.restore(forged);});need(bytes(s)==reward,"forged opposite reward published");
        s.startMission();
        const auto finale=s.checkpoint();rewarded+=earnedEffect(finale,route);
        auto clone=app::newMapCampaign(resources);clone.restore(finale);need(bytes(clone)==bytes(s),"finale loadout/world rebind");
        lengths.push_back(play(s,name+"-finale"));
        need(s.progress().completed && s.progress().graph->completed==std::vector<unsigned>({0,route,3}) && s.progress().graph->choices==std::vector<unsigned>{route} && s.availableMissions().empty(),"installed chapter did not complete");
        const auto complete=bytes(s);clone.restore(s.checkpoint());need(bytes(clone)==complete,"completed chapter restore");
        need(!s.step().result && bytes(s)==complete,"completed result replay");refuses([&]{s.startMission();});
    }
    std::cout<<"{\"success\":true,\"routes\":2,\"opening_ticks\":"<<openingTicks<<",\"mission_ticks\":[";
    for(std::size_t i=0;i<lengths.size();++i) {if(i) std::cout<<',';std::cout<<lengths[i];}
    std::cout<<"],\"earned_spell_deliveries\":"<<rewarded<<",\"chapter_completed\":true}\n";
} catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
