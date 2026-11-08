// Diagnose real original observations under bounded x87 control words. All
// original builder/CRT instructions execute unchanged; only the established
// byte-guarded private allocator endpoint is supplied. No game starts here.
#define ORIGINAL_REFERENCE
#define MNM_PALETTE_EFFECTS_FIXTURE_NO_MAIN
#include "palette-effects-reference.cpp"
struct ObservedPalette {unsigned mode,count,format,id;Bytes input,output;};
int main(int argc,char** argv)try{
 require(argc==3,"Pinned PE and retained observation required");map_image(read(argv[1]));
 const unsigned char prefix[]={0x53,0x8b,0x0d,0x94,0x20,0x5f,0};auto* entry=reinterpret_cast<unsigned char*>(0x597880);require(!std::memcmp(entry,prefix,sizeof(prefix)),"Original allocator bytes");entry[0]=0xe9;const auto delta=std::uint32_t(reinterpret_cast<std::uintptr_t>(&allocate_palette)-0x597885);std::memcpy(entry+1,&delta,4);
 PrivateFs fs;auto raw=read(argv[2]);require(raw.size()>=64&&!std::memcmp(raw.data(),"MNMPAL01",8),"Observed input envelope");std::vector<ObservedPalette> cases,pending;
 for(std::size_t at=64;at<raw.size();){const unsigned size=u32(raw,at),kind=u32(raw,at+8);require(size>=128&&size<=raw.size()-at,"Observed record extent");
  if(kind==1){require(size==976,"Observed builder input");pending.push_back({u32(raw,at+52),u32(raw,at+36),u32(raw,at+40),0,Bytes(raw.begin()+at+128,raw.begin()+at+size),{}});}
  if(kind==2){require(!pending.empty(),"Observed return");auto value=pending.back();pending.pop_back();value.id=u32(raw,at+32);require(value.mode==u32(raw,at+52)&&value.count==u32(raw,at+36),"Original call binding");if(value.id&&value.mode!=4&&value.mode!=5&&value.mode!=7){value.output.assign(raw.begin()+at+128,raw.begin()+at+size);cases.push_back(value);}}
  at+=size;
 }
 require(!pending.size()&&!cases.empty(),"Original normal-return cases missing");unsigned short saved;asm volatile("fnstcw %0":"=m"(saved));
 std::cout<<"{\"success\":true,\"cases_per_control\":"<<cases.size()<<",\"controls\":[";bool first=true;
 for(unsigned precision:{0u,0x200u,0x300u})for(unsigned rounding:{0u,0x400u,0x800u,0xc00u}){
  const unsigned short control=static_cast<unsigned short>(0x7f|precision|rounding);unsigned mismatchedNodes=0;std::uint64_t mismatchWords=0,checkedWords=0;unsigned firstNode=0,firstWord=0,firstExpected=0,firstActual=0;
  for(const auto& c:cases){
   storage.clear();requested=0;PaletteRgb source{};std::memcpy(source.data(),c.input.data(),768);std::memcpy(reinterpret_cast<void*>(0x5f18d0),c.input.data()+768,80);global(0x6e1f88,c.format);void* node=nullptr;
   asm volatile("fldcw %0"::"m"(control));using Build=unsigned(__attribute__((fastcall)) *)(unsigned,void**,const void*);const auto accepted=reinterpret_cast<Build>(builders[c.mode])(c.count,&node,source.data());asm volatile("fldcw %0"::"m"(saved));
   require(accepted&&node==storage.data(),"Original allocation/return");const unsigned nodeBytes=unsigned(c.output.size()-768),stride=c.mode<8?2:4;require(nodeBytes<=storage.size()&&std::memcmp(source.data(),c.output.data()+nodeBytes,768)==0,"Original replay source differs");bool mismatch=false;
   require(!std::memcmp(storage.data(),c.output.data(),12),"Original replay node header differs");
   for(unsigned at=12;at<nodeBytes;at+=stride){unsigned actual=0,expected=0;std::memcpy(&actual,storage.data()+at,stride);std::memcpy(&expected,c.output.data()+at,stride);++checkedWords;if(actual!=expected){mismatch=true;++mismatchWords;if(!firstNode){firstNode=c.id;firstWord=(at-12)/stride;firstExpected=expected;firstActual=actual;}}}
   mismatchedNodes+=mismatch;
  }
  if(!first)std::cout<<',';first=false;std::cout<<"{\"control_word\":"<<control<<",\"mismatched_nodes\":"<<mismatchedNodes<<",\"mismatched_words\":"<<mismatchWords<<",\"checked_words\":"<<checkedWords<<",\"first_node\":"<<firstNode<<",\"first_word\":"<<firstWord<<",\"first_expected\":"<<firstExpected<<",\"first_actual\":"<<firstActual<<'}';
 }
 fs.check();std::cout<<"]}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
