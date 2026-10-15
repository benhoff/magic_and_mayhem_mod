/* Identical caller executed separately against Wine and the owned facade. */
#include "../runtime/shadow/win32_min.h"
API void* WIN LoadLibraryA(const char*);
API void* WIN GetProcAddress(void*,const char*);
API void* WIN CreateWindowExA(u32,const char*,const char*,u32,i32,i32,i32,i32,void*,void*,void*,void*);
API i32 WIN SetWindowPos(void*,void*,i32,i32,i32,i32,u32);
API i32 WIN ShowWindow(void*,i32);
API i32 WIN DestroyWindow(void*);
API i32 WIN PeekMessageA(void*,void*,u32,u32,u32);
API i32 WIN TranslateMessage(const void*);
API i32 WIN DispatchMessageA(const void*);
API void WIN Sleep(u32);
typedef u32 (WIN *Ref)(void*);
static u32 output[6][1036];
static void require(int value,u32 stage){if(!value)ExitProcess(stage);}
static void** table(void* p){return *(void***)p;}
static void pump(void){u32 message[12];for(u32 i=0;i<15;++i){while(PeekMessageA(message,0,0,0,1)){TranslateMessage(message);DispatchMessageA(message);}Sleep(10);}}
void start(void){
 char value[2];u32 native=GetEnvironmentVariableA("MNM_FACTORY_NATIVE",value,2)!=0;
 void* module=LoadLibraryA(native?"MnmDraw.dll":"ddraw.dll");require(module!=0,101);
 typedef i32 (WIN *Create)(void*,void**,void*);Create create=(Create)GetProcAddress(module,"DirectDrawCreate");
 void *draw=0,*clip=0,*window=0,*cover=0;require(create && !create(0,&draw,0),102);
 require(!((i32 (WIN *)(void*,void*,u32))table(draw)[20])(draw,0,8),103);
 require(!((i32 (WIN *)(void*,u32,void**,void*))table(draw)[4])(draw,0,&clip,0),104);
 window=CreateWindowExA(0,"STATIC","clip source",0x90000000,30,40,80,60,0,0,0,0);require(window!=0,105);
 require(!((i32 (WIN *)(void*,u32,void*))table(clip)[8])(clip,0,window),106);
 for(u32 phase=0;phase<6;++phase){
  if(phase==1)require(SetWindowPos(window,0,70,80,100,70,0),107);
  if(phase==2){cover=CreateWindowExA(0,"STATIC","clip cover",0x90000000,80,90,40,30,0,0,0,0);require(cover!=0,108);}
  if(phase==3)ShowWindow(window,0);
  if(phase==4){require(DestroyWindow(cover),109);cover=0;ShowWindow(window,5);}
  if(phase==5)require(!((i32 (WIN *)(void*,u32,void*))table(clip)[8])(clip,0,0),110);
  pump();u32 bytes=0;
  output[phase][0]=(u32)((i32 (WIN *)(void*,const void*,void*,u32*))table(clip)[3])(clip,0,0,&bytes);
  output[phase][1]=bytes;require(bytes<=32+256*16,111);
  if(!output[phase][0]){u32 capacity=32+256*16;
   output[phase][2]=(u32)((i32 (WIN *)(void*,const void*,void*,u32*))table(clip)[3])(clip,0,output[phase]+4,&capacity);
   output[phase][3]=capacity;}
 }
 require(DestroyWindow(window),112);((Ref)table(clip)[2])(clip);((Ref)table(draw)[2])(draw);
 u32 done=0;HANDLE f=CreateFileA("window-clips.bin",0x40000000,0,0,1,0x80,0);
 require(f!=(HANDLE)-1 && WriteFile(f,output,sizeof(output),&done,0) && done==sizeof(output),113);CloseHandle(f);ExitProcess(0);
}
