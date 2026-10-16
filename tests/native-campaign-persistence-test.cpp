#include "persistence/campaign.hpp"
#include <iostream>
#include <limits>
#include <stdexcept>

using namespace mnm::game;
namespace {
const CampaignIdentity identity{"synthetic-campaign", "fixture-content-v1"};
void need(bool ok,const char* why) {if(!ok) throw std::runtime_error(why);}
template<class F> void refuses(F f) {
    bool failed=false;try {f();} catch(const std::invalid_argument&) {failed=true;}
    need(failed,"invalid campaign accepted");
}
CampaignState fixture() {
    CampaignState c;c.identity=identity;c.player="player";c.realm="first";c.region="entry";c.difficulty=3;
    CampaignWizard p;p.id="player";p.experience=std::numeric_limits<std::uint64_t>::max();
    p.attributes={std::numeric_limits<std::int32_t>::min(),-1,0,1,42,std::numeric_limits<std::int32_t>::max()};
    p.spells={"summon", "heal"};p.inventory={"token", "token"};
    CampaignWizard other;other.id="other";c.wizards={p,other};
    c.realms={{"first",{{"entry", "other", false},{"exit",std::nullopt,false}}},
              {"second",{{"entry",std::nullopt,false}}}};
    return c;
}
CampaignCheckpoint worldFixture() {
    World w(4);Entity e;e.state={1,2,3};const auto a=w.spawn(e);const auto b=w.spawn(e);
    w.enqueue({Operation::target,a,b,{},{}});
    auto state=w.state();state.systems={9,8,7};state.campaign={4,5,6};
    return {state,fixture()};
}
// Authored test events only, not production battle resolution or reward rules.
void advance(CampaignCheckpoint& checkpoint,unsigned ticks) {
    World world(0);world.restore(checkpoint.world);
    for(unsigned i=0;i<ticks;++i) {
        world.step();checkpoint.world=world.state();auto c=checkpoint.campaign;++c.transition;
        if(c.transition==2) {checkpoint.world.map="fixture-map";c.battle=CampaignBattle{"first","entry",checkpoint.world.map,{"player","other"}};}
        if(c.transition==5) {c.battle.reset();c.realms[0].regions[0].completed=true;c.realms[0].regions[0].owner="player";c.region="exit";}
        if(c.transition==8) {c.realms[0].regions[1].completed=true;c.realm="second";c.region="entry";}
        if(c.transition==10) c.completed=true;
        setCampaign(checkpoint,c);world.restore(checkpoint.world);
    }
}
Bytes wire(const CampaignCheckpoint& c) {return encodeCampaignSnapshot(c,identity);}
void contract() {
    const auto c=fixture();const auto encoded=encodeCampaign(c);
    need(encodeCampaign(decodeCampaign(encoded))==encoded,"campaign roundtrip lost state");
    for(std::size_t n=0;n<encoded.size();++n) refuses([&]{decodeCampaign(Bytes(encoded.begin(),encoded.begin()+n));});
    auto bad=encoded;bad[0]^=1;refuses([&]{decodeCampaign(bad);});
    bad=encoded;bad[8]=2;refuses([&]{decodeCampaign(bad);});
    bad=encoded;bad[12]=255;bad[13]=255;bad[14]=255;bad[15]=255;refuses([&]{decodeCampaign(bad);});
    bad=encoded;bad.push_back(0);refuses([&]{decodeCampaign(bad);});
    bad=encoded;bad.back()=2;refuses([&]{decodeCampaign(bad);});
    CampaignLimits small;small.bytes=encoded.size()-1;
    refuses([&]{encodeCampaign(c,small);});refuses([&]{decodeCampaign(encoded,small);});
    small={};small.regions=2;refuses([&]{decodeCampaign(encoded,small);});
    small={};small.wizards=1;refuses([&]{decodeCampaign(encoded,small);});
    small={};small.entries=1;refuses([&]{decodeCampaign(encoded,small);});
    small={};small.stringBytes=3;refuses([&]{decodeCampaign(encoded,small);});
    small={};small.bytes=0;refuses([&]{encodeCampaign(c,small);});
    auto malformed=[&](auto edit) {auto v=c;edit(v);refuses([&]{encodeCampaign(v);});};
    malformed([](auto& v){v.difficulty=4;});
    malformed([](auto& v){v.player="absent";});
    malformed([](auto& v){v.realm="absent";});
    malformed([](auto& v){v.region="absent";});
    malformed([](auto& v){v.identity.id="";});
    malformed([](auto& v){v.identity.contentRevision=std::string("a\0b",3);});
    malformed([](auto& v){v.wizards.push_back(v.wizards.front());});
    malformed([](auto& v){v.realms.push_back(v.realms.front());});
    malformed([](auto& v){v.realms[0].regions.push_back(v.realms[0].regions[0]);});
    malformed([](auto& v){v.realms[0].regions[0].owner="absent";});
    malformed([](auto& v){v.wizards[0].spells.push_back("heal");});
    auto state=worldFixture();const auto before=wire(state);
    auto active=c;active.battle=CampaignBattle{c.realm,c.region,"battle-map",{"player","other"}};
    refuses([&]{setCampaign(state,active);});need(wire(state)==before,"map refusal mutated world");
    state.world.map="battle-map";setCampaign(state,active);
    const auto battle=wire(state);
    need(wire(decodeCampaignSnapshot(battle,identity))==battle,"active battle checkpoint lost state");
    auto invalidBattle=[&](auto edit) {auto v=active;edit(v);refuses([&]{encodeCampaign(v);});};
    invalidBattle([](auto& v){v.completed=true;});
    invalidBattle([](auto& v){v.battle->region="exit";});
    invalidBattle([](auto& v){v.battle->participants={};});
    invalidBattle([](auto& v){v.battle->participants={"other"};});
    invalidBattle([](auto& v){v.battle->participants={"player","absent"};});
    invalidBattle([](auto& v){v.battle->participants={"player","player"};});
    refuses([&]{decodeCampaignSnapshot(battle,{identity.id,"different-content"});});
    refuses([&]{decodeCampaignSnapshot(battle,{"different-campaign",identity.contentRevision});});
    auto corrupt=battle;corrupt.back()^=1;refuses([&]{decodeCampaignSnapshot(corrupt,identity);});
    Limits tiny;tiny.bytes=8;refuses([&]{setCampaign(state,active,tiny);});
    need(wire(state)==battle,"world storage failure mutated campaign");
    auto invalidWorld=state;invalidWorld.world.phase20=20;const auto prior=encodeCampaign(invalidWorld.campaign);
    refuses([&]{setCampaign(invalidWorld,c);});need(encodeCampaign(invalidWorld.campaign)==prior,"invalid world was partially updated");
    auto legacy=worldFixture();
    need(decodeSnapshot(encodeSnapshot(legacy.world)).campaign==Bytes({4,5,6}),"generic snapshot compatibility lost");
    refuses([&]{decodeCampaignSnapshot(encodeSnapshot(legacy.world),identity);});
    auto outer=decodeSnapshot(wire(legacy));outer.tick=1;
    refuses([&]{decodeCampaignSnapshot(encodeSnapshot(outer),identity);});
    outer=decodeSnapshot(wire(legacy));outer.systems.back()^=1;
    refuses([&]{decodeCampaignSnapshot(encodeSnapshot(outer),identity);});
    auto uninterrupted=worldFixture();advance(uninterrupted,3);
    auto checkpoint=wire(uninterrupted);auto resumed=decodeCampaignSnapshot(checkpoint,identity);
    advance(uninterrupted,9);advance(resumed,9);
    need(wire(uninterrupted)==wire(resumed),"campaign/world continuation diverged");
    const auto& end=resumed.campaign;
    need(end.completed && end.realm=="second" && end.transition==12,"campaign progress not retained");
    need(resumed.world.campaign==Bytes({4,5,6}) && resumed.world.systems==Bytes({9,8,7}),"opaque battle profile/systems lost");
    need(end.wizards[0].inventory.size()==2 && end.wizards[0].experience==c.wizards[0].experience,"wizard records changed");
}
}
int main(int argc,char** argv) try {
    if(argc==3 && std::string(argv[1])=="create") {
        need(writeCampaignSnapshot(argv[2],worldFixture(),identity).durable,"creation was not durable");
    } else if(argc==6 && std::string(argv[1])=="advance") {
        auto checkpoint=readCampaignSnapshot(argv[2],identity);
        advance(checkpoint,static_cast<unsigned>(std::stoul(argv[4])));
        need(writeCampaignSnapshot(argv[3],checkpoint,identity,std::string(argv[5])=="overwrite").durable,"publication was not durable");
    } else if(argc==1) contract();
    else throw std::invalid_argument("test usage: create FILE | advance INPUT OUTPUT TICKS new/overwrite");
    std::cout<<"Native campaign persistence passed.\n";return 0;
} catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
