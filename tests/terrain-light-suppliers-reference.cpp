// Bounded supplier comparison; original allocator/free/kernel services are explicit
// checked private-image adapters. Supplier bodies and installed inputs stay intact.
#ifdef ORIGINAL_REFERENCE
#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
#else
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
using Bytes=std::vector<unsigned char>;
#endif
#include "terrain_light_suppliers.hpp"
#include <algorithm>
using namespace mnm::reconstruction;
using U=std::uint32_t;
constexpr U Map=0x6c5490,Arena=0x10000000;
struct Memory {
#ifndef ORIGINAL_REFERENCE
 Bytes image=Bytes(0x400000),arena=Bytes(0x100000);
#endif
 U used=0;
 std::vector<U> events;
 unsigned char* ptr(U p) {
#ifdef ORIGINAL_REFERENCE
  return reinterpret_cast<unsigned char*>(std::uintptr_t(p));
#else
  if(p>=0x400000 && p<0x800000)return image.data()+p-0x400000;
  if(p>=Arena && p<Arena+arena.size())return arena.data()+p-Arena;
  throw std::runtime_error("Guest address outside fixture");
#endif
 }
 std::uint8_t get8(U p){return *ptr(p);}
 void set8(U p,std::uint8_t v){*ptr(p)=v;}
 U get32(U p){U v;std::memcpy(&v,ptr(p),4);return v;}
 void set32(U p,U v){std::memcpy(ptr(p),&v,4);}
 U allocate(U n){
  if(used+n+32>0x100000)throw std::runtime_error("Fixture allocation budget");
  U p=Arena+used+16;used+=n+32;events.insert(events.end(),{1,n,p});
  std::memset(ptr(p-16),0xa5,n+32);return p;
 }
 void release(U p){events.insert(events.end(),{2,p});}
 void rebuildKernels(U p){events.insert(events.end(),{3,p});}
} memory;
#ifdef ORIGINAL_REFERENCE
extern "C" U allocate_light(U n){return memory.allocate(n);}
extern "C" void release_light(U p){memory.release(p);}
extern "C" void __attribute__((fastcall)) rebuild_light(void* p,void*){memory.rebuildKernels(U(reinterpret_cast<std::uintptr_t>(p)));}
static void redirect(U address,const Bytes& expected,std::uintptr_t target){
 auto* p=memory.ptr(address);
 if(expected.size()<5 || std::memcmp(p,expected.data(),expected.size()))throw std::runtime_error("Adapter original bytes mismatch");
 p[0]=0xe9;U displacement=U(target)-address-5;std::memcpy(p+1,&displacement,4);
}
#endif
int main(int argc,char** argv)try {
#ifdef ORIGINAL_REFERENCE
 if(argc!=3)throw std::runtime_error("Expected pinned PE and output");
 map_image(read(argv[1]));
 if(mmap(reinterpret_cast<void*>(Arena),0x100000,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED_NOREPLACE,-1,0)==MAP_FAILED)throw std::runtime_error("Arena map failed");
 redirect(0x597850,{0x8b,0x44,0x24,0x04,0x6a,0},reinterpret_cast<std::uintptr_t>(&allocate_light));
 redirect(0x597870,{0x8b,0x44,0x24,0x04,0x85,0xc0},reinterpret_cast<std::uintptr_t>(&release_light));
 redirect(0x4f1890,{0x55,0x8b,0xec,0x83,0xe4,0xf8},reinterpret_cast<std::uintptr_t>(&rebuild_light));
 // Make every executable section RX after the checked service redirects.
 const auto pe=read(argv[1]);const U head=u32(pe,0x3c),opt=head+24;
 const unsigned sections=pe[head+6]|(pe[head+7]<<8),optSize=pe[head+20]|(pe[head+21]<<8);
 for(unsigned i=0;i<sections;++i){U s=opt+optSize+i*40;if(u32(pe,s+36)&0x20000000){U rva=u32(pe,s+12),n=(std::max(u32(pe,s+8),u32(pe,s+16))+4095)&~4095u;if(mprotect(memory.ptr(0x400000+rva),n,PROT_READ|PROT_EXEC))throw std::runtime_error("RX protection failed");}}
 const char* output=argv[2];
#else
 if(argc!=2)throw std::runtime_error("Expected output");
 const char* output=argv[1];
#endif
 std::ofstream out(output,std::ios::binary);
 U bytes=0,relationCases=0,admissions=0,lifetimes=0;
 const auto emit=[&](const void* p,U n){out.write(static_cast<const char*>(p),n);bytes+=n;};
 const auto table=[&](){TerrainLightRelations r;for(U a=0;a<8;++a)for(U b=0;b<8;++b){r.first[a][b]=memory.get8(0x643a80+a*0x154+24+b);r.second[a][b]=memory.get8(0x643a80+a*0x154+32+b);}return r;};
 auto relations=initialTerrainLightRelations();
#ifdef ORIGINAL_REFERENCE
 using OwnerInit=void(__attribute__((thiscall))*)(void*);
 for(U i=0;i<8;++i)reinterpret_cast<OwnerInit>(0x465440)(memory.ptr(0x643a80+i*0x154));
 auto initial=table();if(initial.first!=relations.first || initial.second!=relations.second)throw std::runtime_error("Owner relation initialization differs");
#endif
 for(bool sharing:{false,true})for(U a=0;a<8;++a)for(U b=0;b<8;++b)for(U value:{0u,1u,2u,128u,255u}) {
  // Directed starting bytes establish that setters modify just the selected pair.
  for(U i=0;i<8;++i)for(U j=0;j<8;++j){relations.first[i][j]=(i*13+j*7+value)%256;relations.second[i][j]=(i*19+j*11+value)%256;}
  for(U i=0;i<8;++i)for(U j=0;j<8;++j){memory.set8(0x643a80+i*0x154+24+j,relations.first[i][j]);memory.set8(0x643a80+i*0x154+32+j,relations.second[i][j]);}
  memory.set32(0x5fbf28,0x643a80);memory.set32(0x5e41a8,7);
  setTerrainLightRelation(relations,a,b,std::uint8_t(value),sharing);
#ifdef ORIGINAL_REFERENCE
  using Set=void(__attribute__((thiscall))*)(void*,U,U);
  reinterpret_cast<Set>(sharing?0x4655e0:0x465500)(memory.ptr(0x643a80+a*0x154),b,value);
  auto actual=table();if(actual.first!=relations.first || actual.second!=relations.second)throw std::runtime_error("Relation setter differs");
#endif
  if(memory.get32(0x5e41a8)!=7)throw std::runtime_error("Relation setter requested refresh");
  emit(&relations,sizeof relations);++relationCases;
  // The existing consumer retains directed raw-byte admission after production.
  for(U owner=0;owner<8;++owner)for(U viewer=0;viewer<8;++viewer){bool admitted=admitsTerrainCreatureLight({true,true,int(owner),0,0,0},int(viewer),relations);unsigned char v=admitted;emit(&v,1);++admissions;}
 }
 for(auto pair:{std::pair<unsigned,unsigned>{8,0},{0,8}}){bool refused=false;const auto before=relations;try{setTerrainLightRelation(relations,pair.first,pair.second,1,false);}catch(const std::invalid_argument&){refused=true;}if(!refused || before.first!=relations.first || before.second!=relations.second)throw std::runtime_error("Invalid relation mutation");}
 TerrainLightResourceContracts<Memory> native(memory);
 const auto checkpoint=[&](){emit(memory.ptr(Map),0x66c0);emit(memory.ptr(Arena),memory.used);U n=memory.events.size();emit(&n,4);emit(memory.events.data(),n*4);emit(memory.ptr(0x5e41a8),4);++lifetimes;};
 const auto initialize=[&](){
#ifdef ORIGINAL_REFERENCE
  reinterpret_cast<void(__attribute__((thiscall))*)(void*)>(0x4ecdd0)(memory.ptr(Map));
#else
  native.initialize(Map);
#endif
  checkpoint();};
 const auto clear=[&](){
#ifdef ORIGINAL_REFERENCE
  reinterpret_cast<void(__attribute__((thiscall))*)(void*)>(0x4ecbe0)(memory.ptr(Map));
#else
  native.clear(Map);
#endif
  checkpoint();};
 for(U trial=0;trial<32;++trial){
  memory.used=0;memory.events.clear();std::memset(memory.ptr(Map),0x69,0x66c0);memory.set32(Map+0x4c,0);memory.set32(Map+0x7cc,0);
  memory.set32(Map+4,17+trial%5);memory.set32(Map+8,17+trial%7);memory.set32(Map+12,1+trial%8);memory.set32(Map+16,(17+trial%5)*(17+trial%7));memory.set8(0x5e18f0,std::uint8_t(trial*17));
  initialize(); // five allocations, including signed fill bytes and odd sizes
  memory.set32(Map+12,32);memory.set32(Map+16,1);memory.set8(0x5e18f0,std::uint8_t(trial*29));
  initialize(); // reuse exact handles/count; new dimensions do not resize
  memory.set32(Map+0x4c,memory.allocate(19));clear();clear();
  memory.set32(Map+12,2+trial%7);memory.set32(Map+16,21);initialize();clear();
  // Null publication sentinel preserves sibling handles/count, even inconsistent.
  for(U offset:{0x7d0u,0x7d4u,0x7d8u,0x7dcu,0x7e0u})memory.set32(Map+offset,0x12345678+offset);
  clear();
  memory.set32(0x5e41a8,trial);
#ifdef ORIGINAL_REFERENCE
  reinterpret_cast<void(__attribute__((thiscall))*)(void*)>(0x4f2dd0)(memory.ptr(Map));
#else
  native.requestRefresh();
#endif
  checkpoint();
 }
 if(!out)throw std::runtime_error("Fixture output failed");
 std::cout<<"{\"success\":true,\"relation_cases\":"<<relationCases<<",\"admission_checks\":"<<admissions<<",\"lifetime_checkpoints\":"<<lifetimes<<",\"compared_bytes\":"<<bytes<<",\"kernel_service_recorded\":true,\"allocation_failures_tested\":false}\n";
 return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
