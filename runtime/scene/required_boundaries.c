/* Passive, opt-in normal-return observations for the pinned No-CD image.
 * Preserve GPR/flags/FX and LastError. Read metadata only; never call an
 * original helper, COM service, allocator or renderer to obtain observations.
 * The scopes of normal returns, failed processes and original recovery differ. */
#include "../shadow/win32_min.h"
#include "../../protocols/include/mnm/required_boundaries_v1.h"
API u32 WIN GetCurrentThreadId(void);
API u32 WIN GetTickCount(void);
#define ENTRY(n) extern void boundary_enter_##n(void)
ENTRY(0);ENTRY(1);ENTRY(2);ENTRY(3);ENTRY(4);ENTRY(5);ENTRY(6);ENTRY(7);
ENTRY(8);ENTRY(9);ENTRY(10);ENTRY(11);ENTRY(12);ENTRY(13);ENTRY(14);ENTRY(15);
ENTRY(16);ENTRY(17);ENTRY(18);ENTRY(19);ENTRY(20);ENTRY(21);ENTRY(22);
#undef ENTRY
extern void boundary_trace_leave(void);
u32 boundary_trampolines[MNM_BOUNDARY_HOOKS];
static HANDLE boundary_file;
static u32 sequence,loss,thread,worlds,world_changes;
static u32 last_world[21];
static struct Frame {u32 sp,caller,tag,receiver,entry;} frames[64];
struct Anchor {u32 address,length,pop,relative;u8 bytes[8];void(*entry)(void);};
/* Whole-instruction prefixes from original-anchors-01 and stack cleanup from
 * entry-returns-01. Only tag19 contains a displaced relative CALL. */
static const struct Anchor anchors[MNM_BOUNDARY_HOOKS]={
 {0x4e40d0,6,0,0,{0x83,0xec,0x6c,0x56,0x8b,0xf1},boundary_enter_0},
 {0x58d540,5,0,0,{0x83,0xec,0x20,0x8b,0xd1},boundary_enter_1},
 {0x4a7110,6,0,0,{0x64,0xa1,0,0,0,0},boundary_enter_2},
 {0x4a6f70,7,4,0,{0x6a,0xff,0x68,0xfb,0x04,0x5c,0},boundary_enter_3},
 {0x4a7030,6,0,0,{0x55,0x56,0x8b,0xf1,0x33,0xed},boundary_enter_4},
 {0x4a6e20,5,0,0,{0xa1,0x64,0x2c,0x64,0},boundary_enter_5},
 {0x4a7a80,5,0,0,{0xa1,0x84,0x7c,0x65,0},boundary_enter_6},
 {0x4a7d60,6,0,0,{0x55,0x56,0x8b,0xf1,0x33,0xed},boundary_enter_7},
 {0x4a7b80,7,0,0,{0x8b,0x01,0x56,0x57,0xff,0x50,0x20},boundary_enter_8},
 {0x4b2010,6,0,0,{0x55,0x56,0x8b,0xf1,0x33,0xed},boundary_enter_9},
 {0x473d70,6,0,0,{0x64,0xa1,0,0,0,0},boundary_enter_10},
 {0x474a50,7,4,0,{0x6a,0xff,0x68,0x6b,0xf6,0x5b,0},boundary_enter_11},
 {0x473c10,6,0,0,{0x55,0x56,0x8b,0xf1,0x33,0xed},boundary_enter_12},
 {0x4748a0,6,0,0,{0x51,0x53,0x56,0x57,0x8b,0xf9},boundary_enter_13},
 {0x474fe0,6,0,0,{0x55,0x56,0x8b,0xf1,0x33,0xed},boundary_enter_14},
 {0x475100,6,0,0,{0x64,0xa1,0,0,0,0},boundary_enter_15},
 {0x46a460,6,0,0,{0x81,0xec,0x14,0x01,0,0},boundary_enter_16},
 {0x46ab30,5,0,0,{0xa1,0x38,0x9e,0x5d,0},boundary_enter_17},
 {0x552210,6,0,0,{0x8b,0x81,0x37,0x0a,0,0},boundary_enter_18},
 {0x5522f0,5,0,1,{0xe8,0x7b,0xe0,0xff,0xff},boundary_enter_19},
 {0x4b2c10,6,0,0,{0x64,0xa1,0,0,0,0},boundary_enter_20},
 {0x4b2ae0,6,0,0,{0x53,0x56,0x8b,0xf1,0x33,0xdb},boundary_enter_21},
 {0x4b2990,5,0,0,{0x56,0x8b,0xf1,0x8b,0x06},boundary_enter_22}
};
static void copy(void* d,const void* s,u32 n){for(u32 i=0;i<n;++i)((u8*)d)[i]=((const u8*)s)[i];}
static void zero(void* p,u32 n){volatile u8* d=(volatile u8*)p;for(u32 i=0;i<n;++i)d[i]=0;}
static u32 get(u32 p){u32 v;copy(&v,(void*)p,4);return v;}
static void put(void* p,u32 v){copy(p,&v,4);}
static int readable(u32 p,u32 n){
 if(!p||!n||p>0xffffffffu-n)return 0;u32 end=p+n;
 while(p<end){u32 q[7];if(VirtualQuery((void*)p,q,28)!=28||q[4]!=0x1000||(q[5]&0x101)||!(q[5]&0xee)||q[0]>p||q[3]>0xffffffffu-q[0])return 0;u32 next=q[0]+q[3];if(next<=p)return 0;p=next<end?next:end;}return 1;
}
static void bad(u32 reason){__sync_bool_compare_and_swap(&loss,0,reason);}
static int owned(void){u32 t=GetCurrentThreadId();__sync_bool_compare_and_swap(&thread,0,t);if(thread!=t){bad(1);return 0;}return 1;}
static int write(const void* data,u32 size){u32 done=0;return WriteFile(boundary_file,data,size,&done,0)&&done==size;}
static u32 caller(u32 address){for(u32 i=0;i<MNM_BOUNDARY_HOOKS;++i)if(address>=boundary_trampolines[i]&&address-boundary_trampolines[i]<=anchors[i].length)return anchors[i].address+address-boundary_trampolines[i];return address;}
static void globals(u32* h){
 const u32 at[]={0x6e1f68,0x6e1f6c,0x6e1f70,0x6e1f74,0x6e1f78,0x6e1f88,0x6e1f7c,0x6e1f80,
  0x6a4fe8,0x6a49e8,0x658174,0x6a2dc8,0x6def50,0x6de6d5,0x6de558,0x6a28f8};
 for(u32 i=0;i<16;++i)h[24+i]=get(at[i]);
}
static void snapshot(u32* h,u32 object){
 h[6]=object;if(!readable(object,0x57))return;h[7]=get(object);h[8]=get(object+4);h[9]=get(object+8);h[10]=1;
 const u32 offsets[]={0x13,0x17,0x1b,0x23,0x27,0x2b,0x2f,0x33,0x37,0x3b,0x3f,0x43,0x47,0x4b,0x4f,0x53};
 for(u32 i=0;i<16;++i)h[40+i]=get(object+offsets[i]);
 u32 n=h[7]==0x5c63e4?6:h[7]==0x5c641c?4:h[7]==0x5c6644?5:h[7]==0x5c5ebc?21:0;
 u32 array=n?get(object+(h[7]==0x5c6644?0x53:0x4b)):0;
 if(n&&readable(array,n*4)){h[56]=n;for(u32 i=0;i<n;++i)h[57+i]=get(array+4*i);}
 const u32 callbacks[]={h[7]==0x5c63e4?get(object+0x47):0,h[7]==0x5c5ebc?get(object+0x4f):0};
 for(u32 i=0;i<2;++i)if(readable(callbacks[i],12)){h[80+4*i]=1;for(u32 j=0;j<3;++j)h[81+4*i+j]=get(callbacks[i]+4*j);}
 if(h[7]==0x5c5ebc&&readable(object,0x2a9)){h[88]=1;h[89]=get(object+0x257);h[90]=get(object+0x25b);h[91]=get(object+0x267);h[92]=get(object+0x2a5);}
}
static void emit(u32* h){
 if(!boundary_file||loss)return;if(sequence>=MNM_BOUNDARY_ROWS){bad(2);return;}
 h[0]=++sequence;h[3]=thread;h[4]=GetTickCount();globals(h);if(!write(h,MNM_BOUNDARY_ROW))bad(3);
}
void boundary_trace_enter(u32* r,u32 tag){
 u32 error=GetLastError();if(!boundary_file||loss){SetLastError(error);return;}
 if(tag>=MNM_BOUNDARY_HOOKS||!owned()){bad(4);SetLastError(error);return;}
 /* Realm exits may tail-dispatch indirectly. Observe entry without replacing
  * their continuation; no return/cleanup promise follows from those entries. */
 if(tag==18||tag==19){u32 h[128];zero(h,sizeof h);h[1]=MNM_BOUNDARY_ENTER;h[2]=tag;
  h[5]=caller(r[9]);snapshot(h,r[6]);for(u32 i=0;i<9;++i)h[12+i]=r[i];h[21]=error;
  emit(h);SetLastError(error);return;}
 struct Frame* f=0;for(u32 i=0;i<64;++i)if(!frames[i].sp){f=frames+i;break;}
 if(!f){bad(5);SetLastError(error);return;}
 u32* args=r+9;f->sp=(u32)args+4+anchors[tag].pop;f->caller=args[0];f->tag=tag;f->receiver=r[6];
 u32 h[128];zero(h,sizeof h);h[1]=MNM_BOUNDARY_ENTER;h[2]=tag;h[5]=caller(args[0]);snapshot(h,f->receiver);
 for(u32 i=0;i<9;++i)h[12+i]=r[i];h[21]=error;emit(h);f->entry=sequence;
 args[0]=(u32)boundary_trace_leave;SetLastError(error);
}
void boundary_trace_return(u32* r){
 u32 error=GetLastError(),sp=(u32)r+40;
 for(u32 i=0;i<64;++i)if(frames[i].sp==sp){
  struct Frame f=frames[i];frames[i].sp=0;r[9]=f.caller;
  if(!owned()||loss){SetLastError(error);return;}
  u32 h[128];zero(h,sizeof h);h[1]=MNM_BOUNDARY_RETURN;h[2]=f.tag;h[5]=caller(f.caller);h[11]=f.entry;snapshot(h,f.receiver);
  for(u32 j=0;j<9;++j)h[12+j]=r[j];h[21]=error;emit(h);SetLastError(error);return;
 }
 ExitProcess(96); /* No invented continuation after an unmatched stack slot. */
}
void boundary_trace_world(u32 pixels,u32 width,u32 height,u32 stride,u32 queue){
 if(!boundary_file||loss)return;u32 error=GetLastError();if(!owned()){SetLastError(error);return;}
 if(worlds==0xffffffffu){bad(6);SetLastError(error);return;}++worlds;
 u32 h[128];zero(h,sizeof h);h[1]=MNM_BOUNDARY_WORLD;h[2]=queue;h[6]=pixels;h[7]=width;h[8]=height;h[9]=stride;globals(h);
 u32 key[21];key[0]=pixels;key[1]=width;key[2]=height;key[3]=stride;for(u32 i=0;i<16;++i)key[4+i]=h[24+i];key[20]=0;
 /* Summarize every World, retaining changes only. Never deduplicate a failure. */
 int changed=worlds==1;for(u32 i=0;i<21;++i)if(key[i]!=last_world[i])changed=1;
 if(changed){copy(last_world,key,sizeof(key));h[11]=worlds;emit(h);++world_changes;}
 SetLastError(error);
}
void boundary_trace_close(void){
 if(!boundary_file)return;u32 error=GetLastError(),h[128];zero(h,sizeof h);h[0]=sequence+1;h[1]=MNM_BOUNDARY_END;h[2]=loss;h[3]=thread;h[4]=GetTickCount();h[5]=sequence;h[6]=worlds;h[7]=world_changes;
 for(u32 i=0;i<64;++i)if(frames[i].sp)++h[8];globals(h);write(h,sizeof(h));CloseHandle(boundary_file);boundary_file=0;SetLastError(error);
}
int boundary_trace_install(void){
 u32 error=GetLastError();char flag[2],directory[220];SetLastError(0);u32 n=GetEnvironmentVariableA("MNM_REQUIRED_BOUNDARIES",flag,2);
 if(!n){int absent=GetLastError()==203;SetLastError(error);return absent;}
 if(n!=1||flag[0]!='1'||boundary_file||(u32)GetModuleHandleA(0)!=0x400000)return 0;
 n=GetEnvironmentVariableA("MNM_SCENE_DIR",directory,sizeof directory);if(!n||n>=sizeof directory)return 0;
 for(u32 i=0;i<MNM_BOUNDARY_HOOKS;++i){const struct Anchor* a=anchors+i;if(!readable(a->address,a->length))return 0;for(u32 j=0;j<a->length;++j)if(*(u8*)(a->address+j)!=a->bytes[j])return 0;}
 char name[260];copy(name,directory,n);const char* suffix="\\required-boundaries-v1.bin";u32 at=n;while(*suffix)name[at++]=*suffix++;name[at]=0;
 boundary_file=CreateFileA(name,0x40000000,1,0,1,0x80,0);if(boundary_file==(HANDLE)-1){boundary_file=0;return 0;}
 u32 h[16];zero(h,sizeof h);copy(h,MNM_BOUNDARY_MAGIC,8);h[2]=MNM_BOUNDARY_VERSION;h[3]=MNM_BOUNDARY_HEADER;h[4]=MNM_BOUNDARY_ROW;h[5]=MNM_BOUNDARY_ROWS;h[6]=0x40209ca7;h[7]=MNM_BOUNDARY_HOOKS;if(!write(h,sizeof(h)))return 0;
 for(u32 i=0;i<MNM_BOUNDARY_HOOKS;++i){const struct Anchor* a=anchors+i;u8* t=(u8*)VirtualAlloc(0,a->length+5,0x3000,0x40);if(!t)return 0;copy(t,(void*)a->address,a->length);
  if(a->relative)put(t+1,a->address+5+get(a->address+1)-(u32)t-5);
  t[a->length]=0xe9;put(t+a->length+1,a->address+a->length-(u32)t-a->length-5);if(!FlushInstructionCache(GetCurrentProcess(),t,a->length+5))return 0;boundary_trampolines[i]=(u32)t;
 }
 for(u32 i=0;i<MNM_BOUNDARY_HOOKS;++i){const struct Anchor* a=anchors+i;u32 old,unused;if(!VirtualProtect((void*)a->address,a->length,0x40,&old))return 0;u8 patch[8]={0xe9,0,0,0,0,0x90,0x90,0x90};put(patch+1,(u32)a->entry-a->address-5);copy((void*)a->address,patch,a->length);
  if(!FlushInstructionCache(GetCurrentProcess(),(void*)a->address,a->length)||!VirtualProtect((void*)a->address,a->length,old,&unused))return 0;
 }
 SetLastError(error);return 1;
}
