/* PE32, no CRT. Real IDirectDraw / IDirectDrawSurface2 calls; no native renderer or fake endpoint. */
#include "../runtime/shadow/win32_min.h"
API int WIN ReadFile(HANDLE,void*,u32,u32*,void*);
API HANDLE WIN LoadLibraryA(const char*);
API void* WIN GetProcAddress(HANDLE,const char*);
typedef i32 (WIN *Create)(void*,void**,void*);
struct Rect {i32 l,t,r,b;};
struct Input {u32 id,fast,clip,flags,held;struct Rect src,dst;};
struct Output {struct Input input,after; i32 result;u32 source_desc[27],dest_desc[27];u32 clip_size;i32 clip_result;u32 clip_data[16];u16 source_before[48],dest_before[48],source_after[48],dest_after[48];};
static struct Output out;
static void clear(void* p,u32 n){u8* b=p;while(n--)*b++=0;}
static void require(int ok,u32 stage){if(!ok)ExitProcess(stage);}
static void** table(void* p){return *(void***)p;}
static void release(void* p){if(p)((u32(WIN*)(void*))table(p)[2])(p);}
static void lock(void* p,u32* d){clear(d,108);d[0]=108;require(((i32(WIN*)(void*,void*,void*,u32,HANDLE))table(p)[25])(p,0,d,1,0)==0,20);require(d[2]==6 && d[3]==8 && d[21]==16 && d[22]==0xf800 && d[23]==0x7e0 && d[24]==31,21);}
static void unlock(void* p){require(((i32(WIN*)(void*,void*))table(p)[32])(p,0)==0,22);}
static void pixels(void* p,u16* data,int write,int seed,u32* desc){u32 d[27];lock(p,d);for(u32 y=0;y<6;++y)for(u32 x=0;x<8;++x){u16* q=(u16*)((u8*)d[9]+y*d[4])+x;u32 i=y*8+x;if(write)*q=(u16)(seed?0xa000+i:0x1000+i*73);data[i]=*q;}for(u32 i=0;i<27;++i)desc[i]=i==9?0:d[i];unlock(p);}
static void* surface(void* dd){u32 d[27];void* p=0;clear(d,108);d[0]=108;d[1]=0x1007;d[2]=6;d[3]=8;d[18]=32;d[19]=0x40;d[21]=16;d[22]=0xf800;d[23]=0x7e0;d[24]=31;d[26]=0x840;require(((i32(WIN*)(void*,void*,void**,void*))table(dd)[6])(dd,d,&p,0)==0,12);
 static const u8 iid[16]={0x85,0x58,0x80,0x57,0xec,0x6e,0xcf,0x11,0x94,0x41,0xa8,0x23,3,0xc1,0x0e,0x27};void* second=0;require(((i32(WIN*)(void*,const void*,void**))table(p)[0])(p,iid,&second)==0 && second,18);release(p);return second;}
void start(void){void* dd=0;HANDLE dll=LoadLibraryA("ddraw.dll");require(dll!=0,10);Create create=(Create)GetProcAddress(dll,"DirectDrawCreate");require(create!=0 && create(0,&dd,0)==0,11);require(((i32(WIN*)(void*,void*,u32))table(dd)[20])(dd,0,8)==0,13);
 HANDLE input=CreateFileA("inputs.bin",0x80000000,1,0,3,0,0),output=CreateFileA("outputs.bin",0x40000000,0,0,1,0,0);require(input!=(HANDLE)-1 && output!=(HANDLE)-1,14);u32 n;
 for(;;){clear(&out,sizeof(out));require(ReadFile(input,&out.input,sizeof(out.input),&n,0),15);if(!n)break;require(n==sizeof(out.input),16);out.after=out.input;void* s=surface(dd);void* d=surface(dd);void* c=0;
  pixels(s,out.source_before,1,0,out.source_desc);pixels(d,out.dest_before,1,1,out.dest_desc);
  if(out.input.clip){require(((i32(WIN*)(void*,u32,void**,void*))table(dd)[4])(dd,0,&c,0)==0,30);
   if(out.input.clip!=3){u32 region[16]={32,1,1,16,2,1,6,5,2,1,6,5,0,0,0,0};if(out.input.clip==4){clear(region,64);region[0]=32;region[1]=1;}if(out.input.clip==2){u32 two[16]={32,1,2,32,0,0,8,6,0,0,3,2,5,3,8,6};for(u32 i=0;i<16;++i)region[i]=two[i];}require(((i32(WIN*)(void*,void*,u32))table(c)[7])(c,region,0)==0,31);
    out.clip_size=64;out.clip_result=((i32(WIN*)(void*,void*,void*,u32*))table(c)[3])(c,0,out.clip_data,&out.clip_size);require(out.clip_result==0,32);require(out.clip_size<=64,33);
   }else {out.clip_size=64;out.clip_result=((i32(WIN*)(void*,void*,void*,u32*))table(c)[3])(c,0,out.clip_data,&out.clip_size);}
   require(((i32(WIN*)(void*,void*))table(d)[28])(d,c)==0,34);
  }
  u32 held_desc[27];if(out.input.held)lock(out.input.held==1?s:d,held_desc);
  if(out.input.fast)out.result=((i32(WIN*)(void*,i32,i32,void*,void*,u32))table(d)[7])(d,out.after.dst.l,out.after.dst.t,s,&out.after.src,out.input.flags);
  else out.result=((i32(WIN*)(void*,void*,void*,void*,u32,void*))table(d)[5])(d,&out.after.dst,s,&out.after.src,out.input.flags,0);
  if(out.input.held)unlock(out.input.held==1?s:d);
  pixels(s,out.source_after,0,0,out.source_desc);pixels(d,out.dest_after,0,0,out.dest_desc);require(WriteFile(output,&out,sizeof(out),&n,0) && n==sizeof(out),17);release(d);release(s);release(c);
 }CloseHandle(input);CloseHandle(output);release(dd);ExitProcess(0);
}
