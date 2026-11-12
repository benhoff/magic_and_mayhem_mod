/* Supplied facade descriptor/lease control; no original game execution. */
#include "../runtime/shadow/win32_min.h"
API void* WIN LoadLibraryA(const char*);
API void* WIN GetProcAddress(void*,const char*);
static void require(int ok,u32 code){if(!ok)ExitProcess(code);}
static void** table(void* p){return *(void***)p;}
void start(void){
 char path[1024];require(GetEnvironmentVariableA("MNM_SCOPE_LIBRARY",path,sizeof(path))>0,100);
 void* module=LoadLibraryA(path);require(module!=0,101);
 i32(WIN* create)(void*,void**,void*)=GetProcAddress(module,"NativeDrawCreate");require(create!=0,102);
 u32 results[3][12];
 for(u32 kind=0;kind<3;++kind){
  void *draw=0,*surface=0;require(!create(0,&draw,0)&&draw,103);
  u32 d[27]={108,7,9,13};d[26]=0x840;
  const u32 red=kind==2?0x7c00:0xf800,green=kind==2?0x3e0:0x7e0;
  if(kind){d[1]=0x1007;d[18]=32;d[19]=0x40;d[21]=16;d[22]=red;d[23]=green;d[24]=31;}
  require(!((i32(WIN*)(void*,const u32*,void**,void*))table(draw)[6])(draw,d,&surface,0)&&surface,104);
  u32 format[8]={32};require(!((i32(WIN*)(void*,u32*))table(surface)[21])(surface,format),105);
  require(format[3]==16&&format[4]==red&&format[5]==green&&format[6]==31,106);
  u32 lock[27]={108};require(!((i32(WIN*)(void*,const i32*,u32*,u32,void*))table(surface)[25])(surface,0,lock,1,0),107);
  require(lock[2]==9&&lock[3]==13&&lock[21]==16&&lock[22]==red&&lock[23]==green&&lock[24]==31,108);
  u16* word=(u16*)lock[9];*word=0x9234;require(*word==0x9234,109);
  require(!((i32(WIN*)(void*,void*))table(surface)[32])(surface,(void*)lock[9]),110);
  lock[0]=108;require(!((i32(WIN*)(void*,const i32*,u32*,u32,void*))table(surface)[25])(surface,0,lock,1,0),115);
  require(*(volatile u16*)lock[9]==0x9234,116);
  require(!((i32(WIN*)(void*,void*))table(surface)[32])(surface,(void*)lock[9]),117);
  require(!((u32(WIN*)(void*))table(surface)[2])(surface),111);
  require(!((u32(WIN*)(void*))table(draw)[2])(draw),112);
  results[kind][0]=kind;results[kind][1]=format[3];results[kind][2]=format[4];results[kind][3]=format[5];results[kind][4]=format[6];
  results[kind][5]=lock[3];results[kind][6]=lock[2];results[kind][7]=lock[4];results[kind][8]=0x9234;
  results[kind][9]=((u32(WIN*)(void))GetProcAddress(module,"NativeDrawSurfaces"))();
  results[kind][10]=((u32(WIN*)(void))GetProcAddress(module,"NativeDrawObjects"))();results[kind][11]=0;
  require(!results[kind][9]&&!results[kind][10],113);
 }
 u32 done=0;HANDLE file=CreateFileA("formats.bin",0x40000000,0,0,1,0x80,0);
 require(file!=(HANDLE)-1&&WriteFile(file,results,sizeof(results),&done,0)&&done==sizeof(results),114);
 CloseHandle(file);ExitProcess(0);
}
