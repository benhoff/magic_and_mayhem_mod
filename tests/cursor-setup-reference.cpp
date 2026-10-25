#define main cursor_unused_surface_main
#include "surface-retry-reference.cpp"
#undef main
#include "../reconstruction/rendering/cursor_setup.hpp"
#include <cstdlib>
#include <string>
namespace C=mnm::reconstruction::cursor;
struct Profile {std::string text;U result;};
static unsigned caseId,profileIndex,allocationIndex;
static std::string directory;
static std::vector<std::string> trace;
static void check(bool b,const char* text){if(!b)throw std::runtime_error(text);}
static void note(const std::string& text){check(trace.size()<4096,"Setup endpoint budget");trace.push_back(text);}
static U get(U at){U value;std::memcpy(&value,reinterpret_cast<void*>(at),4);return value;}
static void put(U at,U value){std::memcpy(reinterpret_cast<void*>(at),&value,4);}
static const char* text(U at){return reinterpret_cast<const char*>(at);}
static Profile profile(const std::string& section,const std::string& key,const std::string& fallback,const std::string& path){
 note("profile:"+section+":"+key+":"+fallback+":"+path);unsigned n=profileIndex++;
 static const char* values[]={"-7","0","1","39","40","41","999","1000","1001"," 17", "+5"};
 if(n<2)return {values[(caseId+n*3)%11],(caseId%3)?1u:0u};
 unsigned field=(n-2)%4,type=(n-2)/4;check(type<22,"Profile denominator");
 if(field==2){
  const std::string separator(1,*text(0x5da820));check(separator[0]!=0,"Nonempty original frame delimiters");
  std::string many;for(unsigned frame=1;frame<=14;++frame){if(frame>1)many+=separator;many+=std::to_string(frame);}
  const std::string frames[]={"","0","1=9"+separator+"2=8"+separator+"-3=7",many,separator+separator+"4"+separator+separator+"6"+separator,"-2=5"};
  return {frames[(caseId+type)%6],(caseId+type)%5?1u:0u};
 }
 return {values[(caseId+field+type)%11],(caseId+field+type)%4?1u:0u};
}
static I integer(const char* s){note("integer:"+std::string(s));return I(std::strtol(s,nullptr,10));}
static char* tokenize(char* s,const char* delimiters){char* out=std::strtok(s,delimiters);note("token:"+std::to_string(s!=nullptr)+":"+delimiters+":"+(out?out:"<null>"));return out;}
static char* find(char* s,I c){note("find:"+std::string(s)+":"+std::to_string(c));return std::strchr(s,c);}
static void allocate(U wrapper,const std::array<U,27>& d,U file,U line){
 std::string row="allocate:"+std::to_string(wrapper)+":"+std::to_string(file)+":"+std::to_string(line);for(auto v:d)row+=":"+std::to_string(v);note(row);
 ++allocationIndex;if(caseId%4==allocationIndex)return;put(wrapper+4,0x1000+allocationIndex);put(wrapper+8,0x2000+allocationIndex);
}
static void fill(U wrapper,U color){note("fill:"+std::to_string(wrapper)+":"+std::to_string(color));}
static void key(U wrapper,U red,U green,U blue){note("key:"+std::to_string(wrapper)+":"+std::to_string(red)+":"+std::to_string(green)+":"+std::to_string(blue));}
static void bitmap(U wrapper,U path){note("bitmap:"+std::to_string(wrapper)+":"+text(path));}
static U mutex(){note("mutex");return caseId%2?0x12345678:0;}
static U __attribute__((stdcall)) osDirectory(U size,char* out){check(size==256,"Directory capacity");note("directory");std::strcpy(out,directory.c_str());return caseId%2?U(directory.size()):0;}
static U __attribute__((stdcall)) osProfile(const char* section,const char* key,const char* fallback,char* out,U size,const char* path){check(size==256,"Profile capacity");auto v=profile(section,key,fallback,path);std::strcpy(out,v.text.c_str());return v.result;}
static char* decimal(U n){static char bytes[24];note("decimal:"+std::to_string(n));std::strcpy(bytes,std::to_string(n).c_str());return bytes;}
static char* itoa(U n,char* out,I radix){check(radix==10,"Decimal radix");std::strcpy(out,decimal(n));return out;}
static void __attribute__((thiscall)) allocationCall(U wrapper,const U* d,U file,U line){std::array<U,27> desc;std::copy(d,d+27,desc.begin());allocate(wrapper,desc,file,line);}
static void __attribute__((thiscall)) fillCall(U wrapper,U color){fill(wrapper,color);}
static void __attribute__((thiscall)) keyCall(U wrapper,U r,U g,U b){key(wrapper,r,g,b);}
static void __attribute__((thiscall)) bitmapCall(U wrapper,U path){bitmap(wrapper,path);}
static U __attribute__((stdcall)) mutexCall(U a,U b,U c){check(!a&&!b&&!c,"Mutex arguments");return mutex();}
struct Memory {U get32(U at){return get(at);}void set32(U at,U v){put(at,v);}};
struct Services {
 const char* text(U at){return ::text(at);}
 std::string currentDirectory(){char out[256];osDirectory(256,out);return out;}
 Profile profile(const std::string& s,const std::string& k,const std::string& f,const std::string& p){return ::profile(s,k,f,p);}
 I integer(const char* s){return ::integer(s);}
 std::string decimal(U n){return ::decimal(n);}
 char* tokenize(char* s,const char* d){return ::tokenize(s,d);}char* find(char* s,I c){return ::find(s,c);}
 void allocate(U w,const std::array<U,27>& d,U file,U line){::allocate(w,d,file,line);}void fill(U w,U c){::fill(w,c);}
 void key(U w,U r,U g,U b){::key(w,r,g,b);}void bitmap(U w,U p){::bitmap(w,p);}U createMutex(){return mutex();}
};
static std::vector<U> state(){std::vector<U> out;for(U at=0x642678;at<0x642678+22*68;at+=4)out.push_back(get(at));for(U at:{0x641a8cu,0x642c50u,0x642c58u,0x642028u,0x641fccu,0x641fd0u,0x642004u,0x642008u})out.push_back(get(at));return out;}
static void reset(){trace.clear();profileIndex=allocationIndex=0;for(U at=0x642678;at<0x642678+22*68;at+=4)put(at,0x12340000u+at);for(U at:{0x641a8cu,0x642c50u,0x642c58u,0x642028u,0x641fccu,0x641fd0u,0x642004u,0x642008u})put(at,0xabcdef01);}
static void redirect(U entry,void* function){auto* bytes=reinterpret_cast<unsigned char*>(entry);bytes[0]=0xe9;const U rel=reinterpret_cast<U>(function)-entry-5;std::memcpy(bytes+1,&rel,4);}
int main(int argc,char** argv)try{
 check(argc==2,"Pinned original image required");mapImage(argv[1]);
 std::array<unsigned char,1643> original;std::memcpy(original.data(),reinterpret_cast<void*>(0x4a30b0),original.size());
 put(0x5c50bc,reinterpret_cast<U>(&osDirectory));put(0x5c507c,reinterpret_cast<U>(&osProfile));put(0x5c50b8,reinterpret_cast<U>(&mutexCall));
 // Only external endpoint entries redirect. The entire selected setup function
 // remains byte-for-byte unchanged, checked after every differential case.
 for(auto pair:std::vector<std::pair<U,void*>>{{0x59c5a4,reinterpret_cast<void*>(&integer)},{0x59c99c,reinterpret_cast<void*>(&tokenize)},{0x59c8e0,reinterpret_cast<void*>(&find)},{0x59de2f,reinterpret_cast<void*>(&itoa)},{0x58ad90,reinterpret_cast<void*>(&allocationCall)},{0x58bc10,reinterpret_cast<void*>(&fillCall)},{0x58d0e0,reinterpret_cast<void*>(&keyCall)},{0x58d1a0,reinterpret_cast<void*>(&bitmapCall)}})redirect(pair.first,pair.second);
 unsigned cases=0;for(caseId=0;caseId<132;++caseId)for(const auto& path:std::vector<std::string>{"","C:","C:\\game\\","C:/game/"}){
  directory=path;reset();auto result=reinterpret_cast<U(*)()>(0x4a30b0)();auto expected=state();auto calls=trace;check(profileIndex==90&&allocationIndex==2,"Original setup denominator");
  for(U type=0;type<22;++type){
   if((caseId+type)%6==3 && (caseId+type)%5)check(expected[type*17+2]==12,"Original frame cap reached");
   check(expected[type*17+15]==0x12340000u+0x642678+type*68+60,"Unused thirteenth frame word preserved");
  }
  reset();Memory m;Services p;C::Setup model(m,p);check(model.run()==result,"Setup return differs");check(state()==expected,"Setup state differs");
  if(trace!=calls){for(std::size_t i=0;i<std::min(trace.size(),calls.size());++i)if(trace[i]!=calls[i]){std::cerr<<"case "<<caseId<<" event "<<i<<" original="<<calls[i]<<" model="<<trace[i]<<'\n';break;}throw std::runtime_error("Setup endpoint trace differs");}
  check(!std::memcmp(original.data(),reinterpret_cast<void*>(0x4a30b0),original.size()),"Original setup instructions changed");++cases;
 }
 std::cout<<"{\"success\":true,\"cases\":"<<cases<<",\"descriptors_per_case\":22,\"profile_calls_per_case\":90,\"allocation_calls_per_case\":2,\"original_instructions_unchanged\":true}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
