#include "../apps/command-session/native_geometry.hpp"
#include "../apps/native-encounter/encounter.hpp"
#include "../game/persistence/snapshot.hpp"
#include <fstream>
#include <iostream>
#include <set>
namespace cs = mnm::command_session;
namespace en = mnm::encounter;
namespace game = mnm::game;
int main(int argc, char **argv) try {
  if (argc != 3) throw std::runtime_error("INSTALL NEW_OUTPUT");
  const std::filesystem::path output = argv[2];
  if (!std::filesystem::create_directories(output)) throw std::runtime_error("output exists");
  unsigned checks=0, restores=0, refusals=0, rollbacks=0, pending=0,
           issued=0, cureFrames=0, projectileFrames=0, casts=0, heals=0,
           impacts=0, summons=0, manaUses=0, regeneration=0, deaths=0, wizardDefeats=0, cleanups=0, frames=0;
  std::uint64_t digest=1469598103934665603ull;
  auto check=[&](bool b,const char *why){if(!b)throw std::runtime_error(why);++checks;};
  auto reject=[&](auto f){bool failed=false;try{f();}catch(const std::exception &){failed=true;}check(failed,"expected wizard refusal");++refusals;};
  std::ofstream snapshots(output/"snapshots.jsonl");
  for (unsigned journey=0;journey<4;++journey) {
    const unsigned seed=journey==1?31:17;
    auto p=en::decodeInputs(en::loadInputs(argv[1],{0,0,32,32},10));
    auto &initial=*p.initial.native.wizards;
    const std::string name=journey<2?"mixed-"+std::to_string(seed):journey==2?"care":"duel";
    if(journey==2) {
      for(auto &v:initial.controllers){v.learned.fill(false);v.learned[41]=true;v.producing=true;}
      initial.admission.resource[0]=256;
      auto &item=p.initial.world.creatures[0].inventory[0];
      item.kind=19;item.available=1;item.count=2;item.magnitude=40;
      p.initial.world.creatures[1].ai.health=100;
    }
    if(journey==3) {
      for(auto &v:initial.controllers){v.learned.fill(false);if(v.actor==0)v.learned[73]=true;}
      for(auto &a:p.initial.world.creatures)if(a.ai.type!=0){a.ai.active=false;a.ai.health=0;}
      p.initial.world.creatures[3].ai.health=40;
      for(auto &a:p.initial.world.creatures){a.ai.hth.fill(0);a.ai.enemyRadius.fill(0);a.ai.allyRadius.fill(0);}
      for(auto &cell:p.initial.native.cells){cell.occupant=0xffff;cell.flags&=std::uint16_t(~3u);}
      cs::prepareNativeContinuation(p.initial,p.native);
    }
    check(cs::encode(p.initial).at(4)==20,"wizard CMD1 version");
    reject([&]{(void)cs::encode(p.initial,19);});
    auto bad=p.initial;bad.native.wizards->nextSerial=0;
    reject([&]{(void)cs::encode(bad);});
    bad=p.initial;bad.native.wizards->controllers[0].actor=99;
    reject([&]{(void)cs::encode(bad);});
    bad=p.initial;bad.native.wizards->admission.direction[0]=8;
    reject([&]{(void)cs::encode(bad);});
    bad=p.initial;bad.native.wizards->tick=1;
    reject([&]{(void)cs::encode(bad);});
    bool inject=false, visited=false;
    std::array<bool,8> activated{};
    auto verifier=[&](const cs::Continuation &c){
      visited=true;
      for(unsigned i=0;i<activated.size();++i)
        if(c.native.wizards->usedSummonSlots[i]&&!activated[i]){
          check(c.execution[i].roamHome==c.world.creatures[i].ai.home,"summon wander home differs from spawn");
          activated[i]=true;
        }
      check(c.native.wizards->tick==c.native.tick,"wizard verifier clock");
      if(inject)throw std::runtime_error("injected wizard verifier failure");
    };
    auto session=en::create(p,seed,{},verifier);
    auto write=[&](const game::State &state,const std::string &suffix,unsigned tick){
      const auto file=name+"-"+suffix+".snapshot";
      game::writeSnapshot(output/file,state);
      if(journey<2)snapshots<<"{\"file\":\""<<file<<"\",\"seed\":"<<seed<<",\"tick\":"<<tick<<"}\n";
    };
    write(session.world().state(),"0",0);
    auto invalid=session.world().state();
    bad=cs::decode(invalid.systems);++bad.native.wizards->admission.spells[41].cost;
    invalid.systems=cs::encode(bad);
    reject([&]{en::runtime(p)->validate(invalid);});
    const auto paused=game::encodeSnapshot(session.world().state());
    session.step({false,false,false});
    check(game::encodeSnapshot(session.world().state())==paused,"paused wizard state changed");
    bool savedCure=false,savedProjectile=false,checkedEffect=false;
    const unsigned end=journey<2?2048:512;
    std::ofstream trace(output/(name+".jsonl"));
    std::vector<unsigned> health;
    for(const auto &a:p.initial.world.creatures)health.push_back(a.ai.health);
    for(unsigned frame=1;frame<=end;++frame) {
      try{session.step({});}catch(const std::exception &e){throw std::runtime_error(name+" frame="+std::to_string(frame)+": "+e.what());}
      ++frames;
      const auto state=session.world().state();const auto c=cs::decode(state.systems);
      const auto &s=*c.native.wizards;
      for(unsigned i=0;i<health.size();++i){
        if(health[i]&&!c.world.creatures[i].ai.health){++deaths;wizardDefeats+=c.world.creatures[i].ai.type==0;}
        health[i]=c.world.creatures[i].ai.health;
      }
      check(cs::encode(c)==state.systems,"wizard exact wire roundtrip");
      check(s.tick==frame,"wizard clock drift");
      bool activeCure=false,activeProjectile=false;
      for(const auto &v:s.controllers){
        pending+=!v.selection.reset;
        issued+=v.active&&v.active->issued;
        check(s.admission.resource[v.actor]<=v.maximumMana*256u,"wizard exceeded mana maximum");
      }
      for(const auto &e:s.effects)if(e.active){activeCure|=e.spell==41;activeProjectile|=e.spell==73;}
      cureFrames+=activeCure;projectileFrames+=activeProjectile;
      if(!checkedEffect&&activeProjectile){
        auto malformed=c;
        for(auto &e:malformed.native.wizards->effects)if(e.active&&e.spell==73){e.placement.units[0]=UINT32_MAX;break;}
        reject([&]{(void)cs::encode(malformed);});checkedEffect=true;
      }
      if(frame<=8||frame%64==0||(!savedCure&&activeCure)||(!savedProjectile&&activeProjectile)) {
        game::World world(0);world.restore(game::decodeSnapshot(game::encodeSnapshot(state)));
        game::CreatureRuleSession resumed(std::move(world),en::runtime(p));
        auto expected=session;expected.step({});resumed.step({});
        check(game::encodeSnapshot(expected.world().state())==game::encodeSnapshot(resumed.world().state()),"wizard sliced restore differs");
        ++restores;
      }
      if(frame==32){
        visited=false;inject=true;bool failed=false;
        try{session.step({});}catch(const std::exception &e){if(std::string(e.what()).find("injected wizard verifier failure")==std::string::npos)throw;failed=true;}
        inject=false;
        check(visited&&failed,"wizard verifier did not fail transaction");
        check(game::encodeSnapshot(session.world().state())==game::encodeSnapshot(state),"wizard rollback changed complete snapshot");
        ++rollbacks;
      }
      if(!savedCure&&activeCure){write(state,"cure",frame);savedCure=true;}
      if(!savedProjectile&&activeProjectile){write(state,"projectile",frame);savedProjectile=true;}
      if(frame==1||frame==3||frame==16||frame==64||frame==128||frame==256||frame==512)write(state,std::to_string(frame),frame);
      trace<<"{\"frame\":"<<frame<<",\"casts\":"<<s.casts<<",\"heals\":"<<s.heals<<",\"impacts\":"<<s.impacts<<",\"summons\":"<<s.summons<<",\"mana\":[";
      bool comma=false;for(const auto &v:s.controllers){trace<<(comma?",":"")<<s.admission.resource[v.actor];comma=true;}
      trace<<"],\"health\":[";comma=false;
      for(const auto &a:c.world.creatures){trace<<(comma?",":"")<<a.ai.health;comma=true;}
      trace<<"]}\n";
      for(auto byte:state.systems)digest=(digest^byte)*1099511628211ull;
      if(frame==end){
        casts+=s.casts;heals+=s.heals;impacts+=s.impacts;summons+=s.summons;manaUses+=s.manaUses;regeneration+=s.regeneration;
        if(journey<2){check(s.summons==2&&s.heals&&s.impacts,"mixed encounter omitted spell delivery");check(savedCure&&savedProjectile,"mixed encounter omitted effect checkpoint");}
        if(journey==2)check(s.manaUses&&s.regeneration&&s.heals,"care fixture omitted inventory/production/heal");
        if(journey==3)check(!c.world.creatures[3].ai.health&&s.impacts,"wizard projectile defeat missing");
        for(const auto &a:c.world.creatures)if(!a.ai.health&&!a.ai.active){
          ++cleanups;for(const auto &cell:c.native.cells)check(cell.occupant!=a.ai.slot+1,"dead actor retains occupancy");
          if(a.ai.type==0)for(const auto &v:s.controllers)if(v.actor==a.ai.slot)check(!v.active&&!v.best&&!v.candidate,"dead wizard retains task");
        }
        write(state,"end",frame);
      }
    }
  }
  check(pending&&issued&&cureFrames&&projectileFrames&&casts&&heals&&impacts&&summons&&manaUses&&regeneration&&deaths&&wizardDefeats,"wizard lifecycle witnesses missing");
  std::cout<<"{\"success\":true,\"checks\":"<<checks<<",\"frames\":"<<frames<<",\"restores\":"<<restores<<",\"refusals\":"<<refusals<<",\"rollbacks\":"<<rollbacks<<",\"pending\":"<<pending<<",\"issued\":"<<issued<<",\"cure_frames\":"<<cureFrames<<",\"projectile_frames\":"<<projectileFrames<<",\"casts\":"<<casts<<",\"heals\":"<<heals<<",\"impacts\":"<<impacts<<",\"summons\":"<<summons<<",\"mana_uses\":"<<manaUses<<",\"regeneration\":"<<regeneration<<",\"deaths\":"<<deaths<<",\"wizard_defeats\":"<<wizardDefeats<<",\"cleanups\":"<<cleanups<<",\"digest\":\""<<digest<<"\"}\n";
} catch(const std::exception &e){std::cerr<<e.what()<<'\n';return 1;}
