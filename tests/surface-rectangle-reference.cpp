// Isolated unchanged original wrappers; fake COM endpoints record arguments only.
// This fixture does not execute a DirectDraw driver or assert pixel/error behavior.
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
struct Rect {I l,t,r,b;};struct Point {I x,y;};
struct Object {std::array<U,16> words{};};
struct Interface {void** table;};
static Interface sourceInterface{},destinationInterface{},windowInterface{};
static unsigned calls=0,windowCalls=0;static U op=0,flags=0;static Rect src{},dst{};
static bool correctSource=false,correctDestination=false;static unsigned windowMode=0;
static I __attribute__((stdcall)) blt(void* receiver,Rect* d,void* s,Rect* r,U f,void* effects){
 ++calls;op=1;flags=f;src=*r;dst=*d;correctSource=s==&sourceInterface;
 correctDestination=receiver==&destinationInterface && effects==nullptr;return 0;
}
static I __attribute__((stdcall)) fast(void* receiver,I x,I y,void* s,Rect* r,U f){
 ++calls;op=2;flags=f;src=*r;dst={x,y,0,0};correctSource=s==&sourceInterface;
 correctDestination=receiver==&destinationInterface;return 0;
}
static I __attribute__((stdcall)) getWindow(void*,U* hwnd){*hwnd=0x1234;return 0;}
static I __attribute__((stdcall)) clientToScreen(U hwnd,Point* p){
 ++windowCalls;if(hwnd==0x1234){p->x=-19;p->y=33;return 1;}return 0;
}
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
 for(U entry:{0x58c360u,0x58c4a0u,0x58c8a0u,0x58ca90u,0x58cbc0u}){
  if(*reinterpret_cast<unsigned char*>(entry)!=0x83 ||
     *reinterpret_cast<unsigned char*>(entry+1)!=0xec)throw std::runtime_error("Wrapper signature differs");
 }
 // Bind a known Win32 import in private fixture memory; no original files change.
 *reinterpret_cast<void**>(0x5c51fc)=reinterpret_cast<void*>(&clientToScreen);
}
static void printRect(Rect r){std::cout<<'['<<r.l<<','<<r.t<<','<<r.r<<','<<r.b<<']';}
int main(int argc,char** argv)try{
 if(argc!=3)throw std::runtime_error("Expected pinned PE and fixture inputs");
 mapImage(argv[1]);
 std::array<void*,33> surfaceTable{};surfaceTable[5]=reinterpret_cast<void*>(&blt);surfaceTable[7]=reinterpret_cast<void*>(&fast);
 std::array<void*,5> windowTable{};windowTable[4]=reinterpret_cast<void*>(&getWindow);
 sourceInterface.table=destinationInterface.table=surfaceTable.data();windowInterface.table=windowTable.data();
 std::ifstream input(argv[2]);if(!input)throw std::runtime_error("Cannot open cases");
 U id,entry,fullscreen,noWait,missing;I x,y;Rect rect,dest;
 using Copy=void(__attribute__((thiscall))*)(Object*,Object*,Rect*,I,I);
 using Explicit=void(__attribute__((thiscall))*)(Object*,Object*,Rect*,Rect*);
 std::cout<<"[";bool first=true;
 while(input>>id>>entry>>fullscreen>>noWait>>windowMode>>missing>>rect.l>>rect.t>>rect.r>>rect.b>>x>>y>>dest.l>>dest.t>>dest.r>>dest.b){
  Object source{},target{};source.words[2]=reinterpret_cast<U>(&sourceInterface);target.words[2]=reinterpret_cast<U>(&destinationInterface);
  source.words[4]=target.words[4]=8;source.words[5]=target.words[5]=6;
  if(missing&1)source.words[2]=0;
  if(missing&2)target.words[2]=0;
  *reinterpret_cast<U*>(0x6f68d8)=fullscreen;*reinterpret_cast<U*>(0x6f98d8)=noWait;
  *reinterpret_cast<Object**>(0x6f98c0)=windowMode?&target:nullptr;
  *reinterpret_cast<Interface**>(0x6f98d0)=windowMode==1?&windowInterface:nullptr;
  calls=windowCalls=op=flags=0;correctSource=correctDestination=false;src=dst={0,0,0,0};
  if(entry==0x58cbc0)reinterpret_cast<Explicit>(entry)(&source,&target,&rect,&dest);
  else reinterpret_cast<Copy>(entry)(&source,&target,&rect,x,y);
  if(!first)std::cout<<',';
  first=false;
  std::cout<<"{\"id\":"<<id<<",\"calls\":"<<calls<<",\"operation\":"<<op<<",\"flags\":"<<flags<<",\"source\":";printRect(src);
  std::cout<<",\"destination\":";printRect(dst);std::cout<<",\"source_after\":";printRect(rect);
  std::cout<<",\"destination_after\":";printRect(dest);
  std::cout<<",\"window_calls\":"<<windowCalls<<",\"source_identity\":"<<(correctSource?"true":"false")
           <<",\"destination_identity\":"<<(correctDestination?"true":"false")<<'}';
 }
 if(!input.eof())throw std::runtime_error("Invalid case input");
 std::cout<<"]\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
