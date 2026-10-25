#ifdef ORIGINAL_REFERENCE
#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
#include <asm/ldt.h>
#include <sys/syscall.h>
#include <unistd.h>
#else
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>
using Bytes=std::vector<unsigned char>;
#endif
#include "terrain_light_map_load.hpp"
#include <array>
#include <algorithm>
using namespace mnm::reconstruction;using U=std::uint32_t;
constexpr U Arena=0x10000000,Map=0x6c5490,Stream=0x1234,Sentinel=Arena+0x100,Initial=Arena+0x1000;
struct Memory {
#ifndef ORIGINAL_REFERENCE
 Bytes image=Bytes(0x400000),arena=Bytes(0x400000);
#endif
 Bytes input;std::vector<U> events;std::array<unsigned char,28> record{};
 U cursor=0,used=0,reads=0,allocations=0,failRead=0,failAllocation=0,sectionCalls=0,failSection=0,failTail=0;
 unsigned char* ptr(U p){
#ifdef ORIGINAL_REFERENCE
  return reinterpret_cast<unsigned char*>(std::uintptr_t(p));
#else
  if(p>=0x400000 && p<0x800000)return image.data()+p-0x400000;
  if(p>=Arena && p<Arena+arena.size())return arena.data()+p-Arena;
  throw std::runtime_error("Guest address outside fixture");
#endif
 }
 U get32(U p){U v;std::memcpy(&v,ptr(p),4);return v;}
 void set32(U p,U v){std::memcpy(ptr(p),&v,4);}
 void event(U address,std::initializer_list<U> args){events.push_back(address);events.push_back(U(args.size()));events.insert(events.end(),args);}
 U allocate(U n){++allocations;U at=allocations==failAllocation?0:Arena+0x10000+used;event(0x597850,{n,at});if(at){if(used+n+32>0x300000)throw std::runtime_error("Arena budget");std::memset(ptr(at),0xa5,n+32);used+=n+32;}return at;}
 void release(U p){event(0x597870,{p});}
 U readRaw(void* target,U normalized,U size,U count,U stream){
  ++reads;event(0x59c359,{normalized,size,count,stream});U n=count;
  if(reads==failRead)n=count?count-1:0;
  if(cursor+std::uint64_t(size)*n>input.size())throw std::runtime_error("Scripted input exhausted");
  if(size*n)std::memcpy(target,input.data()+cursor,size*n);cursor+=size*n;return n;
 }
 U readGuest(U p,U size,U count,U stream){return readRaw(ptr(p),p,size,count,stream);}
 U readValue(U& value,U stream){return readRaw(&value,0,4,1,stream);}
 U loadSection(U p,U stream){++sectionCalls;event(0x533550,{p,stream});return sectionCalls==failSection?~U(0):8;}
 U loadTail(U p,U stream){event(0x49f490,{p,stream});return failTail?~U(0):0;}
 void initializeLoadedRecord(){record.fill(0);event(0x464c50,{});event(0x464ca0,{0x6c4518});}
 U loadLinkedRecord(U cell,U stream){event(0x4652a0,{cell,stream});record.fill(0);std::memcpy(record.data(),&cell,4);for(U i=8;i<28;++i)record[i]=static_cast<unsigned char>(i*7);return 16;}
 U allocateLinkedNode(U list,U sentinel,U prev){event(0x502490,{list,sentinel,prev});U node=allocate(36);set32(node,sentinel);set32(node+4,prev);return node;}
 void copyLoadedRecord(U p){event(0x5027e0,{p});std::memcpy(ptr(p),record.data(),record.size());}
 void erase(U list,U node){event(0x4bfeb0,{list,node});U next=get32(node),prev=get32(node+4);set32(prev,next);set32(next+4,prev);set32(list+8,get32(list+8)-1);release(node);}
 void clearLinked(U list){U sentinel=get32(list+4),node=get32(sentinel);while(node!=sentinel){U next=get32(node);erase(list,node);node=next;}}
 void rebuildKernels(U p){event(0x4f1890,{p,get32(0x5e41a8)});}
 void publishExtents(U x,U y,U z){event(0x545570,{x,y,z});}
} memory;
#ifdef ORIGINAL_REFERENCE
struct PrivateFs {
 unsigned short old=0;U head=~U(0);int index=-1;
 PrivateFs(){asm volatile("mov %%fs,%0":"=r"(old));user_desc d{};d.entry_number=unsigned(-1);d.base_addr=U(std::uintptr_t(&head));d.limit=3;d.seg_32bit=1;d.useable=1;if(syscall(SYS_set_thread_area,&d))throw std::runtime_error("Private FS");index=d.entry_number;unsigned short s=static_cast<unsigned short>((index<<3)|3);asm volatile("mov %0,%%fs"::"r"(s):"memory");}
 ~PrivateFs(){asm volatile("mov %0,%%fs"::"r"(old):"memory");}
};
extern "C" U svc_allocate(U n){return memory.allocate(n);}
extern "C" void svc_release(U p){memory.release(p);}
extern "C" U svc_read(void* p,U s,U n,U f){U at=U(std::uintptr_t(p));if(!((at>=0x400000 && at<0x800000) || (at>=Arena && at<Arena+0x400000)))at=0;return memory.readRaw(p,at,s,n,f);}
extern "C" U __attribute__((fastcall)) svc_section(void* p,void*,U stream){return memory.loadSection(U(std::uintptr_t(p)),stream);}
extern "C" U __attribute__((fastcall)) svc_tail(void* p,void*,U stream){return memory.loadTail(U(std::uintptr_t(p)),stream);}
extern "C" void __attribute__((fastcall)) svc_kernel(void* p,void*){memory.rebuildKernels(U(std::uintptr_t(p)));}
extern "C" void __attribute__((fastcall)) svc_extent(U x,U y,U z){memory.publishExtents(x,y,z);}
extern "C" U __attribute__((fastcall)) svc_record_init(void* p,void*){memory.record.fill(0);std::memset(p,0,24);memory.event(0x464c50,{});return U(std::uintptr_t(p));}
extern "C" void __attribute__((fastcall)) svc_record_bind(void*,void*,U v){memory.event(0x464ca0,{v});}
extern "C" U __attribute__((fastcall)) svc_record_load(void* p,void*,U stream){U cell;std::memcpy(&cell,static_cast<unsigned char*>(p)-4,4);U n=memory.loadLinkedRecord(cell,stream);std::memcpy(p,memory.record.data()+4,24);return n;}
extern "C" U __attribute__((fastcall)) svc_node(void* p,void*,U sentinel,U prev){return memory.allocateLinkedNode(U(std::uintptr_t(p)),sentinel,prev);}
extern "C" void __attribute__((fastcall)) svc_copy(void* target,void* source){std::memcpy(memory.record.data(),source,28);memory.copyLoadedRecord(U(std::uintptr_t(target)));}
extern "C" void __attribute__((fastcall)) svc_erase(void* p,void*,U*,U node){memory.erase(U(std::uintptr_t(p)),node);}
#include "refresh-adapter-bytes.hpp"
static void redirect(U at,std::uintptr_t target){const auto& expected=adapterBytes.at(at);auto* p=memory.ptr(at);if(std::memcmp(p,expected.data(),expected.size()))throw std::runtime_error("Adapter bytes");p[0]=0xe9;U delta=U(target)-at-5;std::memcpy(p+1,&delta,4);}
static void protect(const Bytes& pe){U head=u32(pe,0x3c),opt=head+24;unsigned n=pe[head+6]|(pe[head+7]<<8),os=pe[head+20]|(pe[head+21]<<8);for(unsigned i=0;i<n;++i){U s=opt+os+i*40;if(u32(pe,s+36)&0x20000000){U rva=u32(pe,s+12),size=(std::max(u32(pe,s+8),u32(pe,s+16))+4095)&~4095u;if(mprotect(memory.ptr(0x400000+rva),size,PROT_READ|PROT_EXEC))throw std::runtime_error("RX protection");}}}
#endif
int main(int argc,char** argv)try{
#ifdef ORIGINAL_REFERENCE
 if(argc!=3)throw std::runtime_error("Expected PE output");auto pe=read(argv[1]);map_image(pe);if(mmap(reinterpret_cast<void*>(Arena),0x400000,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED_NOREPLACE,-1,0)==MAP_FAILED)throw std::runtime_error("Arena");
#define REDIRECT(at,name) redirect(at,reinterpret_cast<std::uintptr_t>(&name))
 REDIRECT(0x597850,svc_allocate);REDIRECT(0x597870,svc_release);REDIRECT(0x59c359,svc_read);REDIRECT(0x533550,svc_section);REDIRECT(0x49f490,svc_tail);REDIRECT(0x4f1890,svc_kernel);REDIRECT(0x545570,svc_extent);
 REDIRECT(0x464c50,svc_record_init);REDIRECT(0x464ca0,svc_record_bind);REDIRECT(0x4652a0,svc_record_load);REDIRECT(0x502490,svc_node);REDIRECT(0x5027e0,svc_copy);REDIRECT(0x4bfeb0,svc_erase);protect(pe);const char* output=argv[2];PrivateFs fs;
#else
 if(argc!=2)throw std::runtime_error("Expected output");const char* output=argv[1];
#endif
 std::ofstream out(output,std::ios::binary);std::uint64_t bytes=0;U cases=0,successes=0,failures=0;
 const auto emit=[&](const void* p,std::size_t n){out.write(static_cast<const char*>(p),n);bytes+=n;};const auto word=[&](U v){emit(&v,4);};
 const auto append=[&](U v){for(U i=0;i<4;++i)memory.input.push_back(static_cast<unsigned char>(v>>(i*8)));};
 for(U trial=0;trial<512;++trial){
  memory.input.clear();memory.events.clear();memory.cursor=memory.used=memory.reads=memory.allocations=memory.sectionCalls=0;memory.failRead=memory.failAllocation=memory.failSection=memory.failTail=0;
  std::memset(memory.ptr(Map),0x69,0x66c0);memory.set32(Map+0x4c,Initial);for(U i=0;i<5;++i)memory.set32(Map+0x7cc+i*4,Initial+0x100+i*0x100);
  memory.set32(Map+0x7e8,Sentinel);memory.set32(Sentinel,Sentinel);memory.set32(Sentinel+4,Sentinel);memory.set32(Map+0x7ec,trial%4);
  for(U i=0;i<trial%4;++i){U node=Initial+0x800+i*0x40;std::memset(memory.ptr(node),0x72,36);memory.set32(node,Sentinel);memory.set32(node+4,memory.get32(Sentinel+4));memory.set32(memory.get32(Sentinel+4),node);memory.set32(Sentinel+4,node);}
  memory.set32(0x5e41a8,0x80);memory.set32(0x6e1fa0,77);memory.set32(0x6e1fa4,88);memory.set32(0x5e1780,99);memory.set32(0x5e1784,11);memory.set32(0x6c54dc,Initial);
  const U cells=1+trial%4,layers=1+trial%5,area=19*21,count=((layers+1)/2)*area,records=trial%3;
  Bytes header(0x4c,0);auto set=[&](U off,U value){std::memcpy(header.data()+off,&value,4);};set(4,19);set(8,21);set(12,layers);set(16,area);set(20,cells);memory.input=header;
  for(U i=0;i<cells*12;++i)memory.input.push_back(static_cast<unsigned char>(i*13));append(7);
  for(U i=0;i<(0x4b+0x78+0x78+0x1c+0x7b)*4;++i)memory.input.push_back(static_cast<unsigned char>(i*17));
  append(trial%32==31?count+1:count);for(U i=0;i<count;++i)memory.input.push_back(static_cast<unsigned char>(i));append(records);for(U i=0;i<records;++i)append(i?~U(0):0);for(U i=0;i<5;++i)append(0x112200+i);
  U group=trial%8;if(group==1)memory.failAllocation=1+(trial/8)%6;else if(group==2)memory.failRead=1+(trial/8)%24;else if(group==3)memory.failSection=1+(trial/8)%2;else if(group==4)memory.failTail=1;
  U result;
#ifdef ORIGINAL_REFERENCE
  using Load=U(__attribute__((thiscall))*)(void*,U);result=reinterpret_cast<Load>(0x501e00)(memory.ptr(Map),Stream);if(fs.head!=~U(0))throw std::runtime_error("Original FS chain not restored");
#else
  result=TerrainLightMapLoad<Memory>(memory).load(Map,Stream);
#endif
  word(result);word(memory.cursor);word(memory.get32(0x5e41a8));word(memory.get32(0x6e1fa0));word(memory.get32(0x6e1fa4));word(memory.get32(0x5e1780));word(memory.get32(0x5e1784));emit(memory.ptr(Map),0x66c0);emit(memory.ptr(Sentinel),8);emit(memory.ptr(Initial+0x800),0xc0);word(memory.used);emit(memory.ptr(Arena+0x10000),memory.used);word(U(memory.events.size()));emit(memory.events.data(),memory.events.size()*4);
  ++cases;if(result==~U(0))++failures;else ++successes;
 }
#ifdef ORIGINAL_REFERENCE
 for(const auto& v:protectedBytes)if(std::memcmp(memory.ptr(v.first),v.second.data(),v.second.size()))throw std::runtime_error("Original body changed");
#endif
 if(!out)throw std::runtime_error("Output");std::cout<<"{\"success\":true,\"cases\":"<<cases<<",\"successful_loads\":"<<successes<<",\"failed_loads\":"<<failures<<",\"compared_bytes\":"<<bytes<<"}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
