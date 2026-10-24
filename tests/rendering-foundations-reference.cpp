// Actual original selector window and complete original displacement writer.
// Only explicit callees/continuation of the selector receive owned adapters.
#define main unused_sprite_reference_main
#include "sprite-binary-reference.cpp"
#undef main
#include "../reconstruction/rendering/minimap_camera_dispatch.hpp"
#include "../reconstruction/rendering/displacement_table.h"
#include <algorithm>
#include <array>

using U = std::uint32_t;
static std::vector<U> trace;
static U receiver;
extern "C" void original_selector(void*,void*);
__asm__(".text\n.globl original_selector\noriginal_selector:\n"
        "pushl %ebx\npushl %esi\nmovl 12(%esp),%esi\nmovl 16(%esp),%ebx\n"
        "movl $0x555c3c,%eax\ncall *%eax\npopl %esi\npopl %ebx\nret\n");
static void require(bool value, const char* why) {
  if (!value) throw std::runtime_error(why);
}
static void __attribute__((thiscall)) camera0(void* self) { require(U(self)==receiver,"Receiver");trace.push_back(0); }
static void __attribute__((thiscall)) camera1(void* self) { require(U(self)==receiver,"Receiver");trace.push_back(1); }
static void __attribute__((thiscall)) camera2(void* self) { require(U(self)==receiver,"Receiver");trace.push_back(2); }
static void __attribute__((thiscall)) markers(void* self) { require(U(self)==receiver,"Receiver");trace.push_back(4); }
static void adapter(U at, const char* expected, unsigned length, void* target) {
  require(length>=5&&!std::memcmp(reinterpret_cast<void*>(at),expected,length),"Adapter bytes");
  auto* p=reinterpret_cast<unsigned char*>(at);p[0]=0xe9;
  U delta=U(target)-at-5;std::memcpy(p+1,&delta,4);
}
struct Fx { unsigned char bytes[512]; } __attribute__((aligned(16)));
static Fx snapshot() { Fx f{};__asm__ volatile("fxsave %0":"=m"(f));return f; }
static void reset(unsigned short control, bool sticky) {
  __asm__ volatile("fninit; fldcw %0"::"m"(control));
  if(sticky) __asm__ volatile("fldz; fldz; fdivp; fstp %%st(0)":::"st");
}
static void seedFp(unsigned short control,unsigned top,unsigned flags,unsigned conditions) {
  reset(control,false);auto f=snapshot();
  const unsigned short status=(top<<11)|flags|conditions;
  std::memcpy(f.bytes+2,&status,2);
  __asm__ volatile("fxrstor %0"::"m"(f));
}
static void sameFp(const Fx& a,const Fx& b) {
  // CW, complete status including TOP/condition bits, abridged tag, MXCSR.
  require(!std::memcmp(a.bytes,b.bytes,5),"x87 control/status/tag mismatch");
  require(!std::memcmp(a.bytes+24,b.bytes+24,8),"MXCSR mismatch");
}
int main(int argc,char** argv)try{
  require(argc==2,"Expected pinned original image");map_image(read(argv[1]));
  const auto body=[](U at,unsigned n){auto* p=reinterpret_cast<unsigned char*>(at);return Bytes(p,p+n);};
  const Bytes selector=body(0x555c3c,0x32),initializer=body(0x57dbd0,0x46),convert=body(0x59bee0,0x27);
  require(!std::memcmp(selector.data(),"\x8b\x86\xa4\x00\x00\x00\x83\xf8\x03\x77\x20\xff\x24\x85\x08\x5e\x55\x00",18),"Selector bytes");
  require(!std::memcmp(reinterpret_cast<void*>(0x555c6e),"\x8b\x83\x19\x61\x00\x00",6),"Continuation bytes");
  // Private mapped-image mutation; the original selector and jump table stay
  // unchanged. Stop immediately after the real common marker CALL.
  *reinterpret_cast<unsigned char*>(0x555c6e)=0xc3;
  for(auto entry:std::array<std::pair<U,void*>,4>{{{0x5527a0,reinterpret_cast<void*>(camera0)},
      {0x552b50,reinterpret_cast<void*>(camera1)},{0x552f20,reinterpret_cast<void*>(camera2)},
      {0x553850,reinterpret_cast<void*>(markers)}}})
    adapter(entry.first,entry.first==0x553850?"\x83\xec\x08\x53\x55":"\x83\xec\x28\x53\x55\x56",entry.first==0x553850?5:6,entry.second);
  std::array<U,256> owner{},object{};receiver=U(object.data());
  unsigned selectorCases=0;std::uint64_t checksum=0;
  std::vector<U> selectors;
  for(U i=0;i<4096;++i)selectors.push_back(i);
  for(U i=0;i<4096;++i)selectors.push_back(0xffffffffu-i);
  for(U bit=12;bit<32;++bit){selectors.push_back(1u<<bit);selectors.push_back((1u<<bit)-1);}
  for(U value:selectors){
    owner.fill(0x13572468);object.fill(0x24681357);owner[0xa4/4]=value;
    const auto ownerBefore=owner,objectBefore=object;trace.clear();
    original_selector(owner.data(),object.data());
    const auto expected=trace;trace.clear();
    mnm::reconstruction::dispatchOriginalMinimapCamera(value,[](unsigned camera){trace.push_back(camera);},[]{trace.push_back(4);});
    require(trace==expected&&owner==ownerBefore&&object==objectBefore,"Selector trace/storage mismatch");
    for(auto event:trace)checksum=checksum*131+event+1;
    ++selectorCases;
  }
  unsigned tableCases=0,refusals=0;
  std::array<int32_t,18> expected{},actual{};
  for(unsigned precision: {0u,0x200u,0x300u})for(unsigned rounding=0;rounding<4;++rounding)
    for(unsigned top=0;top<8;++top)for(unsigned flags: {0u,1u,2u,4u,8u,16u,32u,63u})
      for(unsigned conditions: {0u,0x100u,0x4400u,0x4700u}){
      const unsigned short cw=0x7f|precision|(rounding<<10);
      expected.fill(0x13572468);actual=expected;
      auto* original=reinterpret_cast<int32_t*>(0x65663c);
      std::copy(expected.begin(),expected.end(),original);
      seedFp(cw,top,flags,conditions);reinterpret_cast<void(*)()>(0x57dbd0)();const Fx reference=snapshot();
      std::copy(original,original+18,expected.begin());
      seedFp(cw,top,flags,conditions);require(mnm_initialize_displacement_table(actual.data()+1,16),"Initializer admission");
      const Fx native=snapshot();sameFp(reference,native);
      require(expected==actual,"All table entries/guards differ");
      for(unsigned i=1;i<=16;++i)checksum=checksum*131+U(actual[i]);
      ++tableCases;
    }
  for(unsigned mode=0;mode<5;++mode){
    reset(mode==3?0x077e:mode==4?0x017f:0x037f,false);
    if(mode==2)__asm__ volatile("fld1":::"st");
    actual.fill(0x12345678);const auto before=actual;const Fx f=snapshot();
    require(!mnm_initialize_displacement_table(mode==0?nullptr:actual.data(),mode==1?15:18),"Invalid state admitted");
    const Fx after=snapshot();sameFp(f,after);require(actual==before,"Refusal mutation");++refusals;
  }
  reset(0x037f,false);
  require(body(0x555c3c,0x32)==selector&&body(0x57dbd0,0x46)==initializer&&body(0x59bee0,0x27)==convert,"Original body changed");
  std::cout<<"{\"success\":true,\"selector_cases\":"<<selectorCases<<",\"table_cases\":"<<tableCases
    <<",\"table_words_compared\":"<<tableCases*18<<",\"atomic_refusals\":"<<refusals<<",\"checksum\":"<<checksum
    <<",\"original_selector_unchanged\":true,\"original_initializer_unchanged\":true,\"live_replacement\":false}\n";
  return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
