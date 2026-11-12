/* Unchanged original query bytes against real production NativeDraw surfaces.
 * The fixture's padded PE reserves the original code/global addresses before
 * Wine heap allocation. No original executable file is patched or launched. */
#include "../runtime/shadow/win32_min.h"
#include "original_format_query.h"
API void* WIN LoadLibraryA(const char*);
API void* WIN GetProcAddress(void*,const char*);
static void require(int value,u32 code){if(!value)ExitProcess(code);}
static void** table(void* object){return *(void***)object;}
void start(void){
 u8* code=(u8*)0x58d540;u32 protection=0;
 require(VirtualProtect(code,sizeof(original_format_query),0x40,&protection),100);
 for(u32 i=0;i<sizeof(original_format_query);++i){require(code[i]==0,101);code[i]=original_format_query[i];}
 require(FlushInstructionCache(GetCurrentProcess(),code,sizeof(original_format_query)),102);
 require(VirtualProtect((void*)0x6e1f60,64,4,&protection),103);
 char path[1024];require(GetEnvironmentVariableA("MNM_QUERY_LIBRARY",path,sizeof(path))>0,104);
 void* library=LoadLibraryA(path);require(library!=0,105);
 i32(WIN* create)(void*,void**,void*)=GetProcAddress(library,"NativeDrawCreate");require(create!=0,106);
 u32 rows[4][12];
 for(u32 kind=0;kind<4;++kind){
  void *draw=0,*surface=0;require(!create(0,&draw,0)&&draw,107);
  u32 d[27]={108,7,9,13};d[26]=0x840;
  u32 red=kind==3?0x7c00:0xf800,green=kind==3?0x3e0:0x7e0;
  if(kind==0){
   require(!((i32(WIN*)(void*,void*,u32))table(draw)[20])(draw,0,8),108);
   d[1]=1;d[2]=0;d[3]=0;d[26]=0x200;
  }else if(kind>1){d[1]=0x1007;d[18]=32;d[19]=0x40;d[21]=16;d[22]=red;d[23]=green;d[24]=31;}
  require(!((i32(WIN*)(void*,const u32*,void**,void*))table(draw)[6])(draw,d,&surface,0)&&surface,109);
  u32 format[8]={32};require(!((i32(WIN*)(void*,u32*))table(surface)[21])(surface,format),110);
  require(format[3]==16&&format[4]==red&&format[5]==green&&format[6]==31,111);
  for(u32 i=0;i<64;++i)((volatile u8*)0x6e1f60)[i]=0xa5;
  u32 wrapper[4]={0,0,(u32)surface,0};
  u32 (__attribute__((thiscall))*query)(void*)=(void*)code;
  u32 result=query(wrapper);require(result==(kind==3),112);
  require(*(volatile u16*)0x6e1f7c==red&&*(volatile u16*)0x6e1f7e==green&&*(volatile u16*)0x6e1f80==31,113);
  for(u32 i=0;i<64;++i)if(i<28||i>=34)require(((volatile u8*)0x6e1f60)[i]==0xa5,114);
  for(u32 i=0;i<sizeof(original_format_query);++i)require(code[i]==original_format_query[i],115);
  require(!((u32(WIN*)(void*))table(surface)[2])(surface),116);
  require(!((u32(WIN*)(void*))table(draw)[2])(draw),117);
  u32 surfaces=((u32(WIN*)(void))GetProcAddress(library,"NativeDrawSurfaces"))();
  u32 devices=((u32(WIN*)(void))GetProcAddress(library,"NativeDrawObjects"))();require(!surfaces&&!devices,118);
  rows[kind][0]=kind;rows[kind][1]=result;rows[kind][2]=red;rows[kind][3]=green;rows[kind][4]=31;
  rows[kind][5]=format[3];rows[kind][6]=1;rows[kind][7]=1;rows[kind][8]=surfaces;rows[kind][9]=devices;rows[kind][10]=0;rows[kind][11]=0;
 }
 HANDLE file=CreateFileA("query-results.bin",0x40000000,0,0,1,0x80,0);u32 done=0;
 require(file!=(HANDLE)-1&&WriteFile(file,rows,sizeof(rows),&done,0)&&done==sizeof(rows),119);CloseHandle(file);ExitProcess(0);
}
