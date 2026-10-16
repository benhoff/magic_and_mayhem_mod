#include "../reconstruction/creature-ai/owner_knowledge.hpp"
#ifdef MNM_PAIRED_REFERENCE
#include "regional-danger-original.hpp"
#else
#include "paired-encounter-projection.hpp"
#include <iostream>
#endif
namespace {
namespace m = mnm::command_session::model;
void need(bool ok, const std::string &s) { if (!ok) throw std::runtime_error(s); }
}
int main(int argc, char **argv) try {
#ifdef MNM_PAIRED_REFERENCE
  need(argc == 3, "PE INSTALL");
  const auto pe = read(argv[1]); map_image(pe); endpoints(); sealText(pe); initializeSearch();
  const auto install = argv[2];
#else
  need(argc == 2, "INSTALL"); const auto install = argv[1];
#endif
  auto p = mnm::encounter::decodeInputs(mnm::encounter::loadInputs(install, {0,0,32,32}, 0));
#ifdef MNM_PAIRED_REFERENCE
  Original base(p); setup(base, p.initial, 17);
#endif
  unsigned checks=0, ticks=0, insertions=0, expiries=0, hidden=0, dead=0, alliedRetained=0, reassociations=0;
  std::uint64_t digest=1469598103934665603ull;
  auto record = [&](unsigned a, unsigned b, const std::string &label) {
    need(a==b,label+" tick="+std::to_string(ticks)+" native="+std::to_string(a)+" original="+std::to_string(b));
    ++checks; digest=(digest^a)*1099511628211ull;
  };
#ifdef MNM_PAIRED_REFERENCE
#define KNOW(value,ptr,offset,size,label) record(unsigned(value), get(ptr,offset,size), label)
#else
#define KNOW(value,ptr,offset,size,label) record(unsigned(value), unsigned(value), label)
#endif
  for (unsigned scenario=0;scenario<8;++scenario) {
    m::OwnerKnowledge state;
    m::RegionalDangerInputs inputs; inputs.dimensions={32,32,23};
    for(unsigned owner=0;owner<8;++owner) {
      inputs.enabled[owner]=owner<(scenario%3+1);
      inputs.allied[owner][owner]=1;
    }
    std::vector<m::OwnerObservation> actors;
    for(unsigned i=0;i<12+scenario*2;++i) {
      m::OwnerObservation a;
      a.actor.slot=int(i); a.actor.owner=int(i%3); a.actor.type=17+i%2;
      a.actor.active=true; a.actor.health=50+i; a.actor.threat=1000+100*int(i);
      a.actor.region=int(i%8); a.actor.aging=24; a.speed=24;
      a.actor.position={int(i*2%32),int(i*3%32),2};
      a.finePosition={a.actor.position.x*32,a.actor.position.y*32,256};
      a.visible.fill(1); actors.push_back(a);
    }
#ifdef MNM_PAIRED_REFERENCE
    DangerOriginal old; old.inputs(inputs); old.memoryInputs(inputs);
    put(old.context.data(),0x11fed,1,1);
    put(old.context.data(),0x11fee,unsigned(-1));
    auto snapshot = [&] {
      for(unsigned owner=0;owner<8;++owner) {
        auto *head=reinterpret_cast<void *>(get(old.context.data(),0x11c06+48*owner));
        unsigned cursor=get(head);
        for(const auto &r:state.records[owner]) {
          need(cursor!=address(head),"missing original memory");
          const auto *n=reinterpret_cast<const void *>(cursor);
          KNOW(r.memory.key,n,12,4,"memory key"); KNOW(r.memory.observedTick,n,16,4,"seen");
          KNOW(r.markedTick,n,20,4,"mark");
          const auto *a=static_cast<const char *>(n)+24;
          KNOW(r.memory.actor.slot,a,0,4,"snapshot slot");
          KNOW(r.memory.actor.owner,a,0x174,4,"snapshot owner");
          KNOW(r.memory.actor.type,a,0xa8,4,"snapshot type");
          KNOW(r.memory.actor.health,a,0xe4,4,"snapshot health");
          KNOW(r.memory.actor.threat,a,0x91e,4,"snapshot threat");
          KNOW(r.memory.actor.region,a,0xde7,4,"snapshot region");
          KNOW(r.memory.actor.position.x,a,8,4,"snapshot x");
          KNOW(r.memory.actor.position.y,a,12,4,"snapshot y");
          KNOW(r.finePosition.x,a,20,4,"fine x"); KNOW(r.finePosition.y,a,24,4,"fine y"); KNOW(r.finePosition.z,a,28,4,"fine z");
          call(0x42d0a0,&cursor);
        }
        need(cursor==address(head),"extra original memory");
        KNOW(state.records[owner].size(),old.context.data(),0x11c0e + 48*owner,4,"memory count");
        KNOW(state.nextKey[owner],old.context.data(),0x11d82+4*owner,4,"next key");
      }
      KNOW(state.previousSweep,old.context.data(),0x11da2,4,"previous sweep");
      KNOW(state.currentSweep,old.context.data(),0x11da6,4,"current sweep");
      KNOW(state.reset,old.context.data(),0x11fed,1,"reset");
      KNOW(state.owner,old.context.data(),0x11fee,4,"owner");
      if(!state.reset) {
        const auto raw=get(old.context.data(),0x11ff2),head=get(old.context.data(),0x11c06+48*unsigned(state.owner));
        record(unsigned(state.cursor),raw==head?unsigned(-1):get(reinterpret_cast<void *>(raw),12),"cursor");
      }
      for(unsigned i=0;i<125;++i)KNOW(state.matched[i],old.context.data(),0x11ff6+i,1,"matched");
    };
#else
    auto snapshot = [&] {
      for(unsigned owner=0;owner<8;++owner) {
        for(const auto &r:state.records[owner]) {
          for(int v:{r.memory.key,r.memory.observedTick,r.markedTick,r.memory.actor.slot,r.memory.actor.owner,int(r.memory.actor.type),int(r.memory.actor.health),r.memory.actor.threat,r.memory.actor.region,r.memory.actor.position.x,r.memory.actor.position.y,r.finePosition.x,r.finePosition.y,r.finePosition.z})record(unsigned(v),unsigned(v),"native");
        }
        record(unsigned(state.records[owner].size()),unsigned(state.records[owner].size()),"count");
        record(unsigned(state.nextKey[owner]),unsigned(state.nextKey[owner]),"key");
      }
      for(int v:{state.previousSweep,state.currentSweep,int(state.reset),state.owner})record(unsigned(v),unsigned(v),"state");
      if(!state.reset)record(unsigned(state.cursor),unsigned(state.cursor),"cursor");
      for(bool v:state.matched)record(v,v,"matched");
    };
#endif
    for(unsigned tick=0;tick<2304;++tick) {
      ++ticks;
      for(unsigned i=0;i<actors.size();++i) {
        auto &a=actors[i];
        a.visible.fill((tick<96||tick>=1344)?1:0);
        // One visible actor crosses the toroidal seam and later changes owner/type.
        if(i==1) {
          a.visible.fill(1); a.finePosition.x=int((tick*3+1000)%1024);
          a.actor.position.x=a.finePosition.x/32;
          a.actor.owner=tick>=1536?2:1;
          a.actor.type=tick>=1792?17:18;
          a.actor.health=tick>=2048?0:51;
        }
        if(i==3 && (scenario&1)) a.actor.active=tick<64;
        // Equal type/owner, swapped positions: memories may reassociate by proximity.
        if((scenario&2)&&tick==48&&i==4)std::swap(a.finePosition,actors[10].finePosition);
      }
      if((scenario&2)&&tick==40)actors[10].finePosition=actors[4].finePosition;
      if(scenario==6)inputs.enabled.fill(!(tick>=512&&tick<576));
      inputs.allied[0][1]=tick>=160&&tick<1920;
      if(scenario==7)inputs.enabled[1]=tick<128||tick>=1408;
      std::size_t before=0;for(const auto &o:state.records)before+=o.size();
      m::markOwnerKnowledge(state,int(tick));
#ifdef MNM_PAIRED_REFERENCE
      inputs.tick=int(tick); old.inputs(inputs,false);
      reinterpret_cast<Value>(0x4181d0)(old.context.data(),1);
#endif
      snapshot();
      m::updateOwnerKnowledge(state,int(tick),inputs.dimensions,inputs.enabled,inputs.allied,actors);
#ifdef MNM_PAIRED_REFERENCE
      std::fill(old.pool.begin(),old.pool.end(),0);
      for(const auto &a:actors) {
        auto *raw=old.pool.data()+unsigned(a.actor.slot)*0xe4b;
        old.packActor(raw,a.actor);
        put(raw,20,a.finePosition.x);put(raw,24,a.finePosition.y);put(raw,28,a.finePosition.z);
        for(unsigned owner=0;owner<8;++owner)put(raw,0x24+4*owner,a.visible[owner]);
        global(0x6a5f80+a.actor.type*0x5c9+0x589,a.speed);
      }
      reinterpret_cast<Value>(0x4181d0)(old.context.data(),2);
#endif
      snapshot(); m::validateOwnerKnowledge(state,unsigned(actors.size()));
      std::size_t after=0;
      for(unsigned owner=0;owner<8;++owner)for(const auto &r:state.records[owner]) {
        ++after;
        const auto &live=actors.at(unsigned(r.memory.actor.slot));
        hidden+=!live.visible[owner]&&r.memory.observedTick<int(tick);
        dead+=!r.memory.actor.health;
        alliedRetained+=inputs.allied[owner][unsigned(r.memory.actor.owner)]!=0;
        reassociations+=r.memory.key!=r.memory.actor.slot;
      }
      if(after>before)insertions+=unsigned(after-before);else expiries+=unsigned(before-after);
    }
#ifdef MNM_PAIRED_REFERENCE
    for(unsigned owner=0;owner<8;++owner)call(0x42f2e0,old.context.data()+0x11c02+48*owner);
    checkText();
#endif
  }
#ifdef MNM_PAIRED_REFERENCE
  destroySearch();
#endif
  need(insertions&&expiries&&hidden&&dead&&alliedRetained&&reassociations,"missing lifecycle witness");
  std::cout<<"{\"success\":true,\"checks\":"<<checks<<",\"ticks\":"<<ticks<<",\"insertions\":"<<insertions<<",\"expiries\":"<<expiries<<",\"hidden\":"<<hidden<<",\"dead\":"<<dead<<",\"allied_retained\":"<<alliedRetained<<",\"anonymous_records\":"<<reassociations<<",\"digest\":\""<<digest<<"\"}\n";
}catch(const std::exception &e){std::cerr<<e.what()<<'\n';return 1;}
