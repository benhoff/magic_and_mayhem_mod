/* Actual facade authority, propagation, refusal and resource retirement. */
#define start retained_resource_probe
#include "native-draw-resource-probe.c"
#undef start
typedef i32 (WIN *History)(const void*,u32,u32,u32,u32,u32);
void start(void){
 void* module=LoadLibraryA("MnmDraw.dll");require(module!=0,101);
 Create create=(Create)GetProcAddress(module,"NativeDrawCreate");
 History history=(History)GetProcAddress(module,"NativeDrawCpuHistoryV1");
 void *draw=0,*s=0;u32 d[31];require(history && create && !create(0,&draw,0),102);
 desc(d,2);d[1]=7;d[2]=9;d[3]=13;d[26]=0x840;
 require(!((Surface)table(draw)[6])(draw,d,&s,0),103);
 desc(d,2);require(!((Lock)table(s)[25])(s,0,d,1,0),104);void* pixels=(void*)d[9];
 require(!history(pixels,0,0,0,0,0) && !history(pixels,2,0,0,0,0),105);
 require(history(pixels,1,0,0,5,6)==(i32)0x80070057u,106);
 require(!history(pixels,1,1,0,5,6),107);
 require(history(pixels,2,0,0,0,0)==(i32)0x887601aeu,108);
 require(history(pixels,1,1,0,5,6)==(i32)0x80070057u && history(pixels,1,2,0,7,8)==(i32)0x80070057u,109);
 require(!history(pixels,1,2,0,5,6),110);
 /* Independent CPU writes and Unlock cannot certify missing World history. */
 *(u16*)pixels=0x1234;require(!((i32 (WIN *)(void*,void*))table(s)[32])(s,pixels),111);
 char mode[4];u32 n=GetEnvironmentVariableA("MNM_HISTORY_REFUSE",mode,4);
 if(n){
  if(mode[0]=='1')history(pixels,0,0,0,0,0);
  else if(mode[0]=='2'){void* dc=0;((i32 (WIN *)(void*,void**))table(s)[17])(s,&dc);}
  else {((i32 (WIN *)(void*,void*,void*,void*,u32,u32*))table(s)[5])(s,0,s,0,0,0);}
  ExitProcess(112);
 }
 require(history((u8*)pixels+2,2,0,0,0,0)==(i32)0x887601aeu,113);
 u32 fx[25]={100};fx[20]=0x5678;i32 rect[4]={0,0,1,1};
 require(!((i32 (WIN *)(void*,i32*,void*,void*,u32,u32*))table(s)[5])(s,rect,0,0,0x400,fx),114);
 require(history(pixels,2,0,0,0,0)==(i32)0x887601aeu,115);
 require(!((i32 (WIN *)(void*,void*,void*,void*,u32,u32*))table(s)[5])(s,0,0,0,0x400,fx),116);
 require(!history(pixels,0,0,0,0,0) && !history(pixels,2,0,0,0,0),117);
 require(!release(s),118);
 require(history(pixels,2,0,0,0,0)==(i32)0x80070057u,119);
 void *front=0,*back=0;u32 caps=4;
 require(!((i32 (WIN *)(void*,void*,u32))table(draw)[20])(draw,GetDesktopWindow(),0x11),121);
 require(!((i32 (WIN *)(void*,u32,u32,u32))table(draw)[21])(draw,13,9,16),122);
 desc(d,2);d[1]=0x21;d[5]=1;d[26]=0x218;
 require(!((Surface)table(draw)[6])(draw,d,&front,0),123);
 require(!((i32 (WIN *)(void*,u32*,void**))table(front)[12])(front,&caps,&back),124);
 desc(d,2);require(!((Lock)table(front)[25])(front,0,d,1,0),125);pixels=(void*)d[9];
 require(!history(pixels,1,1,0,7,8),126);
 require(!((i32 (WIN *)(void*,void*,u32))table(front)[11])(front,0,0),127);
 require(((i32 (WIN *)(void*,void*))table(front)[32])(front,0)==(i32)0x88760248u,128);
 require(!((i32 (WIN *)(void*,void*))table(back)[32])(back,0),129);
 require(history(pixels,2,0,0,0,0)==(i32)0x887601aeu,130);
 /* Full fill of the opposite physical backing must not clear this authority. */
 require(!((i32 (WIN *)(void*,void*,void*,void*,u32,u32*))table(front)[5])(front,0,0,0,0x400,fx),131);
 require(history(pixels,2,0,0,0,0)==(i32)0x887601aeu,132);
 require(!((i32 (WIN *)(void*,void*,void*,void*,u32,u32*))table(back)[5])(back,0,0,0,0x400,fx),133);
 require(!history(pixels,2,0,0,0,0),134);
 require(release(back)==1 && !release(front) && !release(draw),135);
 u32 done=0;HANDLE file=CreateFileA("history-success.bin",0x40000000,0,0,1,0x80,0);
 require(file!=(HANDLE)-1 && WriteFile(file,&checks,4,&done,0) && done==4,120);CloseHandle(file);ExitProcess(0);
}
