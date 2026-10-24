#pragma push_macro("main")
#undef main
#define main unused_resource_refresh_main
#include "snapshot-resource-refresh-test.cpp"
#undef main
#pragma pop_macro("main")
#include "../reconstruction/rendering/member_callbacks.hpp"

namespace {
using Callback = mnm::reconstruction::actor_services::MemberCallbacks<Arena>;
unsigned subject=0, deleting=0, receiverSeen=0, argumentSeen=0, baseSeen=0;
bool releaseDuringCall=false;
unsigned callbackCall(Arena&,unsigned,unsigned,unsigned,bool=false);
unsigned targetBody(unsigned self,unsigned argument){
  receiverSeen=self;argumentSeen=argument;
  if(releaseDuringCall)callbackCall(*current,deleting,subject,1,true);
  return (self^argument)+0x35u;
}
#ifdef MNM_BRAIN_REFERENCE
unsigned __attribute__((thiscall)) callbackTarget(void* self,unsigned argument){
  return targetBody(address(self),argument);
}
void __attribute__((cdecl)) borrowedRelease(void* raw){
  unsigned p=address(raw);
  if(p&&p==subject)baseSeen=current->get32(p);
  current->free(p);
}
#endif
struct CallbackMemory {
  Arena& a;
  unsigned get32(unsigned p){return a.get32(p);}
  void set32(unsigned p,unsigned v){a.set32(p,v);}
  unsigned invoke(unsigned entry,unsigned self,std::initializer_list<unsigned> args){
    if(entry==0x00f00d){need(args.size()==1,"member target arity");return targetBody(self,*args.begin());}
    need(entry==0x597870&&self==0&&args.size()==1,"callback free endpoint");
    unsigned p=*args.begin();if(p==subject)baseSeen=a.get32(p);a.free(p);return 0;
  }
};
unsigned callbackCall(Arena& a,unsigned entry,unsigned self,unsigned argument,bool destroy){
#ifdef MNM_BRAIN_REFERENCE
  (void)a;(void)destroy;
  return reinterpret_cast<unsigned(__attribute__((thiscall)) *)(void*,unsigned)>(entry)(reinterpret_cast<void*>(self),argument);
#else
  (void)destroy;CallbackMemory memory{a};
  auto result=mnm::reconstruction::actor_services::MemberCallbacks<CallbackMemory>(memory).tryRun(entry,self,{argument});
  need(result.has_value(),"known callback class");return *result;
#endif
}
void callbacksMatrix(Arena& a){
  for(auto c:Callback::classes){
    for(unsigned i=0;i<6;++i){
      subject=a.allocate(16);deleting=c.deleting;baseSeen=0;
      a.set32(subject,c.table);
#ifdef MNM_BRAIN_REFERENCE
      a.set32(subject+4,address(reinterpret_cast<void*>(&callbackTarget)));
#else
      a.set32(subject+4,0x00f00d);
#endif
      unsigned adjustment=i%3==0?0:i%3==1?12:~31u;
      unsigned owner=i==4?0xfffffff8u:Pool+32;
      a.set32(subject+8,adjustment);a.set32(subject+12,owner);
      // Invocation through a stripped snapshot's borrowed cf3 still targets
      // the original owner, including nonzero member-pointer adjustment.
      a.zero(Scratch,0xe4b);a.set32(Scratch+0xcf3,subject);
      unsigned argument=0x80000000u+i;releaseDuringCall=i==5;
      unsigned alias=a.get32(Scratch+0xcf3);
      record(callbackCall(a,c.invoke,alias,argument),((owner+adjustment)^argument)+0x35u,"member callback result");
      record(receiverSeen,owner+adjustment,"member callback adjusted receiver");
      record(argumentSeen,argument,"member callback argument");
      if(releaseDuringCall){record(a.live(alias),0,"target may delete callback during invocation");}
      else{
        unsigned flag=i==4?0x100u:i;
        record(callbackCall(a,c.deleting,alias,flag,true),alias,"deleting wrapper return");
        record(a.live(alias),!(flag&1),"only low deleting flag bit frees");
        if(a.live(alias)){record(a.get32(alias),0x5c5c40,"nondestroying flag changes vtable");a.free(alias);}
      }
      if(i==1||i==3||i==5)record(baseSeen,0x5c5c40,"base vtable precedes free");
      record(a.get32(Scratch+0xcf3),alias,"deletion does not repair borrowed snapshot alias");
      // Only the checked native memory policy is exercised for a freed thunk.
      // Calling the original through that pointer would itself be undefined.
#ifndef MNM_BRAIN_REFERENCE
      CallbackMemory memory{a};bool refused=false;
      try{mnm::reconstruction::actor_services::MemberCallbacks<CallbackMemory>(memory).tryRun(c.invoke,alias,{0});}
      catch(const std::exception&){refused=true;}
      need(refused,"freed callback refused before dispatch");
#endif
    }
  }
  subject=0;releaseDuringCall=false;
}
void refreshSource(Arena& a,m::OwnerObservationSnapshots<Arena>& store,unsigned root,unsigned tick,bool expected=true){
#ifdef MNM_BRAIN_REFERENCE
  (void)store;
  bool usable=reinterpret_cast<unsigned char(__attribute__((thiscall)) *)(void*,void*,unsigned,unsigned)>(0x407c10)(reinterpret_cast<void*>(root),reinterpret_cast<void*>(Brain+0x11c02),tick,0)!=0;
#else
  bool usable=m::OwnerTaskSources<Arena>(a).refresh(root,tick,0,32,32,
    [&](unsigned key){auto it=store.records[0].find(int(key));return it==store.records[0].end()?0:it->second;},
    [](unsigned slot){need(slot==0,"current slot");return Pool;},
    [](unsigned owner,unsigned other){need(owner==0&&other<8,"relation");return owner==other;});
#endif
  record(usable,expected,"source refresh admission");
  record(a.get32(0x5d854c),3,"refresh restores source mode");
  // Compare complete live source bytes through the cross-execution digest,
  // including all eight occupied/free traversal cursors after each refresh.
  for(unsigned at=a.get32(Task+0x3b);at!=a.get32(Task+0x3f);at+=4)
    for(unsigned i=0;i<0x8d;++i)
      digest=(digest^a.get8(a.get32(at)+i))*1099511628211ull;
}
unsigned drainSource(Arena& a,unsigned root,unsigned mode){
#ifdef MNM_BRAIN_REFERENCE
  unsigned saved=*reinterpret_cast<unsigned*>(0x5d854c);
  global(0x5d854c,mode);
  unsigned cursor=reinterpret_cast<unsigned(__attribute__((thiscall)) *)(void*)>(0x407920)(reinterpret_cast<void*>(root));
  auto next=reinterpret_cast<unsigned(__attribute__((thiscall)) *)(void*,unsigned)>(0x4079f0);
  unsigned first=next(reinterpret_cast<void*>(root),cursor);
  while(next(reinterpret_cast<void*>(root),cursor)){}
  global(0x5d854c,saved);(void)a;return first;
#else
  return m::OwnerTaskSources<Arena>(a).drain(root,mode);
#endif
}
void staleConsumers(Arena& a){
  a.zero(Brain,0x12100);a.zero(Pool,0xe4b);a.zero(Task,512);a.zero(Nil,0xe68);a.set32(Nil+0xe64,1);
  for(unsigned owner=0;owner<8;++owner){unsigned h=Heads+owner*0xe68;a.zero(h,0xe68);a.set32(h,h);a.set32(h+4,Nil);a.set32(h+8,h);a.set32(Brain+0x11c06+48*owner,h);}
#ifdef MNM_BRAIN_REFERENCE
  global(0x5faf4c,Nil);global(0x6def58,Pool);global(0x6def5c,1);global(0x6c5494,32);global(0x6c5498,32);global(0x5d854c,3);
  for(unsigned j=0;j<8;++j)*reinterpret_cast<unsigned char*>(0x643a80+24+j)=j==0;
#endif
  m::OwnerKnowledge knowledge;m::OwnerObservationSnapshots<Arena> store(a);m::OwnerTaskSources<Arena> sources(a);
  // Use a genuinely owned snapshot, then destroy it while the task clone lives.
  std::array<bool,8> enabled{};enabled[0]=true;
  std::array<std::array<std::uint8_t,8>,8> relation{};for(unsigned i=0;i<8;++i)relation[i][i]=1;
  m::OwnerObservation observation;observation.actor.slot=0;observation.actor.active=true;observation.actor.owner=1;observation.actor.type=17;observation.actor.health=100;observation.actor.position={2,3,2};observation.actor.region=-1;observation.actor.aging=24;observation.visible.fill(true);observation.speed=24;observation.finePosition={64,96,256};observation.planning=m::OwnerPlannerActor{};
  m::markOwnerKnowledge(knowledge,1);m::updateOwnerKnowledge(knowledge,1,{32,32,8},enabled,relation,{observation});
  a.cursor=(a.cursor+4095)&~4095u;
  store.reconcile(knowledge,1,[](int){return Pool;});
  a.cursor=(a.cursor+4095)&~4095u;
  int key=knowledge.records[0].front().memory.key;
  unsigned owned=store.snapshot(0,key),node=store.records[0].at(key);
  a.set32(node+20,1);a.set32(owned,0);
  // Install a one-node tree using its real PE node ABI for original lookup.
  a.set32(Heads,node);a.set32(Heads+4,node);a.set32(Heads+8,node);a.set32(node,Nil);a.set32(node+4,Heads);a.set32(node+8,Nil);a.set32(node+0xe64,1);
  unsigned root=sources.anonymous(Task),group=sources.anonymous(Task),leaf=sources.actor(Task,owned);
  sources.attach(root,group);sources.attach(group,leaf);a.set32(leaf+9,unsigned(key));
  unsigned tag=a.get8(leaf+0x25);
  a.set32(root+0xd,2);a.set32(root+0x11,3);a.set32(root+0x15,2);
  // Old marker:35 remains a valid snapshot and is never inspected.
  refreshSource(a,store,root,2);record(a.get32(leaf+0x89),0,"old marker clears activity");record(a.get32(leaf+0x35),owned,"old marker retains snapshot alias");
  // Missing record after real snapshot destruction. Poisoned backing catches
  // native reads; original PROT_NONE makes any read of the dangling alias fault.
#ifdef MNM_BRAIN_REFERENCE
  a.set32(Brain+0x11c0e,1);
  reinterpret_cast<void(__attribute__((thiscall)) *)(void*)>(0x42f2e0)(reinterpret_cast<void*>(Brain+0x11c02));
  store.records[0].clear();
  need(!mprotect(reinterpret_cast<void*>(node),4096,PROT_NONE),"expired snapshot inaccessible");
#else
  store.clear();
#endif
  a.set32(Heads,Heads);a.set32(Heads+4,Nil);a.set32(Heads+8,Heads);
  record(a.live(owned),0,"owned snapshot expired");
  refreshSource(a,store,root,3);record(a.get32(leaf+0x89),0,"missing key clears activity");record(a.get32(leaf+0x35),owned,"missing key retains freed snapshot alias");
  record(drainSource(a,root,3),0,"mode3 traversal filters freed alias");record(drainSource(a,root,0),owned,"mode0 returns alias identity without dereference");
  // Reacquisition under the same key replaces the dangling snapshot with the
  // current pool slot before reading health/owner/position.
  unsigned replacement=a.allocate(0xe68);a.zero(replacement,0xe68);a.set32(replacement+12,unsigned(key));a.set32(replacement+20,4);a.set32(replacement+24,0);
  a.set32(Heads,replacement);a.set32(Heads+4,replacement);a.set32(Heads+8,replacement);a.set32(replacement,Nil);a.set32(replacement+4,Heads);a.set32(replacement+8,Nil);a.set32(replacement+0xe64,1);
  store.records[0][key]=replacement;
  a.set32(Pool+0xe4,100);a.set32(Pool+0x174,1);a.set32(Pool+8,31);a.set32(Pool+12,0);a.set32(Pool+16,4);
  refreshSource(a,store,root,4);record(a.get32(leaf+0x35),Pool,"reacquisition resolves live pool");record(a.get32(leaf+0x89),3,"reacquisition restores activity");record(a.get8(leaf+0x25),tag,"source construction tag is not repaired");record(a.get32(root+0xd),31,"root center updates from live actor");
  a.set32(Pool+0xe4,0);refreshSource(a,store,root,4,false);record(a.get32(leaf+0x35),Pool,"dead current actor still replaces alias");record(a.get32(leaf+0x89),0,"dead current actor clears activity");
  a.set32(Pool+0xe4,100);a.set32(Pool+0x174,0);refreshSource(a,store,root,4,false);record(a.get32(leaf+0x89),0,"allied current actor clears activity");
#ifdef MNM_BRAIN_REFERENCE
  need(!mprotect(reinterpret_cast<void*>(node),4096,PROT_READ|PROT_WRITE),"restore retired snapshot guard for fixture inspection");
#endif
  store.clear();releaseTask(a);a.free(a.get32(Task+0x3b));
}
}
int main(int argc,char** argv)try{
#ifdef MNM_BRAIN_REFERENCE
  need(argc==2,"PE");map_image(load(argv[1]));need(mmap(reinterpret_cast<void*>(Heap),Size,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED_NOREPLACE,-1,0)!=MAP_FAILED,"alias heap");
  std::array<unsigned char,4096> thread{};user_desc seg{};seg.entry_number=unsigned(-1);seg.base_addr=address(thread.data());seg.limit=0xfffff;seg.seg_32bit=seg.limit_in_pages=seg.useable=1;need(!syscall(SYS_set_thread_area,&seg),"FS");unsigned short selector=(seg.entry_number<<3)|3u;asm volatile("movw %0, %%fs"::"rm"(selector));
  redirect(0x597850,reinterpret_cast<void*>(&ownedAllocate));redirect(0x597870,reinterpret_cast<void*>(&borrowedRelease));redirect(0x5979b0,reinterpret_cast<void*>(&bufferFree));
  const Bytes text(reinterpret_cast<unsigned char*>(0x401000),reinterpret_cast<unsigned char*>(0x5c0000));need(!mprotect(reinterpret_cast<void*>(0x401000),0x1bf000,PROT_READ|PROT_EXEC),"RX");
#else
  (void)argc;(void)argv;
#endif
#ifdef MNM_BRAIN_REFERENCE
  std::memset(reinterpret_cast<void*>(Heap),0xa5,Size);
#endif
  Arena a;current=&a;callbacksMatrix(a);staleConsumers(a);a.guards();for(auto b:a.blocks)record(b.second.live,0,"all callback/source/snapshot allocations released");
#ifdef MNM_BRAIN_REFERENCE
  need(!std::memcmp(text.data(),reinterpret_cast<void*>(0x401000),text.size()),"original alias consumer text unchanged");
#endif
  std::cout<<"{\"success\":true,\"checks\":"<<checks<<",\"callback_classes\":13,\"callback_invocations\":78,\"source_refreshes\":5,\"digest\":\""<<digest<<"\"}\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
