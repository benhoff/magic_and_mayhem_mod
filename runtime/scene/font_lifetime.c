/* Opt-in NoCD font lifecycle observations. All lookup pointers stay private.
 * This observer never acquires a surface/DC or modifies a drawing argument. */
#include "../shadow/win32_min.h"
API u32 WIN GetCurrentThreadId(void);
API u32 WIN GetTickCount(void);
#define LIMIT (16u*1024u*1024u)
#define ROWS 65536u
static HANDLE file;
static u32 count,bytes,loss,owner,resources;
static struct Font {u32 object,raw,token,loads,live;} fonts[64];
static struct Call {u32 sp,caller,tag,font,prior,entry,byte;} calls[64];
static struct Source {u32 token,offset;} sources[4096];
static u32 source_count;
u32 font_trace_trampolines[11];
#define E(n) extern void font_trace_enter_##n(void)
E(0);E(1);E(2);E(3);E(4);E(5);E(6);E(7);E(8);E(9);E(10);
#undef E
extern void font_trace_leave(void);
static const u32 objects[4]={0x6b0400,0x656680,0x6df1c0,0x6a5138};
static void copy(void* d,const void* s,u32 n){for(u32 i=0;i<n;++i)((u8*)d)[i]=((const u8*)s)[i];}
static u32 get(u32 p){u32 v;copy(&v,(void*)p,4);return v;}
static void put(void* p,u32 v){copy(p,&v,4);}
static int readable(u32 p,u32 n){
 if(!p||!n||p>0xffffffffu-n)return 0;
 u32 end=p+n;
 while(p<end){u32 q[7];if(VirtualQuery((void*)p,q,28)!=28||q[4]!=0x1000||(q[5]&0x101)||!(q[5]&0xee)||q[0]>p||q[3]>0xffffffffu-q[0])return 0;u32 next=q[0]+q[3];if(next<=p)return 0;p=next<end?next:end;}
 return 1;
}
static void bad(u32 n){__sync_bool_compare_and_swap(&loss,0,n);}
static int owned(void){u32 id=GetCurrentThreadId();__sync_bool_compare_and_swap(&owner,0,id);if(owner!=id){bad(1);return 0;}return 1;}
static int write(const void* p,u32 n){u32 done=0;return WriteFile(file,p,n,&done,0)&&done==n;}
static void emit(u32* r,const void* body,u32 n){
 if(!file||loss)return;
 if(count>=ROWS||n>LIMIT-128||bytes>LIMIT-128-n){bad(2);return;}
 r[0]=128+n;r[1]=++count;r[3]=owner;r[4]=GetTickCount();
 if(!write(r,128)||(n&&!write(body,n)))bad(3);else bytes+=128+n;
}
static u32 font(u32 p){
 for(u32 i=0;i<64;++i){if(fonts[i].object==p)return i+1;}
 for(u32 i=0;i<64;++i)if(!fonts[i].object){fonts[i].object=p;return i+1;}
 bad(4);return 0;
}
static u32 role(u32 p){for(u32 i=0;i<4;++i)if(objects[i]==p)return i+1;return 0;}
void font_trace_enter(u32* r,u32 tag){
 u32 error=GetLastError();if(!file||loss){SetLastError(error);return;}
 if(tag>10||!owned()){bad(5);SetLastError(error);return;}
 u32 p=tag<8?objects[tag/2]:tag==10?0:r[6],f=p?font(p):0;
 if(p&&(!f||!readable(p,0x238))){bad(6);SetLastError(error);return;}
 struct Call* c=0;for(u32 i=0;i<64;++i)if(!calls[i].sp){c=calls+i;break;}
 if(!c){bad(7);SetLastError(error);return;}
 u32* a=r+9;c->sp=(u32)a+4+(tag==8?8:tag==9?16:0);c->caller=a[0];c->tag=tag;c->font=f;c->prior=f?fonts[f-1].token:0;c->byte=tag==9?a[1]&255:0;
 u32 h[32]={0};h[2]=1;h[5]=tag;h[6]=f;h[7]=c->prior;h[8]=role(p);h[9]=c->caller;h[10]=c->byte;h[11]=r[8];
 /* ECX is recorded only as a font-role/token, never a host pointer. */
 h[13]=f;char path[256];for(u32 i=0;i<256;++i)((volatile char*)path)[i]=0;
 if(tag==8){if(!readable(a[1],256)){bad(8);SetLastError(error);return;}for(u32 i=0;i<256;++i){path[i]=*(char*)(a[1]+i);if(!path[i])break;if(i==255)bad(8);}h[14]=a[2];}
 emit(h,tag==8?path:0,tag==8?256:0);c->entry=count;a[0]=(u32)font_trace_leave;SetLastError(error);
}
void font_trace_return(u32* r){
 u32 error=GetLastError(),sp=(u32)r+40;
 for(u32 i=0;i<64;++i)if(calls[i].sp==sp){
  struct Call c=calls[i];calls[i].sp=0;r[9]=c.caller;
  if(!owned()||loss){SetLastError(error);return;}
  struct Font* f=c.font?fonts+c.font-1:0;u32 h[32]={0};h[2]=2;h[5]=c.tag;h[6]=c.font;h[7]=c.prior;h[8]=f?role(f->object):0;h[9]=c.caller;h[10]=c.entry;h[11]=r[8];h[12]=c.tag==8?r[7]:0;
  if(f){
   if(!readable(f->object,0x238)){bad(9);SetLastError(error);return;}
   u32 raw=get(f->object);
   if(c.tag==8){
    if(raw&&(r[7]||raw!=f->raw)){f->raw=raw;f->token=++resources;f->live=1;++f->loads;}
    else if(!raw){f->raw=0;f->live=0;f->token=0;}
    h[13]=f->token;h[14]=f->loads;h[15]=r[7]!=0;h[16]=c.prior&&c.prior!=f->token;
   }else if(c.tag<8){
    if(raw){bad(10);SetLastError(error);return;}
    f->raw=0;f->token=0;f->live=0;
   }else if(raw!=f->raw){bad(11);SetLastError(error);return;}
   h[17]=get(f->object+0x230);h[18]=get(f->object+0x220);
  }
  emit(h,0,0);SetLastError(error);return;
 }
 /* Never return to an invented continuation on an unmatched return slot. */
 ExitProcess(96);
}
void font_trace_glyph(u32 frame,u32 x,u32 y,u32 caller,u32 queue,u32 sequence){
 if(!file||loss)return;
 u32 error=GetLastError();if(!owned()){SetLastError(error);return;}
 struct Call* parent=0;for(u32 i=0;i<64;++i)if(calls[i].sp&&calls[i].tag==9&&(!parent||calls[i].sp<parent->sp))parent=calls+i;
 u32 h[32]={0};h[2]=3;h[5]=9;h[9]=caller;h[19]=x;h[20]=y;h[21]=queue;h[22]=sequence;
 struct Font* f=0;u32 font_id=0;
 for(u32 i=0;i<64;++i)if(fonts[i].live&&readable(fonts[i].raw,40)){
  u32 span=get(fonts[i].raw+4);
  if(span>=40&&span<=1048576&&frame>=fonts[i].raw&&frame-fonts[i].raw<span){
   if(f){bad(14);SetLastError(error);return;}f=fonts+i;font_id=i+1;
  }
 }
 if(!f){h[23]=1;emit(h,0,0);SetLastError(error);return;}
 if((parent&&parent->font!=font_id)||get(f->object)!=f->raw||!readable(frame,40)){bad(12);SetLastError(error);return;}
 u32 size=get(frame),rawsize=readable(f->raw,40)?get(f->raw+4):0;
 if(size<40||size>1048576||rawsize<40||rawsize>1048576||frame<f->raw||frame-f->raw>rawsize||size>rawsize-(frame-f->raw)||!readable(frame,size)){bad(13);SetLastError(error);return;}
 h[6]=font_id;h[7]=f->token;h[8]=role(f->object);h[10]=parent?parent->entry:0;h[13]=parent?parent->byte:0;h[14]=frame-f->raw;h[15]=size;h[23]=parent?0:2;
 /* FNV is a join diagnostic, not an independent pixel/asset comparison. */
 u32 hash=2166136261u;for(u32 i=0;i<size;++i)hash=(hash^(i>=28&&i<32?0:*(u8*)(frame+i)))*16777619u;h[16]=hash;
 u32 known=0;for(u32 i=0;i<source_count;++i)if(sources[i].token==f->token&&sources[i].offset==h[14]){known=1;break;}
 if(!known){
  if(source_count==4096){bad(15);SetLastError(error);return;}
  u8* body=(u8*)HeapAlloc(GetProcessHeap(),0,size);if(!body){bad(16);SetLastError(error);return;}
  copy(body,(void*)frame,size);for(u32 i=28;i<32;++i)body[i]=0;h[24]=1;
  emit(h,body,size);HeapFree(GetProcessHeap(),0,body);
  sources[source_count].token=f->token;sources[source_count++].offset=h[14];
 }else emit(h,0,0);
 SetLastError(error);
}
void font_trace_close(void){
 if(!file)return;
 u32 error=GetLastError(),h[32]={0};h[0]=128;h[1]=count+1;h[2]=4;h[3]=owner;h[5]=loss;h[6]=count;h[7]=bytes;h[8]=resources;
 for(u32 i=0;i<64;++i){if(calls[i].sp)++h[9];if(fonts[i].live)++h[10];}
 write(h,128);CloseHandle(file);file=0;SetLastError(error);
}
int font_trace_install(void){
 char flag[2],directory[220];SetLastError(0);u32 n=GetEnvironmentVariableA("MNM_FONT_LIFETIME",flag,2);
 if(!n)return GetLastError()==203;
 if(n!=1||flag[0]!='1'||file)return 0;
 n=GetEnvironmentVariableA("MNM_SCENE_DIR",directory,sizeof directory);if(!n||n>=sizeof directory)return 0;
 const u32 addresses[11]={0x4be870,0x4be8c0,0x4be910,0x4be960,0x4be9b0,0x4bea00,0x4bea50,0x4beaa0,0x4a5ae0,0x4a59e0,0x5575b0};
 const u8 anchor[11][7]={{0x33,0xc0,0x68,0xc0,0xe8,0x4b,0},{0xa1,0,4,0x6b,0},{0x33,0xc0,0x68,0x60,0xe9,0x4b,0},{0xa1,0x80,0x66,0x65,0},{0x33,0xc0,0x68,0,0xea,0x4b,0},{0xa1,0xc0,0xf1,0x6d,0},{0x33,0xc0,0x68,0xa0,0xea,0x4b,0},{0xa1,0x38,0x51,0x6a,0},{0x8b,0x44,0x24,4,0x53},{0x53,0x8b,0x5c,0x24,8},{0xa1,0xd5,0xe6,0x6d,0}};
 void(*entries[11])(void)={font_trace_enter_0,font_trace_enter_1,font_trace_enter_2,font_trace_enter_3,font_trace_enter_4,font_trace_enter_5,font_trace_enter_6,font_trace_enter_7,font_trace_enter_8,font_trace_enter_9,font_trace_enter_10};
 if((u32)GetModuleHandleA(0)!=0x400000)return 0;
 for(u32 i=0;i<11;++i){u32 len=i<8&&!(i&1)?7:5;if(!readable(addresses[i],len))return 0;for(u32 j=0;j<len;++j)if(*(u8*)(addresses[i]+j)!=anchor[i][j])return 0;}
 char name[260];copy(name,directory,n);const char* suffix="\\font-lifetime-v2.bin";u32 at=n;while(*suffix)name[at++]=*suffix++;name[at]=0;
 file=CreateFileA(name,0x40000000,1,0,1,0x80,0);if(file==(HANDLE)-1){file=0;return 0;}
 u32 h[16]={0};copy(h,"MNMFLT02",8);h[2]=2;h[3]=64;h[4]=0x40209ca7;h[5]=LIMIT;h[6]=ROWS;if(!write(h,64))return 0;bytes=64;
 for(u32 i=0;i<11;++i){u32 len=i<8&&!(i&1)?7:5;u8* t=(u8*)VirtualAlloc(0,len+5,0x3000,0x40);if(!t)return 0;copy(t,(void*)addresses[i],len);t[len]=0xe9;put(t+len+1,addresses[i]+len-(u32)t-len-5);if(!FlushInstructionCache(GetCurrentProcess(),t,len+5))return 0;font_trace_trampolines[i]=(u32)t;}
 for(u32 i=0;i<11;++i){u32 len=i<8&&!(i&1)?7:5,old,unused;if(!VirtualProtect((void*)addresses[i],len,0x40,&old))return 0;u8 patch[7]={0xe9,0,0,0,0,0x90,0x90};put(patch+1,(u32)entries[i]-addresses[i]-5);copy((void*)addresses[i],patch,len);if(!FlushInstructionCache(GetCurrentProcess(),(void*)addresses[i],len)||!VirtualProtect((void*)addresses[i],len,old,&unused))return 0;}
 return 1;
}
