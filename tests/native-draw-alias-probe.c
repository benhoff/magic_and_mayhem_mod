/* Independent unchanged callers: retain a surface through queried aliases,
 * release its creator, create another surface, and check every retained word.
 * Addresses are never treated as allocation generations across lifetimes. */
#include "../runtime/shadow/win32_min.h"
API void* WIN LoadLibraryA(const char*);
API void* WIN GetProcAddress(void*,const char*);
typedef i32 (WIN *Create)(void*,void**,void*);
typedef i32 (WIN *Query)(void*,const u8*,void**);
typedef u32 (WIN *Ref)(void*);
static void** table(void* p){return *(void***)p;}
static const u8 ids[3][16]={
 {0x85,0x58,0x80,0x57,0xec,0x6e,0xcf,0x11,0x94,0x41,0xa8,0x23,0x03,0xc1,0x0e,0x27},
 {0x00,0x4e,0x04,0xda,0xb2,0x69,0xd0,0x11,0xa1,0xd5,0,0xaa,0,0xb8,0xdf,0xbb},
 {0x30,0x86,0x2b,0x0b,0x35,0xad,0xd0,0x11,0x8e,0xa6,0,0x60,0x97,0x97,0xea,0x5b}};
static const u8 unknown[16]={0,0,0,0,0,0,0,0,0xc0,0,0,0,0,0,0,0x46};
static u32 results[9][16+5*117];
static u32 current_case;
static void progress(u32 step){
 u32 words[2]={current_case,step},written=0;
 HANDLE file=CreateFileA("alias-progress.bin",0x40000000,0,0,2,0x80,0);
 if(file==(HANDLE)-1||!WriteFile(file,words,sizeof(words),&written,0)||written!=sizeof(words))ExitProcess(216);
 CloseHandle(file);
}
static void desc(u32* d){
 for(u32 i=0;i<31;++i)d[i]=0;
 d[0]=108;d[1]=0x1007;d[2]=9;d[3]=13;d[18]=32;d[19]=0x40;d[21]=16;
 d[22]=0xf800;d[23]=0x7e0;d[24]=31;d[26]=0x840;
}
static void sample(void* object,u32 version,u32* out,u32 seed,u32 write){
 u32 d[31];desc(d);d[0]=version==2?124:108;
 if(((i32 (WIN *)(void*,const i32*,u32*,u32,HANDLE))table(object)[25])(object,0,d,1,0))ExitProcess(201);
 if(d[2]!=9||d[3]!=13||d[4]<26||d[21]!=16)ExitProcess(202);
 for(u32 y=0;y<9;++y)for(u32 x=0;x<13;++x){u16* p=(u16*)(d[9]+y*d[4])+x;
  if(write)*p=(u16)(seed+x*337+y*971);
  if(out)out[y*13+x]=*p;
 }
 if(((i32 (WIN *)(void*,void*))table(object)[32])(object,version==2?0:(void*)d[9]))ExitProcess(203);
}
void start(void){
 char mode[2];u32 native=GetEnvironmentVariableA("MNM_FACTORY_NATIVE",mode,2)==1&&mode[0]=='1';
 void* module=LoadLibraryA(native?"MnmDraw.dll":"ddraw.dll");if(!module)ExitProcess(204);
 if(native&&GetModuleHandleA("ddraw.dll"))ExitProcess(205);
 Create create=(Create)GetProcAddress(module,"DirectDrawCreate");if(!create)ExitProcess(206);
 for(u32 a=0;a<3;++a)for(u32 b=0;b<3;++b){
  u32* r=results[a*3+b];u32 d[31];void *draw=0,*surface=0,*left=0,*right=0,*one=0,*two=0,*fresh=0;
  current_case=a*3+b;progress(1);r[0]=a;r[1]=b;
  if(create(0,&draw,0)||((i32 (WIN *)(void*,void*,u32))table(draw)[20])(draw,0,8))ExitProcess(207);
  desc(d);if(((i32 (WIN *)(void*,const u32*,void**,void*))table(draw)[6])(draw,d,&surface,0))ExitProcess(208);
  if(((Query)table(surface)[0])(surface,ids[a],&left)||((Query)table(left)[0])(left,ids[b],&right))ExitProcess(209);
  if(((Query)table(left)[0])(left,unknown,&one)||((Query)table(right)[0])(right,unknown,&two))ExitProcess(210);
  if(one!=two)ExitProcess(211);r[2]=1;progress(2);
  sample(left,a,0,0x1234,1);sample(surface,0,r+16,0,0);sample(right,b,r+16+117,0,0);
  progress(3);((Ref)table(surface)[2])(surface);surface=0;
  desc(d);if(((i32 (WIN *)(void*,const u32*,void**,void*))table(draw)[6])(draw,d,&fresh,0))ExitProcess(212);
  progress(4);sample(fresh,0,0,0x4567,1);sample(left,a,r+16+234,0,0);sample(fresh,0,r+16+351,0,0);
  sample(right,b,0,0x6789,1);sample(left,a,r+16+468,0,0);
  for(u32 y=0;y<9;++y)for(u32 x=0;x<13;++x){u32 at=y*13+x;
   for(u32 n=0;n<5;++n){u32 seed=n==3?0x4567:n==4?0x6789:0x1234;
    if(r[16+n*117+at]!=(u16)(seed+x*337+y*971))ExitProcess(213);
   }
  }
  progress(5);((Ref)table(fresh)[2])(fresh);
  /* Keep the caller's DirectDraw device alive while using retained aliases.
   * The separate diagnostic releases it early and faults in Wine's Lock. */
  progress(6);sample(right,b,0,0,0);progress(7);
  ((Ref)table(right)[2])(right);((Ref)table(left)[2])(left);
  ((Ref)table(one)[2])(one);((Ref)table(two)[2])(two);progress(8);
  ((Ref)table(draw)[2])(draw);
  if(native&&(((u32 (WIN *)(void))GetProcAddress(module,"NativeDrawObjects"))()||
              ((u32 (WIN *)(void))GetProcAddress(module,"NativeDrawSurfaces"))()))ExitProcess(214);
 }
 HANDLE file=CreateFileA("alias-results.bin",0x40000000,0,0,1,0x80,0);u32 written=0;
 if(file==(HANDLE)-1||!WriteFile(file,results,sizeof(results),&written,0)||written!=sizeof(results))ExitProcess(215);
 CloseHandle(file);ExitProcess(0);
}
