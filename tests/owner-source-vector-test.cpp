// Task6 cached source snapshots through real install/retirement and base cleanup.
#define MNM_TASK_STORAGE_FIXTURE
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-variable"
#include "owner-task-storage-test.cpp"
#pragma GCC diagnostic pop
#include "../reconstruction/creature-ai/owner_task_sources.hpp"
#include "../reconstruction/creature-ai/owner_task6_owner.hpp"
#include <map>
namespace {
struct SourceStorage:Storage {
  unsigned mode=0;
  std::vector<unsigned> releases;
  void reset(){Storage::reset();releases.clear();}
  void free(unsigned p){if(p)releases.push_back(p);Storage::free(p);}
  std::map<unsigned,unsigned> roles(unsigned task){
    std::map<unsigned,unsigned> out{{task,1}};unsigned index=0;
    for(unsigned p:owned(task)){out[p]=0x1000+index;if(unsigned q=get32(p+0x29))out[q]=0x2000+index;++index;}
    if(unsigned q=get32(task+0x3b))out[q]=0x3000;
    for(unsigned off:{0xa4u,0xb4u,0x101u,0x111u})out[get32(task+off+4)]=0x4000+off;
    return out;
  }
  void releaseTrace(unsigned from,const std::map<unsigned,unsigned>& known){
    unsigned count=0;for(unsigned i=from;i<releases.size();++i)if(auto at=known.find(releases[i]);at!=known.end()){record(at->second,at->second,"owned release order");++count;}
    record(count,known.size(),"all owned allocations released exactly once");for(auto at:known)need(!live(at.first),"owned allocation still alive");
  }
  unsigned get32(unsigned p){
    if(p==0x5d854c){
#ifdef MNM_BRAIN_REFERENCE
      return *reinterpret_cast<unsigned*>(p);
#else
      return mode;
#endif
    }return Storage::get32(p);
  }
  void set32(unsigned p,unsigned v){if(p==0x5d854c){
#ifdef MNM_BRAIN_REFERENCE
    *reinterpret_cast<unsigned*>(p)=v;
#else
    mode=v;
#endif
  }else Storage::set32(p,v);}
  void destroySource(unsigned p){
#ifdef MNM_BRAIN_REFERENCE
    reinterpret_cast<void(__attribute__((thiscall)) *)(void*)>(0x407650)(reinterpret_cast<void*>(p));
#else
    m::OwnerTaskSources<SourceStorage>(*this).destroy(p);
#endif
  }
  unsigned make6(){
#ifdef MNM_BRAIN_REFERENCE
    return reinterpret_cast<unsigned(*)()>(0x426070)();
#else
    return m::OwnerTaskStorage<SourceStorage>(*this).task6();
#endif
  }
  void destroy6(unsigned p){
#ifdef MNM_BRAIN_REFERENCE
    reinterpret_cast<void(__attribute__((thiscall)) *)(void*)>(0x418050)(reinterpret_cast<void*>(p));
#else
    m::OwnerTaskStorage<SourceStorage>(*this).destroy(p);
#endif
  }
  unsigned leaf(unsigned actor=0){
    unsigned p=allocate(0x8d);for(unsigned i=0;i<0x8d;++i)set8(p+i,0);
    set32(p,UINT32_MAX);set32(p+0x35,actor);set32(p+0x41,0x01010101);set32(p+0x45,0x01010101);set32(p+0x89,3);return p;
  }
  bool live(unsigned p){for(auto b:blocks)if(b.p==p)return b.alive;throw std::runtime_error("unknown source block");}
  std::vector<unsigned> owned(unsigned task){std::vector<unsigned> out;for(unsigned a=get32(task+0x3b);a!=get32(task+0x3f);a+=4)out.push_back(get32(a));return out;}
  // All source identities come from the owned vector, never original outputs.
  void snapshot(unsigned task,unsigned borrowed,const std::vector<unsigned>& ids={}){
    auto nodes=ids.empty()?owned(task):ids;
    auto token=[&](unsigned p){if(!p)return 0u;for(unsigned i=0;i<nodes.size();++i)if(p==nodes[i])return 0x30000000u+i*0x100;need(false,"owned source identity");return 0u;};
    Bytes raw(pointer(task,0x1b4),pointer(task,0x1b4)+0x1b4);
    auto putRaw=[&](unsigned o,unsigned v){std::memcpy(raw.data()+o,&v,4);};
    if(get32(task+8)!=0xa5a5a5a5u){need(get32(task+8)==borrowed,"borrowed task source");putRaw(8,0x31000000);}
    for(unsigned off:{0x37u,0xa4u,0xb4u,0x101u,0x111u}){
      unsigned start=get32(task+off+4);
      if(off==0x37){unsigned end=get32(task+off+8),cap=get32(task+off+12);putRaw(off+4,start?0x32000000:0);putRaw(off+8,start?0x32000000+end-start:0);putRaw(off+12,start?0x32000000+cap-start:0);}
      else{need(start,"live task head");putRaw(off+4,0x33000000+off);need(!get32(task+off+12),"empty task map");
        const unsigned global=off==0xa4?0x5faf2c:off==0xb4?0x5faf34:off==0x101?0x5faf3c:0x5faf44;
        need(get32(start)==start&&get32(start+8)==start&&get32(start+4)==get32(global),"empty head and shared nil identity");}
    }
    if(get32(task+0x16f)!=0xa5a5a5a5u)putRaw(0x16f,token(get32(task+0x16f)));
    for(unsigned o:{0x37u,0xa4u,0xa5u,0xb4u,0xb5u,0xc4u,0xd4u,0x101u,0x102u,0x111u,0x112u,0x121u,0x131u,0x15fu,0x17bu})raw[o]=0;
    for(unsigned v:raw)record(v,v,"kind6 object bytes");
    record(nodes.size(),nodes.size(),"owned source count");
    for(unsigned index=0;index<nodes.size();++index){unsigned p=nodes[index];Bytes data(pointer(p,0x8d),pointer(p,0x8d)+0x8d);
      auto norm=[&](unsigned o,unsigned v){std::memcpy(data.data()+o,&v,4);};
      unsigned start=get32(p+0x29),end=get32(p+0x2d),cap=get32(p+0x31);
      norm(0x39,token(get32(p+0x39)));
      if(!get32(p+0x35))data[0x25]=0;
      for(unsigned o:{0x29u,0x2du,0x31u}){unsigned v=get32(p+o);norm(o,start?0x34000000+v-start:0);}
      for(unsigned i=0;i<8;++i){unsigned v=get32(p+0x49+i*4);if(start&&v>=start&&v<=cap)norm(0x49+i*4,0x34000000+v-start);}
      for(unsigned a=start;a!=end;a+=4)record(token(get32(a)),token(get32(a)),"owned child identity");
      for(unsigned v:data)record(v,v,"owned source bytes");
    }guards();
  }
};
SourceStorage* sourceStore;
#ifdef MNM_BRAIN_REFERENCE
void* __attribute__((cdecl)) sourceVectorAllocate(unsigned n){return reinterpret_cast<void*>(sourceStore->allocate(n));}
void __attribute__((cdecl)) sourceVectorFree(void* p){sourceStore->free(address(p));}
#endif
struct Forest{unsigned root=0,actor=0,borrowed=0;std::vector<unsigned> nodes;};
Forest forest(SourceStorage& s,unsigned n){
  Forest f;f.actor=s.allocate(0x200);for(unsigned i=0;i<0x200;++i)s.set8(f.actor+i,0);
  f.borrowed=s.leaf(f.actor);f.root=s.leaf();f.nodes.push_back(f.root);
  // Create child-first parent destruction cases separately; snapshots allocate
  // parent first, then recursively clone each child in original traversal order.
  const unsigned count=n%10;
  for(unsigned i=0;i<count;++i){unsigned p=s.leaf(i%3?f.actor:0);f.nodes.push_back(p);m::OwnerTaskSources<SourceStorage>(s).attach(f.root,p);
    if(i%3==0){unsigned child=s.leaf(f.actor);f.nodes.push_back(child);m::OwnerTaskSources<SourceStorage>(s).attach(p,child);}
  }
  for(unsigned i=0;i<f.nodes.size();++i){unsigned p=f.nodes[i];
    for(unsigned off:{9u,0xdu,0x11u,0x15u,0x19u,0x1du,0x21u})s.set32(p+off,n*97+i*13+off);
    // Vary free-slot exhaustion/round-robin and source mode activity admission.
    s.set32(p+0x3d,(n+i)%8);for(unsigned j=0;j<8;++j){s.set8(p+0x41+j,(n&1)?0:((j+n+i)%3==0));s.set32(p+0x49+j*4,s.get32(p+0x29));s.set32(p+0x69+j*4,0);}
    s.set32(p+0x89,(n+i)%4);
  }
  if(n%7==0&&count)f.root=f.nodes.back();
  s.set32(0x5d854c,n%4);return f;
}
unsigned copyCached(SourceStorage& s,unsigned task,unsigned root){
#ifdef MNM_BRAIN_REFERENCE
  (void)s;std::array<unsigned,16> cache{};unsigned p=root;cache[9]=address(&p);cache[10]=address(&p+1);
  return reinterpret_cast<unsigned(__attribute__((thiscall)) *)(void*,void*,int,int)>(0x413330)(reinterpret_cast<void*>(task),cache.data(),100,1);
#else
  return m::OwnerTaskSources<SourceStorage>(s).clone(task,root);
#endif
}
void borrowedTrace(SourceStorage& s,const Forest& f){
  for(unsigned p:f.nodes){record(s.live(p),1,"borrowed cache source alive");for(unsigned i=0;i<0x8d;++i)record(s.get8(p+i),s.get8(p+i),"borrowed cache source bytes");}
  for(unsigned i=0;i<0x200;++i)record(s.get8(f.actor+i),0,"borrowed actor untouched");
}
void callerSourceTrace(SourceStorage& s,const Forest& f,unsigned active){
  Bytes data(s.pointer(f.borrowed,0x8d),s.pointer(f.borrowed,0x8d)+0x8d);
  need(s.get32(f.borrowed+5)==active,"caller source identity before normalization");unsigned token=active?0x35000000:0;std::memcpy(data.data()+5,&token,4);
  for(unsigned value:data)record(value,value,"borrowed friendly source bytes");
}
void releaseFixture(SourceStorage& s){for(auto b:s.blocks)if(b.alive)s.free(b.p);need(!s.alive(),"fixture fully released");}
}
int main(int argc,char** argv)try{
#ifdef MNM_BRAIN_REFERENCE
  need(argc==2,"PE");map_image(load(argv[1]));need(mmap(reinterpret_cast<void*>(Heap),Extent,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED_NOREPLACE,-1,0)!=MAP_FAILED,"source heap");
  std::array<unsigned char,4096> thread{};user_desc seg{};seg.entry_number=unsigned(-1);seg.base_addr=address(thread.data());seg.limit=0xfffff;seg.seg_32bit=seg.limit_in_pages=seg.useable=1;need(!syscall(SYS_set_thread_area,&seg),"FS");unsigned short selector=(seg.entry_number<<3)|3u;asm volatile("movw %0, %%fs"::"rm"(selector));
  redirect(0x597850,reinterpret_cast<void*>(&sourceVectorAllocate));redirect(0x597870,reinterpret_cast<void*>(&sourceVectorFree));redirect(0x416fe0,reinterpret_cast<void*>(&storageRegister));redirect(0x476e30,reinterpret_cast<void*>(&storageTraining));
  const Bytes text(reinterpret_cast<unsigned char*>(0x401000),reinterpret_cast<unsigned char*>(0x5c0000));need(!mprotect(reinterpret_cast<void*>(0x401000),0x1bf000,PROT_READ|PROT_EXEC),"source RX");
#else
  (void)argc;(void)argv;
#endif
  SourceStorage s;sourceStore=&s;unsigned copied=0,installed=0,retired=0,early=0;
  for(unsigned n=0;n<64;++n){
    s.reset();auto f=forest(s,n);unsigned task=s.make6();need(task,"task6 creation");s.snapshot(task,f.borrowed);
    for(unsigned j=0;j<1+n%3;++j){unsigned p=copyCached(s,task,f.root);need(p,"cached clone");++copied;s.snapshot(task,f.borrowed);record(s.get32(0x5d854c),n%4,"copy mode preserved");}
    auto ids=s.owned(task);std::vector<unsigned> backing;for(unsigned p:ids)backing.push_back(s.get32(p+0x29));
    // Reverse owned-vector order in half the cases to exercise child-before-parent
    // destruction through the real base loop, without aliased ownership.
    if(n&2){unsigned at=s.get32(task+0x3b);std::reverse(ids.begin(),ids.end());for(unsigned i=0;i<ids.size();++i)s.set32(at+i*4,ids[i]);}
    auto known=s.roles(task);unsigned mark=s.releases.size();
    s.destroy6(task);s.free(task);s.releaseTrace(mark,known);for(unsigned p:ids){record(s.live(p),0,"owned source freed");record(s.get32(p+0x29),0,"owned child vector retired");record(s.get32(p+0x2d),0,"owned child end retired");record(s.get32(p+0x31),0,"owned child cap retired");}
    for(unsigned p:backing)if(p)record(s.live(p),0,"source child backing freed");
    for(unsigned g:{0x5faf2cu,0x5faf34u,0x5faf3cu,0x5faf44u}){record(s.get32(g),0,"direct nil released");record(s.get32(g+4),0,"direct nil count zero");}
    record(s.live(f.actor),1,"borrowed actor alive");record(s.live(f.root),1,"cached tree alive");record(s.get32(0x5d854c),n%4,"destruction mode restored");borrowedTrace(s,f);s.guards();releaseFixture(s);
    s.reset();s.fail=1;record(s.make6(),0,"task6 first allocation refusal");need(!s.alive(),"failed factory owns nothing");
  }
  for(unsigned n=0;n<32;++n){
    s.reset();auto f=forest(s,n+64);
#ifdef MNM_BRAIN_REFERENCE
    m::OwnerSourceStorage v;v.sources.roots.resize(3);RawSources raw(v);auto* b=raw.brain.data();
    global(0x643a80,1);global(0x643a80+12,0);unsigned evaluator=s.make6();put(b,0x11fe5,address(&evaluator));put(b,0xca,0);put(b,0x11ddc,0);
#else
    m::OwnerTask6Owner<SourceStorage> owner(s,f.borrowed);
#endif
    s.set32(0x5d854c,n%4);
    for(unsigned generation=0;generation<4;++generation){unsigned tick=100+generation*40,duration=8+(n+generation)%23;float value=float(n+generation)/8;
#ifdef MNM_BRAIN_REFERENCE
      unsigned target=copyCached(s,evaluator,f.root);s.set32(evaluator+0x16f,target);s.set32(evaluator+0x177,duration);put(b,0x14,tick);put(b,0x11ddd,bits(value));
      reinterpret_cast<void(__attribute__((thiscall)) *)(void*,void*)>(0x4289e0)(b,reinterpret_cast<void*>(f.borrowed));unsigned active=s.get32(f.borrowed+5);
#else
      owner.prepareCached(f.root,duration);owner.install(tick,value);unsigned active=owner.active,evaluator=owner.evaluator;
#endif
      ++installed;callerSourceTrace(s,f,active);s.snapshot(active,f.borrowed);s.snapshot(evaluator,f.borrowed);auto ids=s.owned(active);copied++;
#ifdef MNM_BRAIN_REFERENCE
      put(b,0x14,tick+duration-1);reinterpret_cast<void(__attribute__((thiscall)) *)(void*)>(0x428b40)(b);record(s.get32(f.borrowed+5)==active,1,"before expiry retained");
#else
      need(!owner.retire(tick+duration-1),"before expiry retained");record(s.get32(f.borrowed+5)==active,1,"before expiry retained");
#endif
      ++early;bool completion=(n+generation)%2;auto known=s.roles(active);unsigned mark=s.releases.size();
#ifdef MNM_BRAIN_REFERENCE
      if(completion){s.set8(active+0x14,1);}put(b,0x14,tick+(completion?duration-1:duration));reinterpret_cast<void(__attribute__((thiscall)) *)(void*)>(0x428b40)(b);need(!s.get32(f.borrowed+5),"caller unbound source");
#else
      need(owner.retire(tick+(completion?duration-1:duration),completion),"caller retired task");
#endif
      s.releaseTrace(mark,known);borrowedTrace(s,f);callerSourceTrace(s,f,0);
      // Use equal observations for completion/expiry; original side verified its
      // raw source backlink above, native side verifies through its public API.
      for(unsigned p:ids){record(s.live(p),0,"caller freed owned source");}record(s.live(active),0,"caller freed task");record(s.get8(active+0x14),completion,"retired completion byte");record(s.get32(f.borrowed+5),0,"borrowed backlink clear");
      record(s.get32(0x5d854c),n%4,"caller restored mode");record(s.live(f.actor),1,"caller borrowed actor alive");++retired;s.guards();
    }
#ifdef MNM_BRAIN_REFERENCE
    s.destroy6(evaluator);s.free(evaluator);
#else
    owner.close();
#endif
    for(unsigned g:{0x5faf2cu,0x5faf34u,0x5faf3cu,0x5faf44u}){record(s.get32(g),0,"caller nil released");record(s.get32(g+4),0,"caller nil count zero");}releaseFixture(s);
  }
  std::cout<<"{\"success\":true,\"cases\":96,\"copied_trees\":"<<copied<<",\"installed\":"<<installed<<",\"retired\":"<<retired<<",\"early_retained\":"<<early<<",\"checks\":"<<checks<<",\"digest\":\""<<digest<<"\"}\n";
#ifdef MNM_BRAIN_REFERENCE
  need(!std::memcmp(reinterpret_cast<void*>(0x401000),text.data(),text.size()),"source original text unchanged");
#endif
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
