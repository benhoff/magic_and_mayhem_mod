#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
#include "region_entry_fixture_bytes.h"
#include <iostream>
static unsigned transitions,admissions,counts,kinds,wizards,worlds,pops;
static void check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
static unsigned word(unsigned at){unsigned v;std::memcpy(&v,(void*)at,4);return v;}
static void put(unsigned at,unsigned v){std::memcpy((void*)at,&v,4);}
static void __attribute__((thiscall)) transition(void* self){check(self==(void*)0x6578c0,"transition receiver");++transitions;*(unsigned char*)0x6578cc=1;put(0x6578cd,0);}
static void __attribute__((thiscall)) admission(unsigned region){check(region==7,"admission ECX region");++admissions;}
static int __attribute__((thiscall)) count(void* self,int category){check(self==(void*)0x65b250&&category==2,"wizard count args");++counts;return 7;}
static int __attribute__((thiscall)) kind(const char* realm){check(std::string(realm)=="Greek","realm kind arg");++kinds;return 2;}
static int wizard(){++wizards;return 2;}
static void world(){++worlds;}
static void pop(){check(!word(0x657d37),"Realm pop clears active marker");++pops;}
static void stub(unsigned at,const unsigned char* expected,unsigned size,void* target){check(size>=5&&!std::memcmp((void*)at,expected,size),"stub bytes");auto* b=(unsigned char*)at;b[0]=0xe9;put(at+1,unsigned((uintptr_t)target-at-5));}
int main(int argc,char** argv)try{
 check(argc==2,"PE input");map_image(read(argv[1]));
#define STUB(name) stub(name##_address,name##_bytes,sizeof(name##_bytes),(void*)&name)
 STUB(transition);STUB(admission);STUB(count);STUB(kind);STUB(wizard);STUB(world);STUB(pop);
 using Action=int (__attribute__((thiscall)) *)(void*,unsigned);auto action=(Action)0x4b3420;unsigned cases=0;
 for(unsigned caller:{4u,22u,3u})for(unsigned difficulty=0;difficulty<4;++difficulty)for(unsigned moved:{0u,1u})for(unsigned index:{0u,1u,2u,3u,4u,5u,0xffffffffu}){
  std::memset((void*)0x6578c0,0,0x100);put(0x6578c4,18);put(0x65791f,0x6e3000);put(0x657923,difficulty);put(0x657927,4);put(0x65792f,7);put(0x657933,0x6e5000);std::strcpy((char*)0x6e5000,"Greek");*(unsigned char*)0x657937=moved;put(0x65793c,caller);
  for(unsigned i=0;i<4;++i){put(0x6e3000+i*4,0x6e4000+i*0x80);put(0x6e402d+i*0x80,i);}
  for(unsigned at:{0x689924u,0x656630u,0x65662cu,0x656634u,0x6e1abau,0x6e1abeu,0x6e1ab6u,0x6f2cf0u,0x6f2b30u,0x659e3fu})put(at,0x11223344);
  put(0x656620,2);*(unsigned char*)0x659e50=1;transitions=admissions=counts=kinds=wizards=0;
  check(action((void*)0x6578c0,index)==0,"callback return");unsigned picked=caller==22?0:difficulty;
  unsigned expectedNext=index==2?0x6e0088:index==3?0x6f2aa0:index==4?0x6c5148:0;
  check(word(0x6578f3)==expectedNext,"requested screen");check(word(0x657903)==((index==1||(index==0&&caller==4))?1u:0u),"return request");
  check(transitions==((index==1||index==2||index==3||index==4||(index==0&&caller==4))?1u:0u),"transition count");
  check(word(0x689924)==(index==0||index==2||index==3?picked:index==4?difficulty:0x11223344u),"difficulty writes");
  check(admissions==((index==0&&caller==4)?1u:0u)&&counts==((index==0&&caller!=4)?1u:0u),"Enter caller routing");
  check(*(unsigned char*)0x657940==((index==0&&caller!=4)?1u:0u),"deferred launch flag");
  if(index==0&&caller!=4)check(word(0x656630)==1&&word(0x65662c)==8&&!word(0x656634),"world participant counters");
  check(word(0x659e3f)==(index==1&&!moved?1u:0x11223344u)&&*(unsigned char*)0x659e50==(index==1&&!moved?0u:1u),"Cancel current-region exit");
  check(kinds==(index==2?1u:0u)&&wizards==(index==3?1u:0u),"auxiliary dependencies");
  if(index==2)check(word(0x6e1aba)==2&&word(0x6e1abe)==7&&word(0x6e1ab6)==1,"action-2 request fields");
  if(index==3)check(!word(0x6f2cf0)&&word(0x6f2b30)==0x65b250+2*0x93a,"action-3 wizard binding");
  ++cases;
 }
 using Deferred=void (__attribute__((thiscall)) *)(void*);unsigned deferredCases=0;
 for(unsigned flag:{0u,1u,2u,255u}){
  std::memset((void*)0x6578c0,0,0x100);*(unsigned char*)0x657940=flag;worlds=transitions=0;((Deferred)0x4b3880)((void*)0x6578c0);
  check(!*(unsigned char*)0x657940&&worlds==(flag?1u:0u)&&transitions==(flag?1u:0u)&&word(0x6578f3)==(flag?0x6cbb78u:0u)&&word(0x657903)==(flag?1u:0u),"deferred original world request");++deferredCases;
 }
 unsigned returnCases=0;
 for(unsigned flag:{1u,2u,0xffffffffu}){
  put(0x659e3f,flag);put(0x657d37,0x123);pops=0;((Deferred)0x552210)((void*)0x659408);
  check(!word(0x659e3f)&&!word(0x657d37)&&pops==1,"Realm resume consumes pending exit");++returnCases;
 }
 std::cout<<"{\"success\":true,\"callback_cases\":"<<cases<<",\"deferred_cases\":"<<deferredCases<<",\"return_cases\":"<<returnCases<<"}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
