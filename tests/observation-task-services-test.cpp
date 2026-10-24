#define main unused_snapshot_ownership_main
#include "observation-snapshot-ownership-test.cpp"
#undef main
#include "../reconstruction/creature-ai/owner_observation_tasks.hpp"
namespace {
constexpr unsigned Observer = Pool + 0x2000, Other = Pool + 0x4000;
}
int main(int argc, char **argv) try {
#ifdef MNM_BRAIN_REFERENCE
  need(argc == 2, "PE");
  map_image(load(argv[1]));
  need(mmap(reinterpret_cast<void *>(Heap), Size, PROT_READ | PROT_WRITE,
            MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0) != MAP_FAILED,
       "sighting heap mapping");
  std::array<unsigned char,4096> thread{};
  user_desc seg{}; seg.entry_number=unsigned(-1);seg.base_addr=address(thread.data());
  seg.limit=0xfffff;seg.seg_32bit=seg.limit_in_pages=seg.useable=1;
  need(!syscall(SYS_set_thread_area,&seg),"FS");
  unsigned short selector=(seg.entry_number<<3)|3u;
  asm volatile("movw %0, %%fs" :: "rm"(selector));
  redirect(0x597850,reinterpret_cast<void *>(&ownedAllocate));
  redirect(0x597870,reinterpret_cast<void *>(&ownedFree));
  redirect(0x5979b0,reinterpret_cast<void *>(&bufferFree));
  const Bytes text(reinterpret_cast<unsigned char *>(0x401000),reinterpret_cast<unsigned char *>(0x5c0000));
  need(!mprotect(reinterpret_cast<void *>(0x401000),0x1bf000,PROT_READ|PROT_EXEC),"sighting RX");
#else
  (void)argc;(void)argv;
#endif
  unsigned credits=0,empty=0,insertions=0,refreshes=0,expiries=0,wraps=0,slices=0;
  // Twelve independent source/recipient shapes, both families, wizard and
  // ordinary CFG kinds and health below/at/above the sighting cap.
  for(unsigned episode=0;episode<384;++episode) {
    Arena a;current=&a;
#ifdef MNM_BRAIN_REFERENCE
    std::memset(reinterpret_cast<void *>(Heap),0xa5,Size);
#endif
    a.mode=0;
    a.zero(Brain,0x12100);a.zero(Pool,0x6000);a.zero(Config,0x5c9);
    a.zero(Task,512);a.zero(Nil,0xe68);a.set32(Nil+0xe64,1);
    const unsigned family=episode/12%2,shape=episode%12;
    a.set32(Brain,family);a.set8(Brain+0x11fed,1);a.set32(Brain+0x11fee,~0u);
    for(unsigned owner=0;owner<8;++owner){unsigned h=Heads+owner*0xe68;
      a.zero(h,0xe68);a.set32(h,h);a.set32(h+4,Nil);a.set32(h+8,h);
      a.set32(Brain+0x11c06+48*owner,h);}
    m::ContributionProfiles profiles;
    constexpr unsigned kinds[]={1,0,24,25,26,8,20,23};
    const unsigned type=episode/24%28,kind=kinds[episode/24%8];
    const unsigned healths[]={1,2,9,10,11,127,1000,7};
    const unsigned health=healths[episode/48%8];
    for(unsigned i=0;i<28;++i){auto &p=profiles[i];p.kind=i==type?kind:1;
      p.capacity=1+(episode+i)%47;p.flag58=(episode+i)%2;
      for(unsigned j=0;j<28;++j){p.first[j]=1+(episode*7+i+j)%103;
        p.second[j]=1+(episode+i*3+j)%73;}}
    a.set32(Config,kind);a.set32(Config+0x589,24);
    m::OwnerTaskSources<Arena> sources(a);
    unsigned root=sources.anonymous(Task), selected=sources.anonymous(Task);
    sources.attach(root,selected);
    for(unsigned i=0;i<(shape==3?0:3);++i){unsigned leaf=sources.actor(Task,Other+i*16);
      a.set32(leaf+0x89,shape==4?2:(shape==5?0xffffffffu:3+i));
      sources.attach(selected,leaf);}
    // Task is a preinstalled recipient, not a supplied recipient-ID stub.
    a.set32(root+5,shape==6?0:Task);
    a.set32(selected+5,0); // ancestry must reach terminal parent.
    a.set32(Observer+0x678+4*family,shape==2?0:selected);
    a.set32(Observer+0x678+4*(1-family),0);
    if(shape==7) a.set32(Observer+0x678+4*family,root);
    if(shape==8){a.set32(Observer+0x678+4*family,a.get32(a.get32(selected+0x29)));}
    if(shape==9){a.set32(Observer+0x678+4*family,0);
      a.set32(Observer+0x678+4*(1-family),selected);}
    // Borrowed observer actors' health/activity are deliberately zero: only
    // source activity threshold gates traversal here.
    a.set32(Other+0xe4,0);a.set32(Other+0x174,0);
    unsigned initial=shape==11?0xfffffff0u:episode*19;
    a.set32(Task+0x2f,initial);
#ifdef MNM_BRAIN_REFERENCE
    global(0x5faf4c,Nil);global(0x6def58,Pool);global(0x6def5c,1);global(0x6df180,1);
    global(0x6c5494,32);global(0x6c5498,32);global(0x6c549c,8);global(0x5d854c,0);
    for(unsigned i=0;i<28;++i){unsigned p=0x6a5f80+i*0x5c9;const auto &v=profiles[i];
      global(p,v.kind);global(p+0x18,v.capacity);global(p+0x58,v.flag58);
      global(p+0x589,24);
      for(unsigned j=0;j<28;++j){global(p+0x280+j*4,v.first[j]);global(p+0x1a0+j*4,v.second[j]);}}
    for(unsigned owner=0;owner<8;++owner){global(0x643a80+340*owner,owner==0);
      global(0x643a80+340*owner+4,0);
      for(unsigned j=0;j<8;++j)*reinterpret_cast<unsigned char *>(0x643a80+340*owner+24+j)=owner==j;}
#endif
    m::OwnerKnowledge knowledge;m::OwnerObservationSnapshots<Arena> store(a);
    m::OwnerKnowledgeCache<Arena> cache(a);std::array<bool,8> enabled{};enabled[0]=true;
    std::array<std::array<std::uint8_t,8>,8> relations{};for(unsigned i=0;i<8;++i)relations[i][i]=1;
    unsigned expected=initial,previousNext=0;
    for(unsigned step=0;step<9;++step){
      const int ticks[]={0,1,2,1026,1027,1030,1031,2057,2060};int tick=ticks[step];
      bool visible=step<3||step==5||step==6||step==8;
      m::OwnerObservation o;auto &v=o.actor;v.slot=0;v.active=true;v.owner=1;v.type=int(type);
      v.health=int(health);v.position={2,3,2};v.region=-1;v.aging=24;
      o.finePosition={64,96,256};o.speed=24;o.visible.fill(visible);
      m::OwnerPlannerActor planning;planning.motionB63=24;o.planning=planning;
      a.zero(Pool,0xe4b);
      for(auto row:{std::pair<unsigned,unsigned>{4,1},{8,2},{12,3},{16,2},{20,64},{24,96},{28,256},
         {0xa8,type},{0xac,Config},{0xe4,health},{0x174,1},{0xb63,24},{0xde7,~0u}})a.set32(Pool+row.first,row.second);
      for(unsigned i=0;i<8;++i)a.set32(Pool+0x24+i*4,visible);
      a.set32(Pool+0x48,shape==1?0:Observer);
      m::markOwnerKnowledge(knowledge,tick);
      unsigned oldCount=unsigned(knowledge.records[0].size());
#ifdef MNM_BRAIN_REFERENCE
      a.set32(Brain+0x14,unsigned(tick));
      using Update=void(__attribute__((thiscall)) *)(void *,int);
      reinterpret_cast<Update>(0x4181d0)(reinterpret_cast<void *>(Brain),1);
      m::updateOwnerKnowledge(knowledge,tick,{32,32,8},enabled,relations,{o});
      reinterpret_cast<Update>(0x4181d0)(reinterpret_cast<void *>(Brain),2);
      record(a.get32(Brain+0x11d82),unsigned(knowledge.nextKey[0]),"insertion key");
      record(a.get32(Brain+0x11c0e),unsigned(knowledge.records[0].size()),"observation count");
      record(*reinterpret_cast<unsigned *>(0x5d854c),0,"mode restored before return");
#else
      m::updateOwnerKnowledgeWithTasks(a,knowledge,store,cache,tick,{32,32,8},enabled,relations,{o},family,profiles,[](int){return Pool;});
      record(unsigned(knowledge.nextKey[0]),unsigned(knowledge.nextKey[0]),"insertion key");
      record(unsigned(store.records[0].size()),unsigned(knowledge.records[0].size()),"observation count");
      record(a.mode,0,"mode restored before return");
#endif
      unsigned added=unsigned(knowledge.nextKey[0])-previousNext;
      insertions+=added;expiries+=oldCount+added-unsigned(knowledge.records[0].size());
      if(!added&&visible&&!knowledge.records[0].empty())++refreshes;
      bool eligible=shape!=1&&shape!=2&&shape!=3&&shape!=4&&shape!=5&&shape!=6&&shape!=9;
      for(unsigned n=0;n<added;++n){if(eligible){unsigned cap=kind==0||kind==24||kind==25||kind==26?10:1;
          unsigned amount=m::contributionAccount(profiles,type,int(health),int(std::min(health,cap)),0,0,0);
          unsigned before=expected;expected+=amount;wraps+=expected<before;++credits;
        }else ++empty;}
      record(a.get32(Task+0x2f),expected,"task credit including wrap and refresh silence");
      // Full-drain releases every cursor it began; no unrelated source slot is
      // left busy even when the first actor is absent/ineligible.
      for(unsigned p=a.get32(Task+0x3b);p!=a.get32(Task+0x3f);p+=4){unsigned node=a.get32(p);
        for(unsigned i=0;i<8;++i)record(a.get8(node+0x41+i),1,"traversal drain free mask");}
      previousNext=unsigned(knowledge.nextKey[0]);++slices;
    }
#ifdef MNM_BRAIN_REFERENCE
    reinterpret_cast<void(__attribute__((thiscall)) *)(void *)>(0x42f2e0)(reinterpret_cast<void *>(Brain+0x11c02));
    a.free(a.get32(Brain+0x11c26));
#else
    store.clear();
#endif
    // The comparison deliberately admits negative activity for notification
    // eligibility. Before normal forward owner retirement, the fixture makes
    // every leaf eligible for destructor detachment; otherwise the recovered
    // destroy path skips negative children and leaves their parent borrowed.
    // This is explicit fixture disposal, not repaired engine behavior.
    for(unsigned p=a.get32(Task+0x3b);p!=a.get32(Task+0x3f);p+=4)
      a.set32(a.get32(p)+0x89,3);
    releaseTask(a);a.free(a.get32(Task+0x3b));cache.close();a.guards();
    for(auto b:a.blocks)record(b.second.live,0,"all owned storage released");
  }
#ifdef MNM_BRAIN_REFERENCE
  need(!std::memcmp(text.data(),reinterpret_cast<void *>(0x401000),text.size()),"original text unchanged");
#endif
  need(credits&&empty&&insertions&&refreshes&&expiries&&wraps,"composition witnesses");
  std::cout<<"{\"success\":true,\"checks\":"<<checks<<",\"cases\":384,\"slices\":"<<slices
    <<",\"insertions\":"<<insertions<<",\"refreshes\":"<<refreshes<<",\"expiries\":"<<expiries
    <<",\"task_credits\":"<<credits<<",\"no_credit_insertions\":"<<empty<<",\"wrapped_credits\":"<<wraps
    <<",\"digest\":\""<<digest<<"\"}\n";
  return 0;
}catch(const std::exception &e){std::cerr<<e.what()<<'\n';return 1;}
