// Unchanged original recovery wrappers with explicit scripted COM fault inputs.
// No real DirectDraw loss, driver Restore, reload pixels or native renderer here.
#include <array>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <vector>
#include <sys/mman.h>
using U=std::uint32_t;using I=std::int32_t;
struct Rect {I l,t,r,b;};struct Object {std::array<U,16> words{};};
struct Interface {void** table;U id;};
struct Event {U kind,id,result,flags,a,b;};
static std::vector<Event> events;static std::vector<U> draws;static unsigned drawAt;
static U restores[3],keysResult;static Interface surfaces[3];
static void event(U kind,U id,U result=0,U flags=0,U a=0,U b=0){
 if(events.size()>=128)throw std::runtime_error("Original call budget exceeded");
 events.push_back({kind,id,result,flags,a,b});
}
static U draw(void* target,U op,U flags,U a,U b){
 if(drawAt>=draws.size())throw std::runtime_error("Original exhausted finite result schedule");
 U result=draws[drawAt++];event(op,static_cast<Interface*>(target)->id,result,flags,a,b);return result;
}
static I __attribute__((stdcall)) blt(void* target,Rect* dest,void* source,Rect* rect,U flags,void* effects){
 if(source){if(!dest || !rect || effects)throw std::runtime_error("Copy arguments");return draw(target,1,flags,static_cast<Interface*>(source)->id,0);}
 if(!effects || rect || static_cast<U*>(effects)[0]!=100)throw std::runtime_error("Fill effects");
 return draw(target,1,flags,0,static_cast<U*>(effects)[20]);
}
static I __attribute__((stdcall)) fast(void* target,I,I,void* source,Rect*,U flags){return draw(target,2,flags,static_cast<Interface*>(source)->id,0);}
static I __attribute__((stdcall)) restore(void* object){U id=static_cast<Interface*>(object)->id;event(3,id,restores[id]);return restores[id];}
static I __attribute__((stdcall)) key(void* object,U flags,U* range){U id=static_cast<Interface*>(object)->id;event(4,id,keysResult,flags,range[0],range[1]);return keysResult;}
static void __attribute__((thiscall)) reload(Interface* object,U argument){event(5,object->id,0,argument);}
static I __attribute__((stdcall)) message(void*,const char*,const char*,U){event(6,0);return 1;}
static I __attribute__((stdcall)) destroy(void*){event(7,0);return 0;}
static U word(const std::vector<unsigned char>& b,std::size_t at){
 if(at>b.size() || b.size()-at<4)throw std::runtime_error("PE read extent");
 U v;std::memcpy(&v,b.data()+at,4);return v;
}
static void mapImage(const char* name){
 std::ifstream f(name,std::ios::binary);if(!f)throw std::runtime_error("Cannot open pinned image");
 const std::vector<unsigned char> b((std::istreambuf_iterator<char>(f)),{});
 const U pe=word(b,60),opt=pe+24,length=word(b,opt+56);
 if(word(b,opt+28)!=0x400000 || length>32*1024*1024)throw std::runtime_error("Unexpected image");
 auto* p=static_cast<unsigned char*>(mmap(reinterpret_cast<void*>(0x400000),length,
  PROT_READ|PROT_WRITE|PROT_EXEC,MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED_NOREPLACE,-1,0));
 if(p==MAP_FAILED)throw std::runtime_error("Private image mapping failed");
 const U count=b.at(pe+6)|(U(b.at(pe+7))<<8),size=b.at(pe+20)|(U(b.at(pe+21))<<8);
 for(U i=0;i<count;++i){const U at=opt+size+i*40,rva=word(b,at+12),n=word(b,at+16),raw=word(b,at+20);
  if(std::uint64_t(raw)+n>b.size() || std::uint64_t(rva)+n>length)throw std::runtime_error("PE section extent");
  std::memcpy(p+rva,b.data()+raw,n);
 }
 // No original instruction bytes are patched. Selected entry signatures checked.
 for(U entry:{0x58bac0u,0x58bc10u,0x58c4a0u,0x58c8a0u}){
  if(*reinterpret_cast<unsigned char*>(entry)!=0x83 ||
     *reinterpret_cast<unsigned char*>(entry+1)!=0xec)throw std::runtime_error("Wrapper signature differs");
 }
}
int main(int argc,char** argv)try{
 if(argc!=3)throw std::runtime_error("Expected pinned PE and inputs");
 mapImage(argv[1]);
 *reinterpret_cast<void**>(0x5c520c)=reinterpret_cast<void*>(&message);
 *reinterpret_cast<void**>(0x5c5208)=reinterpret_cast<void*>(&destroy);
 *reinterpret_cast<U*>(0x656614)=0;
 std::array<void*,33> table{};table[5]=reinterpret_cast<void*>(&blt);table[7]=reinterpret_cast<void*>(&fast);
 table[27]=reinterpret_cast<void*>(&restore);table[29]=reinterpret_cast<void*>(&key);
 std::array<void*,1> reloadTable{reinterpret_cast<void*>(&reload)};Interface reloaders[3]={{},{reloadTable.data(),1},{reloadTable.data(),2}};
 surfaces[1]={table.data(),1};surfaces[2]={table.data(),2};
 std::ifstream input(argv[2]);if(!input)throw std::runtime_error("Cannot open cases");
 U id,entry,fastMode,noWait,keyEnabled,reloadBits,rs,rd,kr,count,color;bool first=true;
 std::cout<<"[";
 while(input>>id>>entry>>fastMode>>noWait>>keyEnabled>>reloadBits>>rs>>rd>>kr>>color>>count){
  if(count>16 || reloadBits>3)throw std::runtime_error("Case limits");
  draws.resize(count);for(auto& value:draws)if(!(input>>value))throw std::runtime_error("Incomplete schedule");
  events.clear();drawAt=0;restores[1]=rs;restores[2]=rd;keysResult=kr;
  Object source{},target{};source.words[2]=reinterpret_cast<U>(&surfaces[1]);target.words[2]=reinterpret_cast<U>(&surfaces[2]);
  source.words[11]=0x12345678;target.words[11]=0xabcde123;
  source.words[10]=reloadBits&1?reinterpret_cast<U>(&reloaders[1]):0;target.words[10]=reloadBits&2?reinterpret_cast<U>(&reloaders[2]):0;
  *reinterpret_cast<U*>(0x6f68d8)=fastMode;*reinterpret_cast<U*>(0x6f98d8)=noWait;
  *reinterpret_cast<U*>(0x6f98c0)=0;*reinterpret_cast<U*>(0x6de6dd)=keyEnabled;
  Rect rect{1,1,5,4};
  if(entry==0x58bac0){using Fill=void(__attribute__((thiscall))*)(Object*,Rect*,U);reinterpret_cast<Fill>(entry)(&target,&rect,color);}
  else if(entry==0x58bc10){using Fill=void(__attribute__((thiscall))*)(Object*,U);reinterpret_cast<Fill>(entry)(&target,color);}
  else if(entry==0x58c4a0 || entry==0x58c8a0){using Copy=void(__attribute__((thiscall))*)(Object*,Object*,Rect*,I,I);reinterpret_cast<Copy>(entry)(&source,&target,&rect,2,2);}
  else throw std::runtime_error("Unsupported wrapper");
  if(!first)std::cout<<',';
  first=false;std::cout<<"{\"id\":"<<id<<",\"draws_consumed\":"<<drawAt<<",\"events\":[";
  bool firstEvent=true;for(auto e:events){if(!firstEvent)std::cout<<',';firstEvent=false;std::cout<<'['<<e.kind<<','<<e.id<<','<<e.result<<','<<e.flags<<','<<e.a<<','<<e.b<<']';}
  std::cout<<"]}";
 }
 if(!input.eof())throw std::runtime_error("Malformed input");
 std::cout<<"]\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
