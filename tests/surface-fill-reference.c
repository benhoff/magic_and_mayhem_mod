/* Reuse PE mapping, real Surface2 storage and transparent Blt observation.
 * The original fill and error-reporting instructions remain unchanged.
 * Modal UI imports terminate at bounded observers, separately from driver calls.
 */
#define start unused_key_probe_start
#include "surface-key-reference.c"
#undef start
static u32 dialogs,destroys;
/* Freestanding struct copies can lower to this CRT-free helper. */
void* memcpy(void* destination,const void* source,u32 length){u8* d=destination;const u8* s=source;for(u32 i=0;i<length;++i)d[i]=s[i];return destination;}
static i32 WIN message(void* window,const char* text,const char* title,u32 flags){(void)window;(void)text;(void)title;(void)flags;++dialogs;return 1;}
static i32 WIN destroy(void* window){(void)window;++destroys;return 0;}
struct ClipRecord {u32 size,result,data[16];};
struct FillOutput {struct Output draw;u32 dialogs,destroys;struct ClipRecord before,after;};
static struct ClipRecord clip_state(void* c){struct ClipRecord r;clear(&r,sizeof(r));if(c){r.size=64;r.result=((i32(WIN*)(void*,void*,void*,u32*))table(c)[3])(c,0,r.data,&r.size);require(r.size<=64,35);}return r;}
void start(void){map_original();require(*(u8*)0x58bac0==0x83 && *(u8*)0x58bac1==0xec,50);
 /* Bind only UI IAT cells in private data. Original .text is never rewritten. */
 *(void**)0x5c520c=(void*)message;*(void**)0x5c5208=(void*)destroy;*(u32*)0x656614=0;
 void* dd=0;HANDLE dll=LoadLibraryA("ddraw.dll");require(dll!=0,10);typedef i32(WIN*Create)(void*,void**,void*);Create create=(Create)GetProcAddress(dll,"DirectDrawCreate");require(create && create(0,&dd,0)==0,11);require(((i32(WIN*)(void*,void*,u32))table(dd)[20])(dd,0,8)==0,13);
 HANDLE input=CreateFileA("inputs.bin",0x80000000,1,0,3,0,0),output=CreateFileA("outputs.bin",0x40000000,0,0,1,0,0);require(input!=(HANDLE)-1 && output!=(HANDLE)-1,14);u32 n;struct Input c;
 void* proxy_table[8]={0,0,0,0,0,(void*)blt,0,(void*)fast};void** proxy=proxy_table;
 for(;;){require(ReadFile(input,&c,sizeof(c),&n,0),15);if(!n)break;require(n==sizeof(c) && c.entry==0x58bac0 && c.mode<=4 && c.phase<=1,16);
  void* d=surface(dd);void* clipper=0;actual_destination=d;clear(&out,sizeof(out));out.input=c;dialogs=destroys=0;
  pixels(d,out.dest_before,1,1,&c,out.dest_desc);
  if(c.mode){require(((i32(WIN*)(void*,u32,void**,void*))table(dd)[4])(dd,0,&clipper,0)==0,30);
   if(c.mode!=3){u32 region[16]={32,1,1,16,2,1,6,5,2,1,6,5,0,0,0,0};
    if(c.mode==4){clear(region,64);region[0]=32;region[1]=1;}
    if(c.mode==2){u32 two[16]={32,1,2,32,0,0,8,6,0,0,3,2,5,3,8,6};for(u32 i=0;i<16;++i)region[i]=two[i];}
    require(((i32(WIN*)(void*,void*,u32))table(clipper)[7])(clipper,region,0)==0,31);
   }require(((i32(WIN*)(void*,void*))table(d)[28])(d,clipper)==0,34);
  }
  struct ClipRecord clip_before=clip_state(clipper);
  u32 target[16];clear(target,sizeof(target));target[2]=(u32)&proxy;target[4]=8;target[5]=6;
  typedef void(__attribute__((thiscall))*Fill)(void*,struct Rect*,u32);((Fill)c.entry)(target,c.phase?0:&out.input.dst,c.key);
  pixels(d,out.dest_after,0,1,&c,out.dest_desc);require(out.trace[0]==1 && out.trace[3]!=0x887601c2u,51);
  struct FillOutput result;result.draw=out;result.dialogs=dialogs;result.destroys=destroys;result.before=clip_before;result.after=clip_state(clipper);
  require(WriteFile(output,&result,sizeof(result),&n,0) && n==sizeof(result),17);release(d);release(clipper);
 }CloseHandle(input);CloseHandle(output);release(dd);ExitProcess(0);
}
