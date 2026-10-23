// Fresh task storage and ordinary fallback scheduler through real retirement.
#define MNM_SOURCE_DECISION_FIXTURE
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#include "owner-source-decision-test.cpp"
#pragma GCC diagnostic pop
#include "../reconstruction/creature-ai/owner_task_storage.hpp"
#include "../reconstruction/creature-ai/owner_task_scheduler.hpp"
#include "../reconstruction/creature-ai/owner_fallback_task.hpp"
#include "../reconstruction/creature-ai/owner_fallback_owner.hpp"
#include <initializer_list>
#include <cstdlib>
namespace {
constexpr unsigned Heap=0x12000000, Extent=0x200000, Borrowed=Heap+Extent-512;
struct Storage {
  struct Block {unsigned p,n;bool alive;};
  Bytes bytes=Bytes(Extent),globals=Bytes(32);
  std::vector<Block> blocks;
  unsigned cursor=32,attempts=0,fail=0,freed=0;
  unsigned char* pointer(unsigned p,unsigned n) {
    if(p>=0x5faf2c && p-0x5faf2c<=32 && n<=32-(p-0x5faf2c)) {
#ifdef MNM_BRAIN_REFERENCE
      return reinterpret_cast<unsigned char*>(p);
#else
      return globals.data()+p-0x5faf2c;
#endif
    }
    need(p>=Heap && p-Heap<=Extent && n<=Extent-(p-Heap),"task storage extent");
#ifdef MNM_BRAIN_REFERENCE
    return reinterpret_cast<unsigned char*>(p);
#else
    return bytes.data()+p-Heap;
#endif
  }
  unsigned get32(unsigned p){unsigned v;std::memcpy(&v,pointer(p,4),4);return v;}
  unsigned get8(unsigned p){return *pointer(p,1);}
  void set32(unsigned p,unsigned v){std::memcpy(pointer(p,4),&v,4);}
  void set8(unsigned p,unsigned v){*pointer(p,1)=static_cast<unsigned char>(v);}
  void reset(){blocks.clear();cursor=32;attempts=freed=0;fail=0;std::memset(pointer(Heap,Extent),0xa5,Extent);std::memset(pointer(0x5faf2c,32),0,32);}
  unsigned allocate(unsigned n){
    if(++attempts==fail)return 0;
    need(n<65536 && cursor+n+32<Extent,"task allocation bound");
    unsigned p=Heap+cursor;cursor+=(n+15)&~15u;cursor+=64;
    blocks.push_back({p,n,true});return p;
  }
  void free(unsigned p){
    if(!p)return;
    for(auto& b:blocks)if(b.p==p){need(b.alive,"task double release");b.alive=false;++freed;return;}
#ifdef MNM_BRAIN_REFERENCE
    // Source fixture backing is retained separately, never an owned task block.
    sourceRelease(reinterpret_cast<void*>(p));return;
#else
    throw std::runtime_error("task unknown allocation");
#endif
  }
  void guards(){for(const auto& b:blocks)for(unsigned i=0;i<16;++i){need(get8(b.p-16+i)==0xa5,"task prefix canary");need(get8(b.p+b.n+i)==0xa5,"task suffix canary");}}
  unsigned alive(){unsigned n=0;for(auto b:blocks)n+=b.alive;return n;}
  void destroySource(unsigned){throw std::runtime_error("owned source provider outside empty fallback domain");}
  unsigned make(){
#ifdef MNM_BRAIN_REFERENCE
    return reinterpret_cast<unsigned(*)()>(0x419e40)();
#else
    return m::OwnerTaskStorage<Storage>(*this).fallback();
#endif
  }
  void destroy(unsigned p){
#ifdef MNM_BRAIN_REFERENCE
    reinterpret_cast<void(__attribute__((thiscall)) *)(void*)>(0x418050)(reinterpret_cast<void*>(p));
#else
    m::OwnerTaskStorage<Storage>(*this).destroy(p);
#endif
  }
  // Exclude only constructor stack-derived allocator tags, checked separately
  // against an allowlist; all other initialized and untouched bytes participate.
  std::vector<unsigned> taskFields(unsigned p,unsigned source,bool installed=false){
    Bytes raw(pointer(p,0x14b),pointer(p,0x14b)+0x14b);
    auto normalize=[&](unsigned off,unsigned value){std::memcpy(raw.data()+off,&value,4);};
    if(installed || get32(p+8)!=0xa5a5a5a5u){need(get32(p+8)==source,"task source backlink");normalize(8,0x20000000);}
    for(auto map:{std::array<unsigned,2>{0xa4,0x5faf2c},{0xb4,0x5faf34},{0x101,0x5faf3c},{0x111,0x5faf44}}){
      unsigned head=get32(p+map[0]+4);need(head && get32(head)==head && get32(head+8)==head && get32(head+4)==get32(map[1]),"caller empty owned map linkage");
      need(!get32(p+map[0]+12),"caller empty owned map count");normalize(map[0]+4,0x21000000+map[0]);
    }
    for(unsigned o:{0x37u,0xa4u,0xa5u,0xb4u,0xb5u,0xc4u,0xd4u,0x101u,0x102u,0x111u,0x112u,0x121u,0x131u})raw[o]=0;
    return std::vector<unsigned>(raw.begin(),raw.end());
  }
  void snapshot(){
    guards();record(blocks.size(),blocks.size(),"storage allocations");
    for(auto b:blocks){
      record(b.n,b.n,"storage size");record(b.alive,b.alive,"storage lifetime");
      for(unsigned i=0;i<b.n;++i){bool tag=false;if(b.n==0x14b)for(unsigned o:{0x37u,0xa4u,0xa5u,0xb4u,0xb5u,0xc4u,0xd4u,0x101u,0x102u,0x111u,0x112u,0x121u,0x131u})tag|=i==o;
        const unsigned value=tag?0:get8(b.p+i);record(value,value,"storage bytes");}
    }
    for(unsigned i=0;i<32;++i)record(get8(0x5faf2c+i),get8(0x5faf2c+i),"shared nil state");
  }
};
Storage* store;
#ifdef MNM_BRAIN_REFERENCE
void* __attribute__((cdecl)) storageAllocate(unsigned n){return reinterpret_cast<void*>(store->allocate(n));}
void __attribute__((cdecl)) storageFree(void* p){store->free(address(p));}
void __attribute__((thiscall)) storageNoop(void*){}
void __attribute__((thiscall)) storageSight(void*,unsigned){}
void __attribute__((thiscall)) storageRegister(void*,void*){}
unsigned __attribute__((thiscall)) storageTraining(void*,unsigned,void*){return UINT32_MAX;}
#endif
void seedTree(Storage& s,unsigned task,unsigned off,unsigned global,unsigned size,unsigned seed){
  unsigned head=s.get32(task+off+4),nil=s.get32(global);
  unsigned a=s.allocate(size),b=s.allocate(size),c=s.allocate(size);
  s.set32(a,b);s.set32(a+4,head);s.set32(a+8,c);
  for(unsigned p:{b,c}){s.set32(p,nil);s.set32(p+4,a);s.set32(p+8,nil);}
  for(unsigned p:{a,b,c}){s.set32(p+size-4,p==a?1:0);s.set32(p+12,seed+(p==b?0:p==a?1:2));}
  s.set32(head,b);s.set32(head+4,a);s.set32(head+8,c);s.set32(task+off+12,3);
}
}
#ifndef MNM_TASK_STORAGE_FIXTURE
int main(int argc,char** argv) try {
#ifdef MNM_BRAIN_REFERENCE
  need(argc==2,"PE");map_image(load(argv[1]));
  need(mmap(reinterpret_cast<void*>(Heap),Extent,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED_NOREPLACE,-1,0)!=MAP_FAILED,"task heap mapping");
  std::array<unsigned char,4096> thread{};user_desc seg{};seg.entry_number=unsigned(-1);seg.base_addr=address(thread.data());seg.limit=0xfffff;seg.seg_32bit=seg.limit_in_pages=seg.useable=1;need(!syscall(SYS_set_thread_area,&seg),"FS");unsigned short selector=(seg.entry_number<<3)|3u;asm volatile("movw %0, %%fs"::"rm"(selector));
  redirect(0x597850,reinterpret_cast<void*>(&storageAllocate));redirect(0x597870,reinterpret_cast<void*>(&storageFree));
  for(unsigned a:{0x417580u,0x428e40u,0x428f80u})redirect(a,reinterpret_cast<void*>(&storageNoop));
  redirect(0x4181d0,reinterpret_cast<void*>(&storageSight));redirect(0x416fe0,reinterpret_cast<void*>(&storageRegister));redirect(0x476e30,reinterpret_cast<void*>(&storageTraining));
  const Bytes text(reinterpret_cast<unsigned char*>(0x401000),reinterpret_cast<unsigned char*>(0x5c0000));need(!mprotect(reinterpret_cast<void*>(0x401000),0x1bf000,PROT_READ|PROT_EXEC),"task RX");
#else
  (void)argc;(void)argv;
#endif
  Storage memory;store=&memory;
  unsigned constructions=0,retirements=0,slices=0;
  // Shared sentinel reference counts, populated scalar maps and vector backing,
  // different deletion orders, fresh clone semantics and first allocation failure.
  for(unsigned n=0;n<64;++n){
    memory.reset();std::vector<unsigned> tasks;
    for(unsigned k=0;k<1+n%4;++k){unsigned p=memory.make();need(p,"task construction");tasks.push_back(p);++constructions;
      if(n&1)for(auto t:{std::array<unsigned,3>{0xa4,0x5faf2c,24},{0xb4,0x5faf34,56},{0x101,0x5faf3c,24},{0x111,0x5faf44,56}})seedTree(memory,p,t[0],t[1],t[2],n);
      if(n&2)for(unsigned off:{0xc4u,0xd4u,0x121u,0x131u}){unsigned v=memory.allocate(12);memory.set32(p+off+4,v);memory.set32(p+off+8,v+8);memory.set32(p+off+12,v+12);memory.set32(v,n);memory.set32(v+4,k);}
      memory.set32(p+8,Borrowed);
      memory.snapshot();
    }
    if(n&4)std::reverse(tasks.begin(),tasks.end());
    for(auto p:tasks){memory.destroy(p);memory.snapshot();memory.free(p);++retirements;for(unsigned i=0;i<128;++i)record(memory.get8(Borrowed+i),0xa5,"borrowed source untouched by base destruction");}
    need(memory.alive()==0,"all task-owned storage retired");memory.snapshot();
    memory.reset();memory.fail=1;record(memory.make(),0,"first allocation failure");need(!memory.alive(),"failed construction owns nothing");
  }
  #include "owner-task-storage-caller.inc"
  std::cout<<"{\"success\":true,\"cases\":96,\"slices\":"<<slices<<",\"storage_cases\":64,\"caller_cases\":32,\"installed\":"<<installed<<",\"expiry_drains\":"<<expired<<",\"pending_slices\":"<<pendingSlices<<",\"disabled_slices\":"<<disabledSlices<<",\"constructions\":"<<constructions<<",\"retirements\":"<<retirements<<",\"checks\":"<<checks<<",\"digest\":\""<<digest<<"\"}\n";
#ifdef MNM_BRAIN_REFERENCE
  need(!std::memcmp(reinterpret_cast<void*>(0x401000),text.data(),text.size()),"task unchanged original text");
#endif
} catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}

#endif
