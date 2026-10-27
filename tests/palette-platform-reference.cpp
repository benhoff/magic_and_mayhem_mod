// Actual Wine profile results feed unchanged original builders. No native
// palette algorithm produces the expected tables or source mutations.
#define MNM_PALETTE_EFFECTS_FIXTURE_NO_MAIN
#include "palette-effects-reference.cpp"
#undef MNM_PALETTE_EFFECTS_FIXTURE_NO_MAIN
#include <fstream>

static Bytes transcript;
static const char* sections[]={"STONE_EFFECT","STONE_EFFECT","STONE_EFFECT","METAL_EFFECT","METAL_EFFECT","METAL_EFFECT","METAL_EFFECT","METAL_EFFECT","REDTINT_EFFECT"};
static const char* keys[]={"Reducer","SpecalValue","Darkest","Reducer","SpecalValue","Darkest","BlueTint","Contrast","Reducer"};
static unsigned directoryCalls=0,profileCalls=0;
static unsigned __attribute__((stdcall)) platformDirectory(unsigned capacity,char* out){
 require(capacity==256,"Original directory capacity");++directoryCalls;
 const auto n=u32(transcript,0);require(n<256,"Directory transcript extent");
 std::memcpy(out,transcript.data()+4,256);return n;
}
static unsigned __attribute__((stdcall)) platformProfile(const char* section,const char* key,const char* fallback,char* out,unsigned capacity,const char* path){
 require(capacity==256&&!*fallback,"Original profile admission");
 std::string expected(reinterpret_cast<const char*>(transcript.data()+4));
 if(expected.back()!='/'&&expected.back()!='\\')expected+='\\';
 expected+="cfg\\tables.cfg";require(expected==path,"Original current-directory/path join");
 for(unsigned i=0;i<9;++i)if(!std::strcmp(section,sections[i])&&!std::strcmp(key,keys[i])){
  const auto at=(i+1)*260,n=u32(transcript,at);require(n<256&&transcript.at(at+4+n)==0,"Profile transcript bound");
  ++profileCalls;std::memcpy(out,transcript.data()+at+4,256);return n;
 }
 throw std::runtime_error("Unexpected original profile key");
}
static void put(std::ofstream& out,unsigned value){out.write(reinterpret_cast<const char*>(&value),4);require(bool(out),"Oracle output");}
int main(int argc,char** argv)try{
 require(argc==5,"Expected pinned PE, profile transcript, RGB inputs, oracle output");
 transcript=read(argv[2]);require(transcript.size()==2600,"Ten platform records");
 const auto inputs=read(argv[3]);require(!inputs.empty()&&inputs.size()%768==0&&inputs.size()<=4*768,"RGB input extent");
 map_image(read(argv[1]));
 const unsigned char prefix[]={0x53,0x8b,0x0d,0x94,0x20,0x5f,0x00};
 auto* entry=reinterpret_cast<unsigned char*>(0x597880);require(!std::memcmp(entry,prefix,sizeof(prefix)),"Allocator bytes");
 entry[0]=0xe9;const auto delta=std::uint32_t(reinterpret_cast<std::uintptr_t>(&allocate_palette)-0x597885);std::memcpy(entry+1,&delta,4);
 require(*reinterpret_cast<unsigned*>(6049980)==1926766u&&*reinterpret_cast<unsigned*>(6049916)==1926738u,"Original platform IAT values");
 global(6049980,reinterpret_cast<std::uintptr_t>(&platformDirectory));global(6049916,reinterpret_cast<std::uintptr_t>(&platformProfile));
 PrivateFs fs;std::ofstream out(argv[4],std::ios::binary);require(bool(out),"Oracle file");
 put(out,0x31504650);put(out,unsigned(inputs.size()/768));unsigned cases=0,totalWords=0;
 using Build=unsigned(__attribute__((fastcall)) *)(unsigned,void**,const void*);
 for(unsigned format=0;format<2;++format)for(unsigned p=0;p<inputs.size()/768;++p){
  PaletteRgb rgb;std::memcpy(rgb.data(),inputs.data()+p*768,768);
  for(unsigned mode=0;mode<10;++mode){
   storage.clear();requested=0;void* node=nullptr;lighting({16,30,40,1,1});global(0x6e1f88,format);
   require(reinterpret_cast<Build>(builders[mode])(16,&node,rgb.data())==1,"Original build success");fs.check();
   require(node==storage.data(),"Original allocation identity");
   const unsigned tables=mode<8?15:8,width=mode<8?2:4,produced=12+tables*256*width;
   require(produced<=requested,"Safe original allocation");
   for(std::size_t i=produced;i<storage.size();++i)require(storage[i]==0xa5,"Original allocation tail");
   put(out,format);put(out,p);put(out,mode);put(out,u32(storage,4));put(out,u32(storage,8));put(out,tables);
   out.write(reinterpret_cast<const char*>(rgb.data()),768);
   for(unsigned i=0;i<tables*256;++i){unsigned value=0;std::memcpy(&value,storage.data()+12+i*width,width);require(value<=65535,"Packed original word");put(out,value);++totalWords;}
   ++cases;
  }
 }
 require(directoryCalls==6*(inputs.size()/768)&&profileCalls==18*(inputs.size()/768),"All original platform calls");
 std::cout<<"{\"success\":true,\"cases\":"<<cases<<",\"words\":"<<totalWords<<",\"directory_calls\":"<<directoryCalls<<",\"profile_calls\":"<<profileCalls<<"}\n";
 return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
