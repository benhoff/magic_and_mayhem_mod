/* Execute actual facade call boundaries; no original executable or media. */
#include "../runtime/shadow/win32_min.h"
API void* WIN LoadLibraryA(const char*);
API void* WIN GetProcAddress(void*,const char*);
API i32 WIN SetEnvironmentVariableA(const char*,const char*);
API u32 WIN GetCurrentThreadId(void);
#include "../runtime/scene/glyph_native_only.h"
__declspec(allocate(".own")) static u8 address_space[0x140000] __attribute__((used));
static void require(int value,u32 error){if(!value)ExitProcess(error);}
static void** table(void* object){return *(void***)object;}
void start(void){
 char name[1024],mode[32];
 require(GetEnvironmentVariableA("MNM_TEST_LIBRARY",name,sizeof(name))>0,180);
 require(GetEnvironmentVariableA("MNM_TEST_CASE",mode,sizeof(mode))>0,181);
 if(mode[0]=='g'){glyph_native_only_configure();glyph_native_only_refuse(17);ExitProcess(0);}
 void* module=LoadLibraryA(name);require(module!=0,182);
 i32(WIN* create)(void*,void**,void*)=GetProcAddress(module,"NativeDrawCreate");require(create!=0,183);
 u8* site=(u8*)0x533aec;u32 page[7],old;
 require(VirtualQuery(site,page,28)==28 && page[1]==0x400000 && VirtualProtect(site,8,0x40,&old),184);
 const u8 prefix[20]={0x8b,0xcd,0x8b,0xf0,0x8b,0xfa,0x03,0xc3,0xc1,0xe9,2,0xf3,0xa5,0x8b,0xcd,0x83,0xe1,3,0xf3,0xa4};for(u32 i=0;i<20;++i)site[i]=prefix[i];
 void* draw=0;require(!create(0,&draw,0)&&draw,185);
 u32 desc[27]={108,7,9,13};desc[26]=0x840;void* surface=0;
 require(!((i32(WIN*)(void*,const u32*,void**,void*))table(draw)[6])(draw,desc,&surface,0),186);
 i32(WIN* fast)(void*,u32,u32,void*,const i32*,u32)=table(surface)[7];
 const i32 rect[4]={0,0,4,4};i32 result;
 if(mode[0]=='u')result=fast(surface,0,0,surface,rect,0x20);
 else if(mode[0]=='k'){
  const u32 key[2]={1,1};
  result=((i32(WIN*)(void*,u32,const u32*))table(surface)[29])(surface,0x20,key);
 }else if(mode[0]=='b'){
  u32 lock[27]={108};require(!((i32(WIN*)(void*,void*,u32*,u32,void*))table(surface)[25])(surface,0,lock,0,0),187);
  result=fast(surface,0,0,surface,rect,0x10);require(result==(i32)0x887601aeu,188);
  require(!((i32(WIN*)(void*,void*))table(surface)[32])(surface,(void*)lock[9]),189);
 }else{result=fast(surface,0xffffffffu,0,surface,rect,0);require(result==(i32)0x88760096u,190);}
 if(mode[0]=='u'||mode[0]=='k')require(result==(i32)0x80004001u,191);
 require(!((u32(WIN*)(void*))table(surface)[2])(surface),192);
 require(!((u32(WIN*)(void*))table(draw)[2])(draw),193);
 ExitProcess(0);
}
