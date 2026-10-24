#pragma push_macro("main")
#undef main
#define main unused_alias_snapshot_main
#include "snapshot-resource-refresh-test.cpp"
#undef main
#pragma pop_macro("main")
#include "../reconstruction/rendering/compact_member_callbacks.hpp"
#include "../reconstruction/rendering/alias_target_services.hpp"
#include <sys/wait.h>
#include <signal.h>
#include <sys/resource.h>
namespace {
namespace services=mnm::reconstruction::actor_services;
unsigned receiverSeen=0,argumentSeen=0;
bool releaseDuringCall=false;
unsigned variant=0,placed=0,allowed=0,compactCalls=0;
unsigned eventCases=0,orderCases=0;
#ifdef MNM_BRAIN_REFERENCE
unsigned failAllocation=0,allocationAttempts=0;
#endif
std::map<unsigned,unsigned> world;
unsigned word(unsigned p){
#ifdef MNM_BRAIN_REFERENCE
  unsigned v;std::memcpy(&v,reinterpret_cast<void*>(p),4);return v;
#else
  return current->get32(p);
#endif
}
void put(unsigned p,unsigned v){
#ifdef MNM_BRAIN_REFERENCE
  std::memcpy(reinterpret_cast<void*>(p),&v,4);
#else
  current->set32(p,v);
#endif
}
unsigned byte(unsigned p){
#ifdef MNM_BRAIN_REFERENCE
  return *reinterpret_cast<unsigned char*>(p);
#else
  return current->get8(p);
#endif
}
unsigned service(unsigned entry,unsigned self,std::initializer_list<unsigned> args){
  if(entry==0x597870){current->free(*args.begin());return 0;}
  record(entry,entry,"ordered service entry");record(self,self,"ordered service receiver");
  std::vector<unsigned> a(args);
  if(entry==0x44d970){
    need(a.size()==4,"query arity");
    for(unsigned i=1;i<4;++i)record(word(a[i]),word(a[i]),"query normalized input");
    record(word(Pool+0x104),(variant&1)?1:37,"temporary movement rate");
    put(a[0],29);put(a[1],7);put(a[2],9);put(a[3],2);return 41;
  }
  if(entry==0x4f4900){
    need(a.size()==4,"placement arity");
    for(unsigned i=0;i<3;++i)record(a[i],a[i],"placement position");
    for(unsigned i=0;i<22;++i)record(byte(a[3]+i),byte(a[3]+i),"packed placement descriptor");
    return allowed;
  }
  for(unsigned v:args)record(v,v,"ordered service argument");
  switch(entry){
  case 0x4f5230:return placed;
  case 0x509230:put(self+8,a[0]);put(self+12,a[1]);put(self+16,a[2]);break;
  case 0x4970b0:return Pool+0x2000;
  case 0x50eb60:{unsigned count=word(self+0x668);put(self+0x668,count?count-1:0);break;}
  case 0x52d0c0:{
    unsigned head=word(self+4),node=word(head);
    while(node!=head){unsigned next=word(node);current->free(node);node=next;}
    put(head,head);put(head+4,head);put(self+8,0);break;
  }
  case 0x597870:current->free(a[0]);break;
  case 0x54dad0:case 0x54dce0:case 0x51f5e0:case 0x506d30:
  case 0x5093e0:case 0x5070e0:case 0x51a8f0:case 0x50f850:case 0x51aa30:
  case 0x51a880:case 0x5178c0:case 0x520e70:case 0x50ab30:case 0x4f2dd0:
  case 0x50aa30:case 0x4943f0:case 0x512340:case 0x520ad0:case 0x512220:case 0x52c9d0:break;
  default:throw std::runtime_error("unknown target service");
  }
  return 0;
}
struct TargetMemory {
  Arena& a;
  unsigned get32(unsigned p){auto it=world.find(p);return it==world.end()?a.get32(p):it->second;}
  unsigned get8(unsigned p){return a.get8(p);}
  void set32(unsigned p,unsigned v){a.set32(p,v);}
  void set8(unsigned p,unsigned v){a.set8(p,v);}
  unsigned temporary(unsigned n){need(n<=256,"target scratch extent");return Scratch+0x1000;}
  unsigned invoke(unsigned entry,unsigned self,std::initializer_list<unsigned> args){
    if(entry==0x51e890)return *services::AliasTargetServices<TargetMemory>(*this).tryRun(entry,self,args);
    return service(entry,self,args);
  }
};
#ifdef MNM_BRAIN_REFERENCE
template<unsigned Entry> unsigned __attribute__((thiscall)) end0(void* self){return service(Entry,address(self),{});}
template<unsigned Entry> unsigned __attribute__((thiscall)) end1(void* self,unsigned a){return service(Entry,address(self),{a});}
template<unsigned Entry> unsigned __attribute__((thiscall)) end2(void* self,unsigned a,unsigned b){return service(Entry,address(self),{a,b});}
template<unsigned Entry> unsigned __attribute__((thiscall)) end3(void* self,unsigned a,unsigned b,unsigned c){return service(Entry,address(self),{a,b,c});}
template<unsigned Entry> unsigned __attribute__((thiscall)) end4(void* self,unsigned a,unsigned b,unsigned c,unsigned d){return service(Entry,address(self),{a,b,c,d});}
template<unsigned Entry> unsigned __attribute__((thiscall)) end5(void* self,unsigned a,unsigned b,unsigned c,unsigned d,unsigned e){return service(Entry,address(self),{a,b,c,d,e});}
#endif
unsigned target(Arena& a,unsigned entry,unsigned self,std::initializer_list<unsigned> args={}){
#ifdef MNM_BRAIN_REFERENCE
  (void)a;
  if(args.size())return reinterpret_cast<unsigned(__attribute__((thiscall)) *)(void*,unsigned)>(entry)(reinterpret_cast<void*>(self),*args.begin());
  return reinterpret_cast<unsigned char(__attribute__((thiscall)) *)(void*)>(entry)(reinterpret_cast<void*>(self));
#else
  TargetMemory memory{a};auto result=entry==0x52cbc0?services::MemberCallbacks<TargetMemory>(memory).tryRun(entry,self,args):services::AliasTargetServices<TargetMemory>(memory).tryRun(entry,self,args);
  need(result.has_value(),"known alias target");return *result;
#endif
}
void globals(){
  world={{0x6c5494,32},{0x6c5498,32},{0x644520,3},{0x6def58,Pool},{0x6def5c,4}};
#ifdef MNM_BRAIN_REFERENCE
  for(auto [p,v]:world)global(p,v);
#endif
}
void events(Arena& a){
  unsigned callback=a.allocate(16);a.set32(callback,0x5c7264);a.set32(callback+4,0x51e890);a.set32(callback+8,0);a.set32(callback+12,Pool);
  a.set32(Scratch+0xcf3,callback);
  for(variant=0;variant<256;++variant){
    for(unsigned event:{0u,4u,5u,6u,7u,8u,9u,10u,11u,12u,13u,14u,15u,~0u}){
      a.zero(Pool,0x3000);a.zero(Config,0x5c9);
      a.set32(Pool+0xac,Config);a.set32(Config+8,2);a.set32(Config+12,4);a.set32(Config+0x44,53);
      a.set32(Pool,2);a.set32(Pool+4,(variant&2)?0:1);
      a.set32(Pool+0x70c,~2u);a.set32(Pool+0x710,67);a.set32(Pool+0x714,3);
      a.set32(Pool+8,11);a.set32(Pool+12,13);a.set32(Pool+16,4);
      a.set32(Pool+0x104,37);a.set32(Pool+0x718,variant&1);
      a.set32(Pool+0x7c,variant&4);a.set32(Pool+0x174,(variant&8)?3:2);
      a.set32(Pool+0xa8,(variant&16)?13:(variant&32)?27:17);
      a.set32(Pool+0x618,variant&64);a.set32(Pool+0x977,2);
      a.set32(Pool+0x97f+56,3);a.set32(Pool+0x983+56,4);a.set32(Pool+0x987+56,5);
      a.set32(Pool+0xa0,variant%4);a.set32(Pool+0x9c,23);a.set32(Pool+0x108,31);
      a.set32(Pool+0x798,(variant&128)?1:0);
      a.set8(Pool+0x723,variant&8);a.set32(Pool+0x5ec,(variant&16)?14:2);
      a.set32(Pool+0x664,(variant&32)?1:0);a.set32(Pool+0x5fc,(variant&64)?20:10);
      a.set32(Pool+0x64c,0);a.set32(Pool+0x668,variant%3);a.set32(Pool+0x6a8,1);
      a.set32(Pool+0x2000+0x668,variant%3);
      placed=(variant>>2)&1;allowed=(variant>>3)&1;
      record(target(a,0x52cbc0,a.get32(Scratch+0xcf3),{event}),0,"actual callback event return");
      for(unsigned i=0;i<0xe4b;++i)digest=(digest^a.get8(Pool+i))*1099511628211ull;
      record(a.get32(Pool+0x2000+0x668),a.get32(Pool+0x2000+0x668),"peer count");
      ++eventCases;
    }
  }
  a.free(callback);
}
unsigned compactSubject=0,compactDelete=0;
struct CompactMemory {
  Arena& a;
  unsigned get32(unsigned p){return a.get32(p);}
  void set32(unsigned p,unsigned v){a.set32(p,v);}
  unsigned invoke(unsigned entry,unsigned self,std::initializer_list<unsigned> args){
    if(entry==0x597870){record(a.get32(*args.begin()),0x5c5c40,"compact reset before free");a.free(*args.begin());return 0;}
    need(entry==0xf00d,"compact member target");receiverSeen=self;argumentSeen=*args.begin();++compactCalls;
    if(releaseDuringCall){services::CompactMemberCallbacks<CompactMemory>(*this).tryRun(compactDelete,compactSubject,{1});}
    return (self^argumentSeen)+0x35u;
  }
};
#ifdef MNM_BRAIN_REFERENCE
void* __attribute__((cdecl)) targetAllocate(unsigned n){
  if(failAllocation&&++allocationAttempts==failAllocation)return nullptr;
  return reinterpret_cast<void*>(current->allocate(n));
}
unsigned __attribute__((thiscall)) compactTarget(void* self,unsigned argument){
  receiverSeen=address(self);argumentSeen=argument;++compactCalls;
  if(releaseDuringCall)reinterpret_cast<unsigned(__attribute__((thiscall)) *)(void*,unsigned)>(compactDelete)(reinterpret_cast<void*>(compactSubject),1);
  return (address(self)^argument)+0x35u;
}
void __attribute__((cdecl)) targetFree(void* p){
  unsigned value=address(p);
  if(value==compactSubject)record(current->get32(value),0x5c5c40,"compact reset before free");
  current->free(value);
}
#endif
unsigned compactRun(Arena& a,unsigned entry,unsigned self,std::initializer_list<unsigned> args){
#ifdef MNM_BRAIN_REFERENCE
  (void)a;return reinterpret_cast<unsigned(__attribute__((thiscall)) *)(void*,unsigned)>(entry)(reinterpret_cast<void*>(self),*args.begin());
#else
  CompactMemory memory{a};auto result=services::CompactMemberCallbacks<CompactMemory>(memory).tryRun(entry,self,args);
  need(result.has_value(),"known compact callback");return *result;
#endif
}
void compact(Arena& a){
  for(auto c:services::CompactMemberCallbacks<CompactMemory>::classes){
    for(unsigned flag:{0u,1u,2u,3u,0x100u,~0u}){
      compactSubject=a.allocate(12);compactDelete=c.deleting;
      a.set32(compactSubject,c.table);
#ifdef MNM_BRAIN_REFERENCE
      a.set32(compactSubject+4,address(reinterpret_cast<void*>(&compactTarget)));
#else
      a.set32(compactSubject+4,0xf00d);
#endif
      a.set32(compactSubject+8,Pool+32);releaseDuringCall=flag==~0u;
      record(compactRun(a,c.invoke,compactSubject,{flag}),((Pool+32)^flag)+0x35u,"compact result");
      record(receiverSeen,Pool+32,"compact receiver");record(argumentSeen,flag,"compact argument");
      if(!releaseDuringCall){
        record(compactRun(a,c.deleting,compactSubject,{flag}),compactSubject,"compact delete result");
        record(a.live(compactSubject),!(flag&1),"compact low flag bit");
        if(a.live(compactSubject)){record(a.get32(compactSubject),0x5c5c40,"compact base reset");a.free(compactSubject);}
      }
    }
  }
  compactSubject=0;releaseDuringCall=false;
}
void orders(Arena& a){
  for(unsigned v=0;v<128;++v){
    a.zero(Pool,0xe4b);a.zero(Config,0x5c9);a.set32(Pool+0xac,Config);
    unsigned list=a.allocate(12),head=a.allocate(48);a.zero(list,12);a.zero(head,48);
    a.set32(list+4,head);a.set32(head,head);a.set32(head+4,head);a.set32(Pool+0x8f9,list);
    std::vector<unsigned> nodes;
    for(unsigned i=0;i<3;++i){
      unsigned node=a.allocate(48);a.zero(node,48);unsigned previous=a.get32(head+4);
      a.set32(node,head);a.set32(node+4,previous);a.set32(previous,node);a.set32(head+4,node);
      a.set32(node+8,((v>>2)+i)%4);a.set32(node+12,~2u);a.set32(node+16,67);a.set32(node+20,2);a.set32(node+24,31);
      a.set32(node+40,(v&8)?Pool+0x6000:0);a.set8(node+44,(v&16)?1:0);nodes.push_back(node);
    }
    a.set32(list+8,3);a.set32(Pool+0x902,3);a.set32(Pool+0x8fd,nodes[v%3]);a.set8(Pool+0x901,v&1);
    a.set32(Pool+0x5ec,(v&32)?8:2);a.set32(Pool+0x784,v&64);
    record(target(a,0x524130,Pool),(!(v&96)&&((v&17)||v%3!=2))?1:0,"order tick result");
    for(unsigned i=0;i<0xe4b;++i)digest=(digest^a.get8(Pool+i))*1099511628211ull;
    for(unsigned p:{list,head})for(unsigned i=0;i<(p==list?12u:48u);++i)digest=(digest^a.get8(p+i))*1099511628211ull;
    for(unsigned node:nodes){record(a.live(node),a.live(node),"order node liveness");if(a.live(node)){for(unsigned i=0;i<48;++i)digest=(digest^a.get8(node+i))*1099511628211ull;a.free(node);}}
    a.free(head);a.free(list);++orderCases;
  }
}
#ifdef MNM_BRAIN_REFERENCE
__attribute__((naked)) void stopProjection(){asm volatile("ret");}
__attribute__((naked)) unsigned saveProjection(unsigned,unsigned){
  asm volatile("push %ebx; push %ebp; push %esi; push %edi; sub $32,%esp;"
               "mov 52(%esp),%esi; mov 56(%esp),%eax; mov %eax,12(%esp);"
               "mov $0x123,%ebx; mov 0x8f9(%esi),%ecx; mov $0x526977,%eax; call *%eax;"
               "mov 12(%esp),%eax; add $32,%esp; pop %edi; pop %esi; pop %ebp; pop %ebx; ret");
}
__attribute__((naked)) void loadProjection(unsigned,unsigned){
  asm volatile("push %ebp; sub $32,%esp; mov 40(%esp),%ebp;"
               "mov 44(%esp),%eax; mov %eax,20(%esp); mov $0x5292c6,%eax; call *%eax;"
               "add $32,%esp; pop %ebp; ret");
}
__attribute__((naked)) void seedProjection(unsigned){
  asm volatile("push %esi; mov 8(%esp),%esi; mov $0x523e0f,%eax; call *%eax; pop %esi; ret");
}
#endif
void cursorProjections(Arena& a){
  a.zero(Pool,0xe4b);
  unsigned list=a.allocate(12),head=a.allocate(48),expired=a.allocate(48);
  a.zero(list,12);a.zero(head,48);a.set32(Pool+0x8f9,list);a.set32(list+4,head);
  a.set32(head,head);a.set32(head+4,head);a.set32(list+8,3);
  std::vector<unsigned> nodes;
  for(unsigned i=0;i<3;++i){unsigned p=a.allocate(48);a.zero(p,48);unsigned tail=a.get32(head+4);
    a.set32(p,head);a.set32(p+4,tail);a.set32(tail,p);a.set32(head+4,p);nodes.push_back(p);}
  a.free(expired);
  for(unsigned cursor:{nodes[0],nodes[1],nodes[2],head,expired,0u}){
    for(unsigned retained:{0u,0xabcdu,~0u}){
      a.set32(Pool+0x8fd,cursor);
#ifdef MNM_BRAIN_REFERENCE
      unsigned index=saveProjection(Pool,retained);
#else
      TargetMemory memory{a};unsigned index=services::AliasTargetServices<TargetMemory>(memory).cursorIndex(Pool,retained,0x123);
#endif
      unsigned expected=retained;
      for(unsigned i=0;i<3;++i)if(cursor==nodes[i])expected=i;
      record(index,expected,"serialized cursor identity or retained resource scalar");
    }
  }
  for(unsigned index:{0u,1u,2u,3u,4u,9u,~0u,0x80000000u}){
    a.set32(Pool+0x8fd,expired);
#ifdef MNM_BRAIN_REFERENCE
    loadProjection(Pool,index);
#else
    TargetMemory memory{a};services::AliasTargetServices<TargetMemory>(memory).restoreCursor(Pool,index);
#endif
    unsigned position=(index&0x80000000u)?0:index%4;
    record(a.get32(Pool+0x8fd),position==3?head:nodes[position],"loaded cursor follows signed positive steps");
  }
  a.set32(Pool+0x8fd,expired);a.set32(Pool+0x902,~0u);
#ifdef MNM_BRAIN_REFERENCE
  seedProjection(Pool);
#else
  TargetMemory memory{a};services::AliasTargetServices<TargetMemory>(memory).seedCursorAfterAppend(Pool);
#endif
  record(a.get32(Pool+0x8fd),nodes[0],"append replaces dangling cursor before notification");
  record(a.get32(Pool+0x902),0,"append count wraps");
  for(unsigned p:nodes)a.free(p);
  a.free(head);a.free(list);
}
void staleCursorPolicies(Arena& a){
  a.zero(Pool,0xe4b);
  a.cursor=(a.cursor+4095)&~4095u;
  unsigned expired=a.allocate(48);a.cursor=(a.cursor+4095)&~4095u;a.zero(expired,48);a.free(expired);
  unsigned list=a.allocate(12),head=a.allocate(48);a.zero(list,12);a.zero(head,48);
  a.set32(list+4,head);a.set32(list+8,1);a.set32(head,head);a.set32(head+4,head);
  a.set32(Pool+0x8f9,list);a.set32(Pool+0x8fd,expired);
  for(unsigned mode=0;mode<3;++mode){
    if(mode==0)a.set32(list+8,0);
    if(mode==1)a.set32(list+8,1);
    if(mode==2)a.set32(Pool+0x8f9,0); // stripped snapshot owns no list
#ifdef MNM_BRAIN_REFERENCE
    need(!mprotect(reinterpret_cast<void*>(expired),4096,PROT_NONE),"protect retired cursor");
    pid_t child=fork();need(child>=0,"cursor fault child");
    if(!child){unsigned result=target(a,0x524130,Pool);_exit(result?31:0);}
    int status=0;need(waitpid(child,&status,0)==child,"cursor fault wait");
    bool refused=WIFSIGNALED(status)&&WTERMSIG(status)==SIGSEGV;
    if(!mode)need(WIFEXITED(status)&&WEXITSTATUS(status)==0,"empty list skips stale cursor");
    need(!mprotect(reinterpret_cast<void*>(expired),4096,PROT_READ|PROT_WRITE),"restore retired cursor");
#else
    bool refused=false;
    try{record(target(a,0x524130,Pool),0,"empty list skip result");}catch(const std::exception&){refused=true;}
    // Native memory refusal is compared as a policy outcome, never as SEH.
#endif
#ifdef MNM_BRAIN_REFERENCE
    if(!mode)record(0,0,"empty list skip result");
#endif
    record(refused,mode!=0,"invalid cursor/list original faults; native checked policy refuses");
  }
  a.free(head);a.free(list);
}
void allocationFailures(Arena& parent){
  for(unsigned failure=1;failure<=3;++failure){
#ifdef MNM_BRAIN_REFERENCE
    pid_t child=fork();need(child>=0,"allocation failure child");
    if(!child){
      Arena isolated;current=&isolated;isolated.zero(Pool,0xe4b);
      failAllocation=failure;allocationAttempts=0;
      reinterpret_cast<void(__attribute__((thiscall)) *)(void*,unsigned)>(0x5060a0)(reinterpret_cast<void*>(Pool),0);
      if(failure!=1||isolated.get32(Pool+0x708)||isolated.get32(Pool+0xcf3))_exit(32);
      reinterpret_cast<void(__attribute__((thiscall)) *)(void*)>(0x505f70)(reinterpret_cast<void*>(Pool));
      for(auto [p,b]:isolated.blocks){(void)p;if(b.live)_exit(33);}
      _exit(0);
    }
    int status=0;need(waitpid(child,&status,0)==child,"allocation failure wait");
    bool refused=WIFSIGNALED(status)&&WTERMSIG(status)==SIGSEGV;
    if(failure==1)need(WIFEXITED(status)&&WEXITSTATUS(status)==0,"null callback still owns valid list and retires");
#else
    Arena isolated;current=&isolated;isolated.zero(Pool,0xe4b);
    struct FailingBackend:ActorBackend{
      unsigned failure,attempts=0;
      unsigned allocate(unsigned n){return ++attempts==failure?0:a.allocate(n);}
    } backend{{isolated},failure};
    bool refused=false;
    try{mnm::reconstruction::ActorLifecycle<FailingBackend>(backend).reset(Pool,0);}
    catch(const std::exception&){refused=true;}
    if(failure==1){
      need(!isolated.get32(Pool+0x708)&&!isolated.get32(Pool+0xcf3),"failed callback allocation leaves both aliases null");
      mnm::reconstruction::ActorLifecycle<FailingBackend>(backend).destruct(Pool);
      for(auto [p,b]:isolated.blocks){(void)p;need(!b.live,"null callback list cleanup");}
    }else{
      unsigned live=0;for(auto [p,b]:isolated.blocks){(void)p;live+=b.live;}
      need(live==failure-1,"list/head failure retains completed allocation prefix");
      // Fixture disposal only. Original faults have no credited rollback.
      for(auto [p,b]:isolated.blocks)if(b.live)isolated.free(p);
    }
    isolated.guards();
#endif
    record(refused,failure!=1,"callback allocation null accepted; list/head allocation failure faults or refuses");
  }
  current=&parent;
}
void duplicateOwnershipPolicy(Arena& parent){
#ifdef MNM_BRAIN_REFERENCE
  pid_t child=fork();need(child>=0,"duplicate owner child");
  if(!child){
    Arena isolated;current=&isolated;isolated.zero(Pool,0x3000);
    unsigned callback=isolated.allocate(16);isolated.zero(callback,16);isolated.set32(callback,0x5c7264);
    isolated.set32(Pool+0x708,callback);isolated.set32(Pool+0x2000+0x708,callback);
    auto destroy=reinterpret_cast<void(__attribute__((thiscall)) *)(void*)>(0x505f70);
    destroy(reinterpret_cast<void*>(Pool));destroy(reinterpret_cast<void*>(Pool+0x2000));_exit(34);
  }
  int status=0;need(waitpid(child,&status,0)==child,"duplicate owner wait");
  bool refused=WIFSIGNALED(status)&&WTERMSIG(status)==SIGSEGV;
#else
  Arena isolated;current=&isolated;isolated.zero(Pool,0x3000);
  unsigned callback=isolated.allocate(16);isolated.zero(callback,16);isolated.set32(callback,0x5c7264);
  isolated.set32(Pool+0x708,callback);isolated.set32(Pool+0x2000+0x708,callback);
  m::OwnerObservationSnapshots<Arena> snapshots(isolated);snapshots.destroy(Pool);
  bool refused=false;try{snapshots.destroy(Pool+0x2000);}catch(const std::exception&){refused=true;}
  need(!isolated.get32(Pool+0x708)&&isolated.get32(Pool+0x2000+0x708)==callback,"failed duplicate release retains second owning field");
#endif
  record(refused,1,"duplicate owning aliases original faults; native refuses second release");
  current=&parent;
}
void nativeOwnershipPolicies(Arena& a){
#ifndef MNM_BRAIN_REFERENCE
  struct ThrowMemory:CompactMemory{
    bool failFree=false;unsigned callback=0;
    unsigned invoke(unsigned e,unsigned,std::initializer_list<unsigned> args){
      if(e==0x597870&&failFree)throw std::runtime_error("authored free failure");
      a.free(e==0x597870?*args.begin():callback);throw std::runtime_error("authored target failure after self deletion");
    }
  };
  for(bool freeFailure:{false,true}){
    unsigned p=a.allocate(12);a.zero(p,12);a.set32(p+4,0xf00d);a.set32(p+8,Pool);
    ThrowMemory memory{{a},freeFailure,p};bool caught=false;
    try{services::CompactMemberCallbacks<ThrowMemory>(memory).tryRun(freeFailure?0x475ba0:0x475b80,p,{1});}
    catch(const std::runtime_error&){caught=true;}
    need(caught,"callback exception propagated");
    if(freeFailure){need(a.live(p)&&a.get32(p)==0x5c5c40,"failed free leaves reset owned storage");a.free(p);}
    else need(!a.live(p),"throw after self deletion has no post-return read");
  }
#else
  (void)a;
#endif
}
}
int main(int argc,char** argv)try{
#ifdef MNM_BRAIN_REFERENCE
  need(argc==2,"PE");map_image(load(argv[1]));need(mmap(reinterpret_cast<void*>(Heap),Size,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED_NOREPLACE,-1,0)!=MAP_FAILED,"target heap");
  std::array<unsigned char,4096> thread{};user_desc seg{};seg.entry_number=unsigned(-1);seg.base_addr=address(thread.data());seg.limit=0xfffff;seg.seg_32bit=seg.limit_in_pages=seg.useable=1;need(!syscall(SYS_set_thread_area,&seg),"FS");unsigned short selector=(seg.entry_number<<3)|3u;asm volatile("movw %0, %%fs"::"rm"(selector));
  redirect(0x597850,reinterpret_cast<void*>(&targetAllocate));redirect(0x597870,reinterpret_cast<void*>(&targetFree));redirect(0x5979b0,reinterpret_cast<void*>(&bufferFree));
#define END(E,N) redirect(E,reinterpret_cast<void*>(&end##N<E>))
  END(0x44d970,4);END(0x4f5230,2);END(0x4f4900,4);END(0x5093e0,3);END(0x5070e0,4);END(0x509230,3);
  END(0x54dad0,1);END(0x54dce0,1);END(0x51f5e0,1);END(0x506d30,0);END(0x51a8f0,0);END(0x50f850,0);END(0x51aa30,0);
  END(0x51a880,0);END(0x5178c0,0);END(0x520e70,0);END(0x50ab30,0);END(0x4f2dd0,0);END(0x4970b0,1);END(0x50eb60,1);END(0x50aa30,1);
  END(0x52c9d0,1);END(0x512220,0);
  redirect(0x5269a5,reinterpret_cast<void*>(&stopProjection));redirect(0x5292f0,reinterpret_cast<void*>(&stopProjection));redirect(0x523e34,reinterpret_cast<void*>(&stopProjection));
  END(0x4943f0,0);END(0x512340,0);END(0x52d0c0,0);END(0x520ad0,5);
#undef END
  const Bytes text(reinterpret_cast<unsigned char*>(0x401000),reinterpret_cast<unsigned char*>(0x5c0000));need(!mprotect(reinterpret_cast<void*>(0x401000),0x1bf000,PROT_READ|PROT_EXEC),"target RX");std::memset(reinterpret_cast<void*>(Heap),0xa5,Size);
#else
  (void)argc;(void)argv;
#endif
  struct rlimit noCore{0,0};need(!setrlimit(RLIMIT_CORE,&noCore),"disable disposable child core files");
  Arena a;current=&a;globals();allocationFailures(a);duplicateOwnershipPolicy(a);compact(a);events(a);orders(a);cursorProjections(a);staleCursorPolicies(a);nativeOwnershipPolicies(a);a.guards();
  for(auto [p,b]:a.blocks){(void)p;need(!b.live,"all target allocations retired");}
#ifdef MNM_BRAIN_REFERENCE
  need(!std::memcmp(text.data(),reinterpret_cast<void*>(0x401000),text.size()),"target code unchanged");
#endif
  std::cout<<"{\"success\":true,\"checks\":"<<checks<<",\"compact_classes\":33,\"compact_calls\":"<<compactCalls<<",\"event_cases\":"<<eventCases<<",\"order_cases\":"<<orderCases<<",\"digest\":\""<<digest<<"\"}\n";
  return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
