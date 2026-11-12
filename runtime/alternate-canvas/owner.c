/* Opt-in repair for the pinned original dispatcher, not a recovered allocator.
 * Only this module owns its published pair. Original rasters/copy remain intact. */
#include "../shadow/win32_min.h"
#include "alternate_owner_pinned.h"
API int WIN VirtualFree(void*,u32,u32);
API u32 WIN GetCurrentThreadId(void);
API u32 WIN SetFilePointer(HANDLE,i32,void*,u32);
API int WIN FlushFileBuffers(HANDLE);
void* memset(void* p,int v,unsigned n){u8* d=p;while(n--)*d++=(u8)v;return p;}
void* memcpy(void* p,const void* q,unsigned n){u8* d=p;const u8* s=q;while(n--)*d++=*s++;return p;}
#define HOOKS 6
extern void alternate_owner_leave(void);
#define ENTRY(n) extern void alternate_owner_enter_##n(void)
ENTRY(0);ENTRY(1);ENTRY(2);ENTRY(3);ENTRY(4);ENTRY(5);
u32 alternate_owner_trampolines[HOOKS];
static void (*const entries[HOOKS])(void)={alternate_owner_enter_0,alternate_owner_enter_1,
 alternate_owner_enter_2,alternate_owner_enter_3,alternate_owner_enter_4,alternate_owner_enter_5};
static u32 header[64]={0x414e4d4d,1,256},rows;
static HANDLE output;static u32 installed;
static u8 *reservation,*pixels;static u32 committed,bytes,source,width,height,padx,pady,stride,generation;
static u32 thread,initializing,handled_padding;
static struct Frame {u32 sp,caller,tag;} frames[64];
static u32 get(u32 p){return *(volatile u32*)p;}
static void put(u32 p,u32 v){*(volatile u32*)p=v;}
static int same(const void* p,const void* q,u32 n){const u8* a=p;const u8* b=q;while(n--)if(*a++!=*b++)return 0;return 1;}
static void write_at(u32 offset,const void* p,u32 n){u32 done;if(!output)return;
 if(SetFilePointer(output,(i32)offset,0,0)!=offset||!WriteFile(output,p,n,&done,0)||done!=n)ExitProcess(191);
}
static void save(void){header[10]=get(0x6e1f68);header[11]=get(0x6e1f6c);write_at(0,header,sizeof(header));}
static void event(u32 kind,u32 caller){
 if(rows==2048){save();return;}
 u32 row[20]={++rows,kind,generation,(u32)pixels,source,width,height,padx,pady,stride,bytes,caller,
  get(0x658174),get(0x6a2dc8),get(0x6def50),get(0x6e1f68),get(0x6e1f6c),header[7],header[8],header[9]};
 header[22]=rows;write_at(256+(rows-1)*sizeof(row),row,sizeof(row));save();
}
static void clear_owned(void){
 u32 lo=(u32)pixels,hi=lo+bytes;
 for(u32 p=0x6e1f68;p<=0x6e1f6c;p+=4){u32 v=get(p);if(lo&&v>=lo&&v<hi)put(p,0);}
 u32 active=get(0x658174);
 if(lo&&active>=lo&&active<hi){put(0x658174,0);put(0x6a2dc8,0);put(0x6def50,0);}
}
static void fail(u32 code){
 header[9]=code;clear_owned();
 if(reservation){VirtualFree(reservation,0,0x8000);reservation=0;pixels=0;source=0;++header[6];}
 if(output){event(9,0);FlushFileBuffers(output);}ExitProcess(170+code);
}
static void check_thread(void){u32 id=GetCurrentThreadId();if(!thread){thread=id;header[23]=id;}else if(id!=thread)fail(1);}
static void guards(void){
 if(!reservation)return;
 for(u32 i=0;i<256;++i)if(reservation[4096+i]!=0xa5)fail(2);
 for(u32 i=256+bytes;i<committed;++i)if(reservation[4096+i]!=0xa5)fail(2);
 ++header[19];
}
static void retire(u32 caller){
 if(!reservation){if(get(0x6e1f68)||get(0x6e1f6c))fail(3);return;}
 guards();if(get(0x6e1f68)!=(u32)pixels||get(0x6e1f6c)!=source)fail(3);
 clear_owned();if(!VirtualFree(reservation,0,0x8000))fail(4);
 reservation=0;pixels=0;source=0;++header[6];event(4,caller);
}
static void allocate(void){
 if(reservation||get(0x6e1f68)||get(0x6e1f6c))fail(3);
 width=get(0x6de6d5)?800:640;height=get(0x6de6d5)?600:480;
 padx=get(0x6e1f70);pady=get(0x6e1f74);
 u32 red=*(u16*)0x6e1f7c,green=*(u16*)0x6e1f7e,blue=*(u16*)0x6e1f80;
 if(!red||!green||!blue||(red&green)||(red&blue)||(green&blue)||((red|green|blue)!=0xffff&&(red|green|blue)!=0x7fff))fail(5);
 header[30]=red;header[31]=green;header[32]=blue;
 if(padx!=96||pady||get(0x6a49f0)!=width||get(0x6a49f4)!=height)fail(5);
 stride=width+2*padx;bytes=stride*(height+2*pady)*2;committed=(bytes+512+4095)&~4095u;
#ifdef MNM_ALT_OWNER_SELFTEST
 char refuse[2];if(GetEnvironmentVariableA("MNM_ALT_OWNER_TEST_ALLOC_FAIL",refuse,2))fail(6);
#endif
 reservation=VirtualAlloc(0,committed+8192,0x2000,1);
 if(!reservation)fail(6);
 if(VirtualAlloc(reservation+4096,committed,0x1000,4)!=reservation+4096)fail(6);
 memset(reservation+4096,0xa5,committed);pixels=reservation+4096+256;memset(pixels,0,bytes);
 source=(u32)pixels+2*(pady*stride+padx);++generation;++header[4];
 header[12]=width;header[13]=height;header[14]=padx;header[15]=pady;
 header[16]=width;header[17]=stride;header[18]=height+2*pady;
 event(1,0);
 /* The branch selector is published last, after a complete source and clear. */
 put(0x6e1f6c,source);put(0x6e1f68,(u32)pixels);++header[5];event(2,0);
}
static void validate_copy(int after){
 if(!reservation||get(0x6e1f68)!=(u32)pixels||get(0x6e1f6c)!=source)fail(3);
 if(get(0x6a2dc8)!=width||get(0x6def50)!=height||get(0x6e1f70)!=padx||get(0x6e1f74)!=pady||
    get(0x658174)==(u32)pixels||!get(0x658174))fail(7);
 guards();if(!after)return;
 ++header[7];
 if(header[7]<=8||!(header[7]&63)){
  const u16* from=(const u16*)source;const u16* to=(const u16*)get(0x658174);
  for(u32 y=0;y<height;++y)for(u32 x=0;x<width;++x){
   u16 value=from[y*stride+x];if(value!=to[y*width+x])fail(8);header[33]+=value!=0;
  }
  ++header[20];header[21]+=width*height;event(3,0);
 }else save();
}
void alternate_owner_enter(u32* r,u32 tag){
 u32 error=GetLastError();u32* stack=r+9;u32 caller=stack[0];int observe_return=0;
 if(tag==0){
  check_thread();if(initializing)fail(9);retire(caller);initializing=1;++header[25];
  if(!get(0x6e1f78)&&handled_padding){put(0x6e1f70,0);put(0x6e1f74,0);++header[29];}
  observe_return=1;
 }else if(tag==1&&initializing&&get(0x6e1f78)&&r[6]==0x6a49e0){
  check_thread();u32* d=(u32*)stack[1];width=get(0x6de6d5)?800:640;height=get(0x6de6d5)?600:480;
  if(d[0]!=108||d[1]!=7||d[2]!=height||d[3]!=width+96||d[26]!=0x840)fail(5);
  d[3]=width;d[2]=height;++header[27];handled_padding=1;event(5,caller);
 }else if(tag==2&&(r[6]==0x6a49e0||r[6]==0x6a4fe0)){check_thread();retire(caller);
 }else if(tag==3){check_thread();++header[26];retire(caller);event(6,caller);FlushFileBuffers(output);
 }else if(tag==4&&reservation){check_thread();validate_copy(0);observe_return=1;
 }else if(tag==5&&caller==0x4fd480){
  check_thread();if(!reservation||r[6]!=(u32)pixels||r[5]!=stride||stack[1]!=height+2*pady)fail(10);
  ++header[8];if(header[8]<=4)event(7,caller);
 }
 if(observe_return){
  for(u32 i=0;i<64;++i)if(!frames[i].sp){frames[i]=(struct Frame){(u32)stack+4,caller,tag};stack[0]=(u32)&alternate_owner_leave;SetLastError(error);return;}
  fail(11);
 }
 SetLastError(error);
}
void alternate_owner_return(u32* r){
 u32 error=GetLastError(),sp=(u32)r+40;check_thread();
 for(u32 i=0;i<64;++i)if(frames[i].sp==sp){
  struct Frame f=frames[i];frames[i].sp=0;r[9]=f.caller;
  if(f.tag==0){initializing=0;if(r[7]!=1)fail(12);if(get(0x6e1f78))allocate();else event(8,f.caller);}
  else if(f.tag==4)validate_copy(1);
  SetLastError(error);return;
 }
 fail(11);
}
static int install(void){
 if(installed)return 0;
#ifndef MNM_ALT_OWNER_SELFTEST
 if((u32)GetModuleHandleA(0)!=0x400000)return 0;
#endif
 for(u32 i=0;i<HOOKS;++i)if(!same((void*)owner_hooks[i].address,owner_hooks[i].bytes,owner_hooks[i].length))return 0;
 if(get(0x6e1f68)||get(0x6e1f6c))return 0;
 char path[1024];u32 size=GetEnvironmentVariableA("MNM_ORIGINAL_ALTERNATE_LOG",path,sizeof(path));
 if(size>=sizeof(path))return 0;
 if(size){output=CreateFileA(path,0x40000000,1,0,1,0x80,0);if(output==(HANDLE)-1){output=0;return 0;}}
 installed=1;
 header[24]=1;header[28]=HOOKS;save();
 /* Allocate and seal every trampoline before mutating an original entry. */
 for(u32 i=0;i<HOOKS;++i){
  u32 n=owner_hooks[i].length;u8* t=VirtualAlloc(0,n+5,0x3000,4);u32 old;
  if(!t)fail(6);memcpy(t,owner_hooks[i].bytes,n);t[n]=0xe9;*(u32*)(t+n+1)=owner_hooks[i].address+n-(u32)t-n-5;
  if(!VirtualProtect(t,n+5,0x20,&old)||!FlushInstructionCache(GetCurrentProcess(),t,n+5))fail(13);
  alternate_owner_trampolines[i]=(u32)t;
 }
 for(u32 i=0;i<HOOKS;++i){u32 old,unused,n=owner_hooks[i].length;u8* p=(u8*)owner_hooks[i].address;
  if(!VirtualProtect(p,n,0x40,&old))fail(13);p[0]=0xe9;*(u32*)(p+1)=(u32)entries[i]-(u32)p-5;
  for(u32 j=5;j<n;++j)p[j]=0x90;
  if(!VirtualProtect(p,n,old,&unused)||!FlushInstructionCache(GetCurrentProcess(),p,n))fail(13);
 }return 1;
}
void AlternateOwnerAnchor(void){}
#ifdef MNM_ALT_OWNER_SELFTEST
__declspec(dllexport) int WIN AlternateOwnerInstallForTest(void){return install();}
__declspec(dllexport) void WIN AlternateOwnerSaturateLogForTest(void){rows=2048;}
#endif
int WIN DllMain(void* module,u32 reason,void* reserved){
 (void)module;(void)reserved;
 if(reason==1){
#ifndef MNM_ALT_OWNER_SELFTEST
  char flag[2];u32 n=GetEnvironmentVariableA("MNM_ORIGINAL_ALTERNATE_OWNER",flag,2);
  if(n&&(n!=1||flag[0]!='1'||!install()))return 0;
#endif
 }else if(reason==0&&output){save();FlushFileBuffers(output);CloseHandle(output);output=0;}
 return 1;
}
