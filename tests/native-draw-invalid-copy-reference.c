/* Real provider comparison for the two historical invalid-copy shapes plus
 * three boundary controls. No reconstructed coordinates or game execution. */
#define start surface_corpus_start
#include "native-draw-surface-probe.c"
#undef start
static const u32 shapes[][10]={
 {640,480,40,40,640,512,680,552,0,0},
 {400,280,640,480,0,0,40,40,640,512},
 {400,280,640,480,0,0,40,40,0xffffffffu,0},
 {400,280,640,480,0,0,40,40,0,0x80000000u},
 {640,480,40,40,640,512,680,552,7,9}
};
static void output(HANDLE file,const void* bytes,u32 size){
 u32 written=0;if(!WriteFile(file,bytes,size,&written,0)||written!=size)ExitProcess(181);
}
static void sample(HANDLE file,void* surface,u32 width,u32 height,u32 initialize){
 u32 d[31]={108};if(((Lock)table(surface)[25])(surface,0,d,1,0))ExitProcess(182);
 if(d[2]!=height||d[3]!=width)ExitProcess(183);
 for(u32 y=0;y<height;++y){u16* row=(u16*)(d[9]+y*d[4]);
  if(initialize)for(u32 x=0;x<width;++x)row[x]=(u16)(0x1500+x*3+y*17);
  output(file,row,width*2);
 }
 if(((Unlock)table(surface)[32])(surface,(void*)d[9]))ExitProcess(184);
}
void start(void){
 char value[2];u32 native=GetEnvironmentVariableA("MNM_FACTORY_NATIVE",value,2)!=0;
 void* module=LoadLibraryA(native?"MnmDraw.dll":"ddraw.dll");if(!module)ExitProcess(185);
 Create create=(Create)GetProcAddress(module,native?"NativeDrawCreate":"DirectDrawCreate");if(!create)ExitProcess(186);
 HANDLE file=CreateFileA("invalid-results.bin",0x40000000,0,0,1,0,0);if(file==(HANDLE)-1)ExitProcess(187);
 for(u32 index=0;index<5;++index){
  const u32* s=shapes[index];void *draw=0,*source=0,*target=0;u32 d[31];
  if(create(0,&draw,0)||((i32(WIN*)(void*,void*,u32))table(draw)[20])(draw,GetDesktopWindow(),8))ExitProcess(188);
  describe(d,0);d[3]=s[0];d[2]=s[1];
  if(((i32(WIN*)(void*,const u32*,void**,void*))table(draw)[6])(draw,d,&source,0))ExitProcess(189);
  describe(d,0);d[3]=s[2];d[2]=s[3];
  if(((i32(WIN*)(void*,const u32*,void**,void*))table(draw)[6])(draw,d,&target,0))ExitProcess(190);
  u32 header[16]={index,s[0],s[1],s[2],s[3],s[4],s[5],s[6],s[7],s[8],s[9],index==1?17:16};
  output(file,header,sizeof(header));sample(file,source,s[0],s[1],1);sample(file,target,s[2],s[3],1);
  i32 rectangle[4]={(i32)s[4],(i32)s[5],(i32)s[6],(i32)s[7]};
  u32 result=(u32)((Fast)table(target)[7])(target,s[8],s[9],source,rectangle,header[11]);output(file,&result,4);
  sample(file,source,s[0],s[1],0);sample(file,target,s[2],s[3],0);
  ((Ref)table(target)[2])(target);((Ref)table(source)[2])(source);((Ref)table(draw)[2])(draw);
 }
 CloseHandle(file);if(native&&GetModuleHandleA("ddraw.dll"))ExitProcess(191);ExitProcess(0);
}
