#define main unused_observation_fixture_main
#include "observation-snapshot-ownership-test.cpp"
#undef main
#include "../reconstruction/rendering/actor_visual_lifecycle.hpp"
#include "../reconstruction/creature-ai/owner_source_resolution.hpp"
#include <memory>
namespace {
std::set<unsigned> callbacks;
unsigned callbacksReleased = 0;
void actorFree(Arena &a, unsigned p) {
  if (callbacks.erase(p)) {
    record(a.get32(p), 0x5c5c40, "real callback destructor vtable");
    ++callbacksReleased;
  }
  a.free(p);
}
#ifdef MNM_BRAIN_REFERENCE
void __attribute__((cdecl)) resourceRelease(void *p) {
  actorFree(*current, address(p));
}
#endif
struct ActorBackend {
  Arena &a;
  unsigned r(unsigned p) {
    if (p == 0x5c7268)
      return 0x52cbe0;
    return a.get32(p);
  }
  void w(unsigned p, unsigned v) { a.set32(p, v); }
  void w8(unsigned p, unsigned v) { a.set8(p, v); }
  unsigned allocate(unsigned n) { return a.allocate(n); }
  void free(unsigned p) { actorFree(a, p); }
  void arrayFree(unsigned p) { a.freeBuffer(p); }
  unsigned call(unsigned entry, unsigned receiver, unsigned flag) {
    need(entry == 0x52cbe0 && flag == 1, "real callback deleting contract");
    a.set32(receiver, 0x5c5c40);
    free(receiver);
    return receiver;
  }
};

#ifdef MNM_BRAIN_REFERENCE
unsigned __attribute__((thiscall)) noLine(void *, int, int, int, int, int, int, void *) { return 0; }
#endif
unsigned liveActor(int slot) { need(slot>=0 && slot<20,"live slot"); return Scratch+0x10000+unsigned(slot)*0xe4b; }
void resourceReset(Arena &a,unsigned actor) {
#ifdef MNM_BRAIN_REFERENCE
  reinterpret_cast<void(__attribute__((thiscall)) *)(void *,unsigned)>(0x5060a0)(reinterpret_cast<void *>(actor),0);
#else
  ActorBackend backend{a}; mnm::reconstruction::ActorLifecycle<ActorBackend>(backend).reset(actor,0);
#endif
  unsigned callback=a.get32(actor+0x708);
  need(callbacks.insert(callback).second,"unique callback generation");
}
void resourceDestroy(Arena &a,unsigned actor) {
  (void)a;
#ifdef MNM_BRAIN_REFERENCE
  reinterpret_cast<void(__attribute__((thiscall)) *)(void *)>(0x505f70)(reinterpret_cast<void *>(actor));
#else
  ActorBackend backend{a}; mnm::reconstruction::ActorLifecycle<ActorBackend>(backend).destruct(actor);
#endif
}
std::vector<unsigned> leaves(Arena &a,unsigned root) {
  std::vector<unsigned> out,stack{root};
  while(!stack.empty()) {
    unsigned p=stack.back();stack.pop_back();
    if(a.get32(p+0x35)){out.push_back(p);continue;}
    for(unsigned e=a.get32(p+0x2d),b=a.get32(p+0x29);e!=b;) {e-=4;stack.push_back(a.get32(e));}
  }
  return out;
}
}
int main(int argc,char **argv) try {
#ifdef MNM_BRAIN_REFERENCE
  need(argc==2,"PE");map_image(load(argv[1]));
  need(mmap(reinterpret_cast<void *>(Heap),Size,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED_NOREPLACE,-1,0)!=MAP_FAILED,"heap");
  std::array<unsigned char,4096> thread{}; user_desc seg{};
  seg.entry_number=unsigned(-1);seg.base_addr=address(thread.data());seg.limit=0xfffff;seg.seg_32bit=seg.limit_in_pages=seg.useable=1;
  need(!syscall(SYS_set_thread_area,&seg),"FS");unsigned short selector=(seg.entry_number<<3)|3u;
  asm volatile("movw %0, %%fs" ::"rm"(selector));
  redirect(0x597850,reinterpret_cast<void *>(&ownedAllocate));
  redirect(0x597870,reinterpret_cast<void *>(&resourceRelease));
  redirect(0x5979b0,reinterpret_cast<void *>(&bufferFree));
  redirect(0x4f4ba0,reinterpret_cast<void *>(&noLine));
  const Bytes text(reinterpret_cast<unsigned char *>(0x401000),reinterpret_cast<unsigned char *>(0x5c0000));
  need(!mprotect(reinterpret_cast<void *>(0x401000),0x1bf000,PROT_READ|PROT_EXEC),"RX");
#else
  (void)argc;(void)argv;
#endif
  unsigned erased=0,inserted=0,continued=0,resolved=0,missing=0,stale=0,reacquired=0,disabled=0,boundary=0,freedBorrows=0;
  for(unsigned episode=0;episode<8;++episode) {
    Arena a;current=&a;
#ifdef MNM_BRAIN_REFERENCE
    std::memset(reinterpret_cast<void *>(Heap),0xa5,Size);
#endif
    a.zero(Brain,0x12100);a.zero(liveActor(0),20*0xe4b);a.zero(Config,0x5c9);a.zero(Task,8*512);a.zero(Nil,0xe68);a.set32(Nil+0xe64,1);
    a.set8(Brain+0x11fed,1);a.set32(Brain+0x11fee,~0u);a.set32(Config,1);a.set32(Config+0x589,0);
    std::array<bool,8> enabled{};
    std::array<std::array<std::uint8_t,8>,8> relation{};
    std::array<std::unique_ptr<m::OwnerKnowledgeCache<Arena>>,8> caches;
    for(unsigned o=0;o<8;++o) {
      a.set32(Task+o*512+0x27,o);
      enabled[o]=episode%3==0 || o==0 || o==3 || o==7;relation[o][o]=1;
      unsigned h=Heads+o*0xe68;a.zero(h,0xe68);a.set32(h,h);a.set32(h+4,Nil);a.set32(h+8,h);a.set32(Brain+0x11c06+48*o,h);
      caches[o]=std::make_unique<m::OwnerKnowledgeCache<Arena>>(a);
    }
    for(unsigned i=0;i<20;++i)resourceReset(a,liveActor(i));
#ifdef MNM_BRAIN_REFERENCE
    global(0x5faf4c,Nil);global(0x6def58,liveActor(0));global(0x6def5c,20);global(0x6df180,20);
    global(0x6c5494,32);global(0x6c5498,32);global(0x6c549c,8);global(0x5d854c,3);
    global(0x6a5f80+17*0x5c9+0x589,0);
#endif
    m::OwnerKnowledge knowledge;m::OwnerObservationSnapshots<Arena> store(a);
    std::array<std::map<int,Bytes>,8> expected;
    std::array<std::map<int,unsigned>,8> pointers;
    std::array<std::vector<unsigned>,8> clones;
    std::map<unsigned,std::pair<unsigned,unsigned>> priorLeaf;
    std::set<unsigned> delayed;
    for(unsigned step=0;step<112;++step) {
      int tick=step<24?int(step):step<40?int(1024+step-24):step<64?int(1049+step-40):step<88?int(1100+step-64):int(1200+step-88);
      if(episode%2 && step==35)enabled[3]=false;
      if(episode%2 && step==74)enabled[3]=true;
      relation[0][1]=step>=63 && step<67;
      std::vector<m::OwnerObservation> observations;
      for(unsigned i=0;i<20;++i) {
        unsigned p=liveActor(i);
        if(step==30 && i%3==0)resourceReset(a,p);
        m::OwnerObservation ob;auto &v=ob.actor;
        v.slot=i;v.owner=int(1+i%2);v.type=17;v.active=true;v.health=step==66 && i%5==0?0:100+i;v.region=-1;v.aging=0;
        v.position={int((i*7+episode)%32),int((i*11)%32),2};
        ob.finePosition={v.position.x*32,v.position.y*32,256};ob.speed=0;
        m::OwnerPlannerActor plan;plan.motionB63=0;ob.planning=plan;
        for(unsigned o=0;o<8;++o)ob.visible[o]=(step<16 || step>=80 || (i%4==0 && step%3==0)) && ((i+o+episode)%7!=0);
        // Preserve actual resource fields and aliases, author ordinary fields.
        for(auto row:{std::pair<unsigned,unsigned>{0,i},{4,1},{8,unsigned(v.position.x)},{12,unsigned(v.position.y)},{16,2},{20,unsigned(ob.finePosition.x)},{24,unsigned(ob.finePosition.y)},{28,256},{0xa8,17},{0xac,Config},{0xe4,v.health},{0x174,unsigned(v.owner)},{0xb63,0},{0xde7,~0u}})a.set32(p+row.first,row.second);
        for(unsigned o=0;o<8;++o){a.set32(p+0x24+4*o,ob.visible[o]);a.set32(p+0x48+4*o,0);}
        observations.push_back(ob);
      }
#ifdef MNM_BRAIN_REFERENCE
      for(unsigned o=0;o<8;++o){global(0x643a80+340*o,enabled[o]);global(0x643a80+340*o+4,0);for(unsigned j=0;j<8;++j)*reinterpret_cast<unsigned char *>(0x643a80+340*o+24+j)=relation[o][j];}
      a.set32(Brain+0x14,unsigned(tick));
#endif
      auto before=knowledge;
      m::markOwnerKnowledge(knowledge,tick);
      for(auto &c:caches)c->invalidate();
#ifdef MNM_BRAIN_REFERENCE
      reinterpret_cast<void(__attribute__((thiscall)) *)(void *,int)>(0x4181d0)(reinterpret_cast<void *>(Brain),1);
#endif
      m::updateOwnerKnowledge(knowledge,tick,{32,32,8},enabled,relation,observations);
#ifdef MNM_BRAIN_REFERENCE
      reinterpret_cast<void(__attribute__((thiscall)) *)(void *,int)>(0x4181d0)(reinterpret_cast<void *>(Brain),2);
      record(a.get32(Brain+0x11fee),unsigned(knowledge.owner),"owner continuation");
      unsigned cursor=a.get32(Brain+0x11ff2),h=Heads+unsigned(knowledge.owner)*0xe68;
      record(cursor==h?~0u:a.get32(cursor+12),unsigned(knowledge.cursor),"cursor continuation");
      record(a.get32(Brain+0x11da2),unsigned(knowledge.previousSweep),"previous sweep");
      record(a.get32(Brain+0x11da6),unsigned(knowledge.currentSweep),"current sweep");
      for(unsigned i=0;i<125;++i)record(a.get8(Brain+0x11ff6+i),knowledge.matched[i],"matched across slices");
#else
      store.reconcile(knowledge,tick,liveActor);
      for(int v:{knowledge.owner,knowledge.cursor,knowledge.previousSweep,knowledge.currentSweep})record(unsigned(v),unsigned(v),"cursor projection");
      for(bool v:knowledge.matched)record(v,v,"matched projection");
#endif
      continued+=knowledge.cursor!=-1;
      for(unsigned o=0;o<8;++o) {
        const auto &rows=knowledge.records[o];std::map<int,unsigned> nodes;
#ifdef MNM_BRAIN_REFERENCE
        unsigned h=Heads+o*0xe68,node=a.get32(h);
        while(node!=h){need(nodes.size()<125,"bounded RB walk");nodes.emplace(int(a.get32(node+12)),node);reinterpret_cast<void(__attribute__((thiscall)) *)(void *)>(0x42d0a0)(&node);}
        record(a.get32(Brain+0x11c0e + 48*o),rows.size(),"owner row count");record(a.get32(Brain+0x11d82+4*o),unsigned(knowledge.nextKey[o]),"owner next key");
#else
        nodes=store.records[o];record(nodes.size(),rows.size(),"owner row count");record(unsigned(knowledge.nextKey[o]),unsigned(knowledge.nextKey[o]),"owner next key");
#endif
        record(nodes.size(),rows.size(),"complete ordered rows");
        for(auto old:pointers[o])if(!nodes.count(old.first)){
          record(a.live(old.second),0,"erased value retired once");++erased;
        }
        std::map<int,unsigned> nextPointers;
        for(const auto &row:rows) {
          int key=row.memory.key;unsigned p=nodes.at(key),snapshot=p+24;nextPointers[key]=snapshot;
          record(a.get32(p+16),unsigned(row.memory.observedTick),"row seen");record(a.get32(p+20),unsigned(row.markedTick),"row marker");
          if(!pointers[o].count(key)){++inserted;if(step>=80)++reacquired;}
          else record(snapshot==pointers[o].at(key),1,"surviving record identity");
          if(tick-row.memory.observedTick==1024)++boundary;
          if(!enabled[o] && before.records[o].size())++disabled;
          if(row.memory.observedTick==tick) {
            Bytes bytes(0xe4b);unsigned actor=liveActor(row.memory.actor.slot);
            for(unsigned j=0;j<0xe4b;++j)bytes[j]=((j>=0x6d0&&j<0x6d8)||(j>=0x708&&j<0x70c)||(j>=0x8f9&&j<0x8fd))?0:a.get8(actor+j);
            expected[o][key]=std::move(bytes);
          }
          for(unsigned j=0;j<0xe4b;++j) {
            unsigned actual=a.get8(snapshot+j), want=expected[o].at(key)[j];
            // Check full pointer bytes against this execution's independently
            // owned actor; normalize allocator identities only in the digest.
            if((j>=0xcf3&&j<0xcf7)||(j>=0x8fd&&j<0x901))
              record(actual==want,1,"packed borrowed alias identity");
            else record(actual,want,"packed value after erase/refresh");
          }
        }
        pointers[o]=std::move(nextPointers);
        if(enabled[o] && (step==2 || step==3 || step==90)) {
          unsigned task=Task+o*512,clone;
#ifdef MNM_BRAIN_REFERENCE
          clone=reinterpret_cast<unsigned(__attribute__((thiscall)) *)(void *,void *,int,unsigned)>(0x413330)(reinterpret_cast<void *>(task),reinterpret_cast<void *>(Brain+0x11c02+o*48),tick,1);
#else
          unsigned root=caches[o]->prepare(1,knowledge,o,tick,{32,32,8},relation,{[](int,int,int &){return false;},[](auto,auto){return false;}},[&](int key){return store.snapshot(o,key);});
          clone=m::OwnerTaskSources<Arena>(a).clone(task,root);
#endif
          need(clone,"task clone");clones[o].push_back(clone);
          if(step==3)delayed.insert(clone);
          for(unsigned leaf:leaves(a,clone))priorLeaf[leaf]={a.get32(leaf+0x35),a.get8(leaf+0x25)};
        }
        for(unsigned root:clones[o]) {
          if(delayed.count(root) && step<78)continue;
          auto ls=leaves(a,root);bool expectedUsable=false;
          std::array<int,3> center{int(a.get32(root+0xd)),int(a.get32(root+0x11)),int(a.get32(root+0x15))};
          int liveCount=0;
          for(unsigned leaf:ls) {
            auto at=std::find_if(rows.begin(),rows.end(),[&](const auto &r){return r.memory.key==int(a.get32(leaf+9));});
            bool current=at!=rows.end()&&at->markedTick==tick;
            if(current) {
              bool hostile=a.get32(liveActor(at->memory.actor.slot)+0xe4)&&!relation[o][at->memory.actor.owner];expectedUsable|=hostile;
              if(hostile){++liveCount;for(unsigned axis=0;axis<3;++axis){int value=int(a.get32(liveActor(at->memory.actor.slot)+8+axis*4));center[axis]+=(value-center[axis])/liveCount;if(axis<2){center[axis]%=32;if(center[axis]<0)center[axis]+=32;}}}
            }else expectedUsable=true;
          }
#ifdef MNM_BRAIN_REFERENCE
          bool usable=(reinterpret_cast<unsigned(__attribute__((thiscall)) *)(void *,void *,int,unsigned)>(0x407c10)(reinterpret_cast<void *>(root),reinterpret_cast<void *>(Brain+0x11c02+48*o),tick,o)&255)!=0;
#else
          bool usable=m::resolveOwnerTaskSources(a,root,knowledge,o,tick,{32,32,8},relation,liveActor);
#endif
          record(usable,expectedUsable,"resolution usable");++resolved;
          for(unsigned axis=0;axis<3;++axis)record(a.get32(root+0xd+axis*4),unsigned(center[axis]),"resolved raw-coordinate center");
          for(unsigned leaf:ls) {
            auto at=std::find_if(rows.begin(),rows.end(),[&](const auto &r){return r.memory.key==int(a.get32(leaf+9));});
            bool current=at!=rows.end()&&at->markedTick==tick;
            unsigned want=current?liveActor(at->memory.actor.slot):priorLeaf.at(leaf).first;
            record(a.get32(leaf+0x35)==want,1,"current rebind or stale borrow preserved");
            record(a.get8(leaf+0x25)==priorLeaf.at(leaf).second,1,"constructor pointer tag preserved");
            record(a.get32(leaf+0x89),current && a.get32(liveActor(at->memory.actor.slot)+0xe4) && !relation[o][at->memory.actor.owner]?3:0,"source activity after resolution");
            if(!current && want>=Heap+0x100000 && !a.live(want))++freedBorrows;
            if(at==rows.end())++missing;else if(!current)++stale;
            priorLeaf[leaf].first=want;
          }
        }
      }
    }
    for(unsigned o=0;o<8;++o) {
      unsigned task=Task+o*512;
#ifdef MNM_BRAIN_REFERENCE
      reinterpret_cast<void(__attribute__((thiscall)) *)(void *)>(0x40a340)(reinterpret_cast<void *>(task));
      reinterpret_cast<void(__attribute__((thiscall)) *)(void *)>(0x42f2e0)(reinterpret_cast<void *>(Brain+0x11c02+48*o));
      a.free(a.get32(Brain+0x11c26+48*o));
#else
      for(unsigned at=a.get32(task+0x3b);at!=a.get32(task+0x3f);at+=4){unsigned p=a.get32(at);a.destroySource(p);a.free(p);}
#endif
      a.free(a.get32(task+0x3b));caches[o]->close();
    }
#ifndef MNM_BRAIN_REFERENCE
    store.clear();
#endif
    for(unsigned i=0;i<20;++i)resourceDestroy(a,liveActor(i));
    a.guards();for(auto b:a.blocks)record(b.second.live,0,"all allocations retired");
  }
#ifdef MNM_BRAIN_REFERENCE
  need(!std::memcmp(text.data(),reinterpret_cast<void *>(0x401000),text.size()),"original text unchanged");
#endif
  need(callbacks.empty()&&erased&&continued&&resolved&&missing&&stale&&reacquired&&disabled&&boundary&&freedBorrows,"lifecycle witnesses");
  std::cout<<"{\"success\":true,\"checks\":"<<checks<<",\"cases\":8,\"slices\":896,\"insertions\":"<<inserted<<",\"expiries\":"<<erased<<",\"cursor_continuations\":"<<continued<<",\"resolutions\":"<<resolved<<",\"missing_leaves\":"<<missing<<",\"stale_leaves\":"<<stale<<",\"reacquisitions\":"<<reacquired<<",\"disabled_rows\":"<<disabled<<",\"age1024_rows\":"<<boundary<<",\"freed_borrow_resolutions\":"<<freedBorrows<<",\"callbacks_released\":"<<callbacksReleased<<",\"digest\":\""<<digest<<"\"}\n";
  return 0;
}catch(const std::exception &e){std::cerr<<e.what()<<'\n';return 1;}
