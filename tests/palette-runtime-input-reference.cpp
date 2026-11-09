// Reconstruct directly observed original builder inputs independently of its
// table bytes. Profile-dependent modes require a separate API transcript and
// are explicitly skipped; they cannot supply a cycling acceptance witness.
#include "../reconstruction/rendering/palette_effects.hpp"
#include "../reconstruction/rendering/screen_palette_providers.hpp"
#include <array>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <cstdlib>
#include <vector>
using Bytes=std::vector<unsigned char>;
static void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
static unsigned word(const Bytes& b,std::size_t at){require(at<=b.size()&&b.size()-at>=4,"DWORD extent");return unsigned(b[at])|unsigned(b[at+1])<<8|unsigned(b[at+2])<<16|unsigned(b[at+3])<<24;}
static double real(const Bytes& b,std::size_t at){require(at<=b.size()&&b.size()-at>=8,"binary64 extent");double out;std::memcpy(&out,b.data()+at,8);return out;}
struct FloatingContext {
 unsigned short saved;unsigned savedSse;
 FloatingContext(unsigned control,unsigned sse){require(control<=65535&&sse<=65535,"Floating context extent");const auto cw=static_cast<unsigned short>(control);asm volatile("fnstcw %0;stmxcsr %1":"=m"(saved),"=m"(savedSse));asm volatile("fldcw %0;ldmxcsr %1"::"m"(cw),"m"(sse):"memory");}
 ~FloatingContext(){asm volatile("fldcw %0;ldmxcsr %1"::"m"(saved),"m"(savedSse):"memory");}
};
struct RefreshMemory {
 static constexpr unsigned Node=0x100000,Rgb=0x200000,Globals=0x5f18d0;
 Bytes node,rgb,globals;unsigned format=0;
 unsigned char& byte(unsigned p){if(p>=Node&&p-Node<node.size())return node[p-Node];if(p>=Rgb&&p-Rgb<rgb.size())return rgb[p-Rgb];if(p>=Globals&&p-Globals<globals.size())return globals[p-Globals];throw std::runtime_error("Independent refresh memory extent");}
 unsigned get8(unsigned p){return byte(p);}
 unsigned get32(unsigned p){if(p==0x6e1f88)return format;unsigned v=0;for(unsigned i=0;i<4;++i)v|=unsigned(byte(p+i))<<(8*i);return v;}
 void set16(unsigned p,unsigned v){for(unsigned i=0;i<2;++i)byte(p+i)=static_cast<unsigned char>(v>>(8*i));}
 void set32(unsigned p,unsigned v){for(unsigned i=0;i<4;++i)byte(p+i)=static_cast<unsigned char>(v>>(8*i));}
};
static Bytes encodedNode(const mnm::reconstruction::PaletteEffectResult& result,unsigned stride){
 Bytes out(12+result.palette.tables.size()*256*stride);auto put=[&](unsigned at,unsigned value,unsigned n){for(unsigned i=0;i<n;++i)out[at+i]=static_cast<unsigned char>(value>>(8*i));};put(4,result.palette.shift,4);put(8,result.palette.neutral,4);unsigned at=12;for(const auto& table:result.palette.tables)for(unsigned value:table){put(at,value,stride);at+=stride;}return out;
}
int main(int argc,char** argv)try{
 require(argc==2||argc==3||argc==4,"Retained original observation and optional context/profile input required");std::ifstream in(argv[1],std::ios::binary);require(bool(in),"Observation input");Bytes raw{std::istreambuf_iterator<char>(in),{}};
 const bool version2=raw.size()>=64&&((std::memcmp(raw.data(),"MNMPAL02",8)==0&&word(raw,8)==2)||(std::memcmp(raw.data(),"MNMPAL03",8)==0&&word(raw,8)==3));
 require(raw.size()>=64&&raw.size()<=128*1024*1024+128&&(version2||(std::memcmp(raw.data(),"MNMPAL01",8)==0&&word(raw,8)==1)),"Observation envelope");require(version2||argc==3,"Historical v1 lacks observed floating context; explicit diagnostic control required");
 const unsigned authored=argc==3?static_cast<unsigned>(std::stoul(argv[2],nullptr,0)):0;
 require(!version2||argc!=3,"Observed context must not be overridden");
 std::map<unsigned,mnm::reconstruction::PaletteEffectProfile> profiles;
 if(argc==4){
  require(std::string(argv[2])=="--profiles","Profile input option");std::ifstream profileFile(argv[3],std::ios::binary);require(bool(profileFile),"Actual profile transcript input");Bytes bytes{std::istreambuf_iterator<char>(profileFile),{}};require(bytes.size()>=128&&!std::memcmp(bytes.data(),"MNMPPR01",8),"Profile transcript envelope");
  struct Property{unsigned mode,result;std::array<std::string,5> text;};std::map<unsigned,std::vector<Property>> groups;
  for(std::size_t at=64;at+64<=bytes.size()&&std::memcmp(bytes.data()+at,"MNMPPEND",8);at+=1344){require(word(bytes,at)==1344&&bytes.size()-at>=1344,"Profile record extent");Property p{};p.mode=word(bytes,at+12);p.result=word(bytes,at+36);for(unsigned i=0;i<5;++i){const char* data=reinterpret_cast<const char*>(bytes.data()+at+64+i*256);const auto end=static_cast<const char*>(std::memchr(data,0,256));require(end,"Profile string extent");p.text[i]=std::string(data,end);}require(p.text[4].size()==p.result,"Profile result text extent");groups[word(bytes,at+8)].push_back(p);}
  for(const auto& pair:groups){auto& profile=profiles[pair.first];const auto& rows=pair.second;require(!rows.empty(),"Empty profile group");const unsigned mode=rows[0].mode;const char* section=mode==4?"STONE_EFFECT":mode==5?"METAL_EFFECT":"REDTINT_EFFECT";const char* keys[]={"Reducer","SpecalValue","Darkest","BlueTint","Contrast"};const unsigned count=mode==4?3:mode==5?5:1;require(rows.size()==count,"Incomplete actual profile queries");
   double* stone[]={&profile.stoneReducer,&profile.stoneSpecalValue,&profile.stoneDarkest};double* metal[]={&profile.metalReducer,&profile.metalSpecalValue,&profile.metalDarkest,&profile.metalBlueTint,&profile.metalContrast};
   for(unsigned i=0;i<count;++i){const auto& row=rows[i];require(row.mode==mode&&row.text[0]==section&&row.text[1]==keys[i],"Actual profile query order/identity");if(row.result){const double value=std::strtod(row.text[4].c_str(),nullptr);if(mode==4)*stone[i]=value;else if(mode==5)*metal[i]=value;else profile.redTintReducer=value;}}
  }
 }
 struct Input{unsigned mode,count,format,control,sse,entry;Bytes rgb;mnm::reconstruction::PaletteShadingConfig lighting;};
 std::vector<Input> pending;std::map<unsigned,Bytes> ownedNodes;struct Refresh{unsigned id,count,control,sse,entry;std::array<unsigned,6> tint;RefreshMemory memory;};std::vector<Refresh> refreshing;std::array<unsigned,10> modes{};unsigned compared=0,skipped=0,refreshed=0;std::uint64_t refreshedWords=0;std::uint64_t words=0;std::size_t at=64;
 while(at<raw.size()){
  require(raw.size()-at>=128,"Complete observation record");unsigned size=word(raw,at),kind=word(raw,at+8);require(size>=128&&size<=raw.size()-at,"Record extent");
  if(kind==7){
   require(version2&&word(raw,8)==3,"Refresh version");Refresh value{};value.id=word(raw,at+32);value.count=word(raw,at+36);value.control=word(raw,at+64);value.sse=word(raw,at+68);value.entry=word(raw,at+52)==12?0x584430:0x589280;require(ownedNodes.count(value.id),"Refresh lacks independent initial model");value.memory.node=ownedNodes.at(value.id);const auto bytes=word(raw,at+44);require(size==128+872+bytes&&value.memory.node.size()==bytes&&std::memcmp(value.memory.node.data(),raw.data()+at+128+872,bytes)==0,"Independent refresh preimage differs");
   value.memory.format=word(raw,at+40);value.memory.rgb.assign(raw.begin()+at+128,raw.begin()+at+128+768);value.memory.globals.assign(raw.begin()+at+128+768,raw.begin()+at+128+848);for(unsigned i=0;i<6;++i)value.tint[i]=word(raw,at+128+848+4*i);refreshing.push_back(value);
  }else if(kind==8){
   require(!refreshing.empty(),"Refresh return without independent input");auto value=refreshing.back();refreshing.pop_back();require(word(raw,at+32)==value.id&&word(raw,at+36)==value.count,"Refresh input identity");
   const auto result=[&]{FloatingContext context(value.control,value.sse);mnm::reconstruction::ScreenPaletteProviders<RefreshMemory> model(value.memory);return model.tryRun(value.entry,value.count,RefreshMemory::Node,{RefreshMemory::Rgb,value.tint[0],value.tint[1],value.tint[2],value.tint[3],value.tint[4],value.tint[5]});}();
   require(result&&*result==word(raw,at+60)&&*result==1,"Original/native refresh return differs");const unsigned bytes=word(raw,at+44);require(size==128+bytes+768&&value.memory.node.size()==bytes,"Refresh output extent");require(std::memcmp(value.memory.node.data(),raw.data()+at+128,bytes)==0,"Independent refreshed table differs");require(std::memcmp(value.memory.rgb.data(),raw.data()+at+128+bytes,768)==0,"Refresh changed RGB input");ownedNodes[value.id]=value.memory.node;++refreshed;refreshedWords+=(bytes-12)/(value.entry==0x584430?2:4);
  }else if(kind==1){
   require(size==976,"Observed RGB/global input extent");Input value{};value.entry=word(raw,at+4);value.mode=word(raw,at+52);value.count=word(raw,at+36);value.format=word(raw,at+40);value.control=version2?word(raw,at+64):authored;value.sse=version2?word(raw,at+68):0x1f80;require(value.mode<10,"Original mode");value.rgb.assign(raw.begin()+at+128,raw.begin()+at+896);
   const auto settings=at+128+768+((value.mode==1||value.mode==9)?40:0);value.lighting.count=value.count;value.lighting.intensityLevel=static_cast<std::int32_t>(word(raw,settings));value.lighting.saturationLevel=static_cast<std::int32_t>(word(raw,settings+4));value.lighting.intensityPower=real(raw,settings+8);value.lighting.saturationPower=real(raw,settings+16);pending.push_back(value);
  }else if(kind==2){
   require(!pending.empty(),"Original return without input");const auto value=pending.back();pending.pop_back();require(value.mode==word(raw,at+52)&&value.count==word(raw,at+36)&&value.format==word(raw,at+40),"Original input/return binding");
   if(!word(raw,at+32)){at+=size;continue;}
   if((value.mode==4||value.mode==5||value.mode==7)&&!profiles.count(value.entry)){++skipped;at+=size;continue;}
   mnm::reconstruction::PaletteRgb rgb{};std::memcpy(rgb.data(),value.rgb.data(),768);
   // No original table, selected colour or post-call source enters the model.
   const auto profile=profiles.count(value.entry)?profiles.at(value.entry):mnm::reconstruction::PaletteEffectProfile{};
   const auto actual=[&]{FloatingContext context(value.control,value.sse);return mnm::reconstruction::buildPaletteEffect(rgb,value.mode,value.lighting,profile,value.format!=0);}();
   const unsigned stride=value.mode<8?2:4,nodeBytes=word(raw,at+44);require(size==128+nodeBytes+768&&nodeBytes==12+actual.palette.tables.size()*256*stride,"Independent table extent");
   require(word(raw,at+128)==0&&word(raw,at+132)==actual.palette.shift&&word(raw,at+136)==actual.palette.neutral,"Independent node header differs");
   auto p=at+140;for(const auto& table:actual.palette.tables)for(auto colour:table){unsigned expected=unsigned(raw[p])|unsigned(raw[p+1])<<8;if(stride==4)expected|=unsigned(raw[p+2])<<16|unsigned(raw[p+3])<<24;
    if(expected!=colour){std::cerr<<"node="<<word(raw,at+32)<<" mode="<<value.mode<<" byte="<<p<<" actual="<<colour<<" expected="<<expected<<'\n';throw std::runtime_error("Independent runtime-input table mismatch");}p+=stride;++words;}
   require(std::memcmp(actual.source.data(),raw.data()+at+128+nodeBytes,768)==0,"Independent source mutation differs");ownedNodes[word(raw,at+32)]=encodedNode(actual,stride);++compared;++modes[value.mode];
  }
  at+=size;
 }
 require(pending.empty()&&refreshing.empty()&&compared>0,"Missing normal-return input comparison");
 std::cout<<"{\"success\":true,\"compared_nodes\":"<<compared<<",\"compared_words\":"<<words<<",\"profile_dependent_nodes_skipped\":"<<skipped<<",\"observed_refreshes\":"<<refreshed<<",\"refreshed_words\":"<<refreshedWords<<",\"original_tables_used_as_model_input\":false,\"authored_floating_context\":"<<(version2?"false":"true")<<",\"modes\":[";
 for(unsigned i=0;i<10;++i){if(i)std::cout<<',';std::cout<<modes[i];}std::cout<<"]}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
