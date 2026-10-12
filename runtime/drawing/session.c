/* Original-active NoCD API observation. All pixels are independent oracles. */
#include "../shadow/win32_min.h"
API void* WIN GetProcAddress(void*,const char*);
API u32 WIN GetCurrentThreadId(void);
API u32 WIN GetTickCount(void);
API void WIN InitializeCriticalSection(void*);
API void WIN EnterCriticalSection(void*);
API void WIN LeaveCriticalSection(void*);
API i32 WIN VirtualFree(void*,u32,u32);
struct Table {void** vt;void* original[36];u32 kind;};
struct Identity {void* object;u32 id;};
struct Context {u32 r[32],args[6],observed,interval_related;};
static struct Table tables[64];static u32 table_count;
static struct Identity identities[1024];static u32 next_identity;
static HANDLE journal;static char directory[512];
static u32 enabled,records,entries,planes,drops,gaps,bytes,readbacks_enabled=1;static u32 journal_lock[6],pending;
static void* original_create;static void* original_create_ex;static void* original_proc;
static void* original_async;static void* original_key;static void* original_cursor;static void* original_map;
void* memset(void* out,int value,u32 n){u8* p=out;while(n--)*p++=(u8)value;return out;}
static void copy(void* out,const void* in,u32 n){u8* d=out;const u8* s=in;while(n--)*d++=*s++;}
static int readable(const void* pointer,u32 n){u32 at=(u32)pointer,end=at+n;if(!at||end<at)return 0;while(at<end){u32 m[7];if(VirtualQuery((void*)at,m,28)!=28||m[4]!=0x1000||(m[5]&0x101))return 0;u32 next=m[0]+m[3];if(next<=at)return 0;at=next;}return 1;}
static int equal(const void* a,const void* b,u32 n){const u8* x=a,*y=b;while(n--)if(*x++!=*y++)return 0;return 1;}
static void path(char* out,const char* suffix){u32 i=0;while(directory[i]){out[i]=directory[i];++i;}out[i++]='\\';while(*suffix)out[i++]=*suffix++;out[i]=0;}
static int write(HANDLE f,const void* data,u32 n){u32 count;return WriteFile(f,data,n,&count,0)&&count==n;}
/* Definitions are shared by the two additive diagnostic journals. */
static u32 physical_enabled,physical_byte_limit=1024u*1024u*1024u;
#include "call_intervals.h"
#include "source_points.h"
static struct Table* table(void* object){void** vt=*(void***)object;for(u32 i=0;i<table_count;++i)if(tables[i].vt==vt)return tables+i;return 0;}
static u32 identity(void* object){if(!object)return 0;for(u32 i=0;i<1024;++i)if(identities[i].object==object)return identities[i].id;for(u32 i=0;i<1024;++i)if(!identities[i].object){identities[i].object=object;return identities[i].id=++next_identity;}++gaps;return 0;}
static void emit(u32* r){if(!journal)return;r[0]=++records;if(records>262144||!write(journal,r,128)){++drops;enabled=0;}}
static void wine_submission_close(u32);
static u32 wine_blit_worker_pending;
static void finish(u32 reason){if(!journal)return;u32 r[32]={0};r[1]=4;r[2]=reason;r[6]=GetCurrentThreadId();r[11]=GetTickCount();r[21]=records;r[22]=planes;r[23]=drops;r[24]=gaps;r[25]=bytes;emit(r);interval_close(reason);producer_points_close(reason);wine_submission_close(reason);CloseHandle(journal);journal=0;enabled=0;char name[600];path(name,"session.done");HANDLE f=CreateFileA(name,0x40000000,3,0,1,0x80,0);if(f!=(HANDLE)-1){write(f,r,128);CloseHandle(f);}}
static void install(void*,u32);
/* Observer APIs preserve the application's result LastError and x87/SSE state. */
static void snapshot_fp(void* p){__asm__ volatile("fxsave (%0)"::"r"(p):"memory");}
static void restore_fp(void* p){__asm__ volatile("fxrstor (%0)"::"r"(p):"memory");}
#include "wine_submission.h"
static struct Context begin(void* object,u32 method,u32* args,u32 count,u32 caller,u32 type){
 struct Context c={0};c.r[7]=GetLastError();if(!enabled||(type==1&&(caller<0x400000||caller>=0x5c0000)))return c;
 u8 fp[512] __attribute__((aligned(16)));snapshot_fp(fp);
 EnterCriticalSection(journal_lock);if(!enabled){LeaveCriticalSection(journal_lock);restore_fp(fp);SetLastError(c.r[7]);return c;}++pending;
 c.observed=1;c.r[1]=type;c.r[2]=method;c.r[3]=identity(object);c.r[5]=caller;c.r[6]=GetCurrentThreadId();c.r[10]=GetTickCount();c.r[29]=++entries;
 for(u32 i=0;i<count;++i)c.args[i]=args[i];
 /* Owned scalar call inputs. Process pointers are never a portable input. */
 if(type==1){
  if(method==0&&readable((void*)args[0],16))copy(c.r+21,(void*)args[0],16);
  if(method==5){c.r[4]=identity((void*)args[1]);c.r[21]=args[3];if(readable((void*)args[0],16)){copy(c.r+22,(void*)args[0],16);c.r[31]|=1;}if(readable((void*)args[2],16)){copy(c.r+26,(void*)args[2],12);copy(c.r+20,(u8*)args[2]+12,4);c.r[31]|=2;}}
  else if(method==7){c.r[4]=identity((void*)args[2]);c.r[21]=args[4];c.r[22]=args[0];c.r[23]=args[1];if(readable((void*)args[3],16))copy(c.r+24,(void*)args[3],16);}
  else if(method==11){c.r[4]=identity((void*)args[0]);c.r[21]=args[1];}
  else if(method==25){c.r[21]=args[2];if(readable((void*)args[0],16))copy(c.r+22,(void*)args[0],16);}
  else if(method==6){c.r[21]=args[2];c.r[22]=args[1];}
  else if(method==29){c.r[21]=args[0];if(readable((void*)args[1],8))copy(c.r+22,(void*)args[1],8);}
  else if(method==30){c.r[21]=args[0];c.r[22]=args[1];}
  else if(method==13||method==18||method==34||method==35)c.r[21]=args[0];
 }else if(type==2){c.r[21]=args[0];if(count>1)c.r[22]=args[1];}
 else if(type==5)for(u32 i=0;i<count&&i<6;++i)c.r[21+i]=args[i];
 c.interval_related=c.r[4];interval_event(&c,1);LeaveCriticalSection(journal_lock);wine_submission_parent(&c,1);producer_points_capture(&c,object,1);restore_fp(fp);SetLastError(c.r[7]);return c;
}
struct Pixels {void* data;u32 size;};
/* Never hold the journal mutex across a driver call: Lock/GetDesc can
 * synchronously invoke original input/window handlers on another thread. */
#include "physical_fence.h"
static struct Pixels primary(struct Context* c,void* object,struct Table* t){
 struct Pixels out={0};if(!readbacks_enabled&&!physical_enabled){c->r[13]=2;return out;}u32 desc[31]={0};desc[0]=t->kind>=14?124:108;
 typedef i32(WIN *Desc)(void*,void*);typedef i32(WIN *Lock)(void*,void*,void*,u32,HANDLE);typedef i32(WIN *Unlock)(void*,void*);
 i32 h=((Desc)t->original[22])(object,desc);c->r[13]=h<0?2:!!(desc[26]&0x200);c->r[17]=desc[26];c->r[18]=c->r[19]=0xffffffffu;
 if(h<0){__sync_fetch_and_add(&gaps,1);return out;}if(!c->r[13])return out;
 if(physical_enabled){
  c->r[14]=desc[3];c->r[15]=desc[2];c->r[16]=desc[21];
  if(!desc[3]||!desc[2]||desc[3]>2048||desc[2]>2048||desc[21]!=16||desc[22]!=0xf800||desc[23]!=0x7e0||desc[24]!=31){__sync_fetch_and_add(&gaps,1);c->r[13]=2;}
  return out;
 }
 desc[0]=t->kind>=14?124:108;h=((Lock)t->original[25])(object,0,desc,0x4810,0);c->r[18]=h;if(h<0){__sync_fetch_and_add(&drops,1);return out;}
 u32 w=desc[3],height=desc[2],pitch=desc[4],p=desc[9];c->r[14]=w;c->r[15]=height;c->r[16]=desc[21];
 int okay=w&&height&&w<=2048&&height<=2048&&desc[21]==16&&desc[22]==0xf800&&desc[23]==0x7e0&&desc[24]==31&&pitch>=w*2&&pitch<=32768&&readable((void*)p,(height-1)*pitch+w*2);
 if(okay){out.size=w*height*2;out.data=VirtualAlloc(0,out.size,0x3000,4);okay=out.data!=0;if(okay)for(u32 y=0;y<height;++y)copy((u8*)out.data+y*w*2,(void*)(p+y*pitch),w*2);}
 c->r[19]=((Unlock)t->original[32])(object,t->kind>=14?0:(void*)p);
 if(!okay||(i32)c->r[19]<0){if(out.data)VirtualFree(out.data,0,0x8000);out.data=0;__sync_fetch_and_add(&drops,1);}return out;
}
static void publish(struct Context* c,struct Pixels out){
 if(!out.data)return;if(planes>=1024||out.size>1073741824u-bytes){++drops;return;}
 char name[600],suffix[32]="api-oracle-000000.565";u32 n=planes+1;for(i32 i=16;i>=11;--i){suffix[i]=(char)('0'+n%10);n/=10;}path(name,suffix);
 HANDLE f=CreateFileA(name,0x40000000,3,0,1,0x80,0);int okay=f!=(HANDLE)-1;if(okay){okay=write(f,out.data,out.size);if(!CloseHandle(f))okay=0;}
 if(okay){c->r[12]=++planes;bytes+=out.size;}else ++drops;
}
static u32 kind(const u8* guid){
 static const u8 ids[9][16]={{0x80,0xdb,0x14,0x6c,0x33,0xa7,0xce,0x11,0xa5,0x21,0,0x20,0xaf,0x0b,0xe5,0x60},{0xe0,0xf3,0xa6,0xb3,0x43,0x2b,0xcf,0x11,0xa2,0xde,0,0xaa,0,0xb9,0x33,0x56},{0x9a,0x50,0x59,0x9c,0xbd,0x39,0xd1,0x11,0x8c,0x4a,0,0xc0,0x4f,0xd9,0x30,0xc5},{0xc0,0x5e,0xe6,0x15,0x9c,0x3b,0xd2,0x11,0xb9,0x2f,0,0x60,0x97,0x97,0xea,0x5b},{0x81,0xdb,0x14,0x6c,0x33,0xa7,0xce,0x11,0xa5,0x21,0,0x20,0xaf,0x0b,0xe5,0x60},{0x85,0x58,0x80,0x57,0xec,0x6e,0xcf,0x11,0x94,0x41,0xa8,0x23,3,0xc1,0x0e,0x27},{0,0x4e,0x04,0xda,0xb2,0x69,0xd0,0x11,0xa1,0xd5,0,0xaa,0,0xb8,0xdf,0xbb},{0x30,0x86,0x2b,0x0b,0x35,0xad,0xd0,0x11,0x8e,0xa6,0,0x60,0x97,0x97,0xea,0x5b},{0x80,0x5a,0x67,0x06,0x9b,0x3b,0xd2,0x11,0xb9,0x2f,0,0x60,0x97,0x97,0xea,0x5b}};
 static const u32 kinds[]={1,2,4,7,11,12,13,14,17};if(!readable(guid,16))return 0;for(u32 i=0;i<9;++i)if(equal(guid,ids[i],16))return kinds[i];return 0;
}
static void acquired(void* alias,u32 k,const void* guid){
 static const u8 unknown[16]={0,0,0,0,0,0,0,0,0xc0,0,0,0,0,0,0,0x46};
 if(k){install(alias,k);return;}
 /* IUnknown may only reuse an already observed vtable. Never assume that an
  * arbitrary three-slot IUnknown interface has a surface ABI. */
 if(readable(guid,16)&&equal(guid,unknown,16)&&readable(alias,4)&&table(alias))return;
 ++gaps;
}
static u32 end(struct Context* c,void* object,u32 result){
 u32 error=GetLastError();if(!c->observed||!enabled){SetLastError(error);return result;}u8 fp[512] __attribute__((aligned(16)));snapshot_fp(fp);wine_submission_parent(c,2);
 struct Pixels pixels={0};struct Table* capture_table=0;EnterCriticalSection(journal_lock);
 c->r[8]=result;c->r[9]=error;c->r[11]=GetTickCount();
 interval_event(c,2);
 if(c->r[1]==1){u32 m=c->r[2];struct Table* t=m==2?0:table(object);
  if((i32)result>=0&&(m==0||m==12)){u32* output=(u32*)c->args[m==0?1:1];if(readable(output,4)&&*output){void* alias=(void*)*output;u32 k=m==0?kind((void*)c->args[0]):t->kind;acquired(alias,k,(void*)c->args[0]);c->r[4]=identity(alias);}}
  if(m==4||m==9||m==10||m==30||m>=33)++gaps; /* Overlay/callback surfaces require separate physical-output proof. */
  if((i32)result>=0&&(m==5||m==6||m==7||m==11||m==26||m==27||m==31||m==32))capture_table=t;
  if(m==2&&!result)for(u32 i=0;i<1024;++i)if(identities[i].object==object)identities[i].object=0;
 }else if(c->r[1]==2&&c->r[2]==3&&(i32)result>0&&readable((void*)c->args[0],8)){copy(c->r+27,(void*)c->args[0],8);c->r[31]=1;}
 LeaveCriticalSection(journal_lock);producer_points_capture(c,object,2);if(capture_table)pixels=primary(c,object,capture_table);
 EnterCriticalSection(journal_lock);if(!enabled){LeaveCriticalSection(journal_lock);if(pixels.data)VirtualFree(pixels.data,0,0x8000);restore_fp(fp);SetLastError(error);return result;}publish(c,pixels);
 if(physical_enabled&&c->r[1]==1&&c->r[13]==1&&!(c->r[8]&0x80000000u)){
  u32 size=c->r[14]*c->r[15]*4;
  if(planes>=1024||bytes>physical_byte_limit||size>physical_byte_limit-bytes){++drops;}else{c->r[12]=++planes;bytes+=size;}
 }
 c->r[30]=planes;emit(c->r);
 LeaveCriticalSection(journal_lock);physical_fence(c);producer_points_capture(c,object,3);EnterCriticalSection(journal_lock);interval_event(c,3);--pending;
 char name[600];path(name,"stop.flag");HANDLE stop=CreateFileA(name,0x80000000,3,0,3,0x80,0);if(stop!=(HANDLE)-1){CloseHandle(stop);if(!pending&&!wine_blit_worker_pending)finish(2);}
 LeaveCriticalSection(journal_lock);if(pixels.data)VirtualFree(pixels.data,0,0x8000);restore_fp(fp);SetLastError(error);return result;
}
static u32 WIN surface_0(void* o,u32 a0,u32 a1){u32 a[6]={a0,a1};struct Context c=begin(o,0,a,2,(u32)__builtin_return_address(0),1);u32 r=((u32(WIN*)(void*,u32,u32))table(o)->original[0])(o,a0,a1);return end(&c,o,r);}
static u32 WIN surface_1(void* o){u32 a[6]={0};struct Context c=begin(o,1,a,0,(u32)__builtin_return_address(0),1);u32 r=((u32(WIN*)(void*))table(o)->original[1])(o);return end(&c,o,r);}
static u32 WIN surface_2(void* o){u32 a[6]={0};struct Context c=begin(o,2,a,0,(u32)__builtin_return_address(0),1);u32 r=((u32(WIN*)(void*))table(o)->original[2])(o);return end(&c,o,r);}
static u32 WIN surface_3(void* o,u32 a0){u32 a[6]={a0};struct Context c=begin(o,3,a,1,(u32)__builtin_return_address(0),1);u32 r=((u32(WIN*)(void*,u32))table(o)->original[3])(o,a0);return end(&c,o,r);}
static u32 WIN surface_4(void* o,u32 a0){u32 a[6]={a0};struct Context c=begin(o,4,a,1,(u32)__builtin_return_address(0),1);u32 r=((u32(WIN*)(void*,u32))table(o)->original[4])(o,a0);return end(&c,o,r);}
static u32 WIN surface_5(void* o,u32 a0,u32 a1,u32 a2,u32 a3,u32 a4){u32 a[6]={a0,a1,a2,a3,a4};struct Context c=begin(o,5,a,5,(u32)__builtin_return_address(0),1);u32 r=((u32(WIN*)(void*,u32,u32,u32,u32,u32))table(o)->original[5])(o,a0,a1,a2,a3,a4);return end(&c,o,r);}
static u32 WIN surface_6(void* o,u32 a0,u32 a1,u32 a2){u32 a[6]={a0,a1,a2};struct Context c=begin(o,6,a,3,(u32)__builtin_return_address(0),1);u32 r=((u32(WIN*)(void*,u32,u32,u32))table(o)->original[6])(o,a0,a1,a2);return end(&c,o,r);}
static u32 WIN surface_7(void* o,u32 a0,u32 a1,u32 a2,u32 a3,u32 a4){u32 a[6]={a0,a1,a2,a3,a4};struct Context c=begin(o,7,a,5,(u32)__builtin_return_address(0),1);u32 r=((u32(WIN*)(void*,u32,u32,u32,u32,u32))table(o)->original[7])(o,a0,a1,a2,a3,a4);return end(&c,o,r);}
static u32 WIN surface_8(void* o,u32 a0,u32 a1){u32 a[6]={a0,a1};struct Context c=begin(o,8,a,2,(u32)__builtin_return_address(0),1);u32 r=((u32(WIN*)(void*,u32,u32))table(o)->original[8])(o,a0,a1);return end(&c,o,r);}
static u32 WIN surface_9(void* o,u32 a0,u32 a1){u32 a[6]={a0,a1};struct Context c=begin(o,9,a,2,(u32)__builtin_return_address(0),1);u32 r=((u32(WIN*)(void*,u32,u32))table(o)->original[9])(o,a0,a1);return end(&c,o,r);}
static u32 WIN surface_10(void* o,u32 a0,u32 a1,u32 a2){u32 a[6]={a0,a1,a2};struct Context c=begin(o,10,a,3,(u32)__builtin_return_address(0),1);u32 r=((u32(WIN*)(void*,u32,u32,u32))table(o)->original[10])(o,a0,a1,a2);return end(&c,o,r);}
static u32 WIN surface_11(void* o,u32 a0,u32 a1){u32 a[6]={a0,a1};struct Context c=begin(o,11,a,2,(u32)__builtin_return_address(0),1);u32 r=((u32(WIN*)(void*,u32,u32))table(o)->original[11])(o,a0,a1);return end(&c,o,r);}
static u32 WIN surface_12(void* o,u32 a0,u32 a1){u32 a[6]={a0,a1};struct Context c=begin(o,12,a,2,(u32)__builtin_return_address(0),1);u32 r=((u32(WIN*)(void*,u32,u32))table(o)->original[12])(o,a0,a1);return end(&c,o,r);}
static u32 WIN surface_13(void* o,u32 a0){u32 a[6]={a0};struct Context c=begin(o,13,a,1,(u32)__builtin_return_address(0),1);u32 r=((u32(WIN*)(void*,u32))table(o)->original[13])(o,a0);return end(&c,o,r);}
static u32 WIN surface_14(void* o,u32 a0){u32 a[6]={a0};struct Context c=begin(o,14,a,1,(u32)__builtin_return_address(0),1);u32 r=((u32(WIN*)(void*,u32))table(o)->original[14])(o,a0);return end(&c,o,r);}
static u32 WIN surface_15(void* o,u32 a0){u32 a[6]={a0};struct Context c=begin(o,15,a,1,(u32)__builtin_return_address(0),1);u32 r=((u32(WIN*)(void*,u32))table(o)->original[15])(o,a0);return end(&c,o,r);}
static u32 WIN surface_16(void* o,u32 a0,u32 a1){u32 a[6]={a0,a1};struct Context c=begin(o,16,a,2,(u32)__builtin_return_address(0),1);u32 r=((u32(WIN*)(void*,u32,u32))table(o)->original[16])(o,a0,a1);return end(&c,o,r);}
static u32 WIN surface_17(void* o,u32 a0){u32 a[6]={a0};struct Context c=begin(o,17,a,1,(u32)__builtin_return_address(0),1);u32 r=((u32(WIN*)(void*,u32))table(o)->original[17])(o,a0);return end(&c,o,r);}
static u32 WIN surface_18(void* o,u32 a0){u32 a[6]={a0};struct Context c=begin(o,18,a,1,(u32)__builtin_return_address(0),1);u32 r=((u32(WIN*)(void*,u32))table(o)->original[18])(o,a0);return end(&c,o,r);}
static u32 WIN surface_19(void* o,u32 a0,u32 a1){u32 a[6]={a0,a1};struct Context c=begin(o,19,a,2,(u32)__builtin_return_address(0),1);u32 r=((u32(WIN*)(void*,u32,u32))table(o)->original[19])(o,a0,a1);return end(&c,o,r);}
static u32 WIN surface_20(void* o,u32 a0){u32 a[6]={a0};struct Context c=begin(o,20,a,1,(u32)__builtin_return_address(0),1);u32 r=((u32(WIN*)(void*,u32))table(o)->original[20])(o,a0);return end(&c,o,r);}
static u32 WIN surface_21(void* o,u32 a0){u32 a[6]={a0};struct Context c=begin(o,21,a,1,(u32)__builtin_return_address(0),1);u32 r=((u32(WIN*)(void*,u32))table(o)->original[21])(o,a0);return end(&c,o,r);}
static u32 WIN surface_22(void* o,u32 a0){u32 a[6]={a0};struct Context c=begin(o,22,a,1,(u32)__builtin_return_address(0),1);u32 r=((u32(WIN*)(void*,u32))table(o)->original[22])(o,a0);return end(&c,o,r);}
static u32 WIN surface_23(void* o,u32 a0,u32 a1){u32 a[6]={a0,a1};struct Context c=begin(o,23,a,2,(u32)__builtin_return_address(0),1);u32 r=((u32(WIN*)(void*,u32,u32))table(o)->original[23])(o,a0,a1);return end(&c,o,r);}
static u32 WIN surface_24(void* o){u32 a[6]={0};struct Context c=begin(o,24,a,0,(u32)__builtin_return_address(0),1);u32 r=((u32(WIN*)(void*))table(o)->original[24])(o);return end(&c,o,r);}
static u32 WIN surface_25(void* o,u32 a0,u32 a1,u32 a2,u32 a3){u32 a[6]={a0,a1,a2,a3};struct Context c=begin(o,25,a,4,(u32)__builtin_return_address(0),1);u32 r=((u32(WIN*)(void*,u32,u32,u32,u32))table(o)->original[25])(o,a0,a1,a2,a3);return end(&c,o,r);}
static u32 WIN surface_26(void* o,u32 a0){u32 a[6]={a0};struct Context c=begin(o,26,a,1,(u32)__builtin_return_address(0),1);u32 r=((u32(WIN*)(void*,u32))table(o)->original[26])(o,a0);return end(&c,o,r);}
static u32 WIN surface_27(void* o){u32 a[6]={0};struct Context c=begin(o,27,a,0,(u32)__builtin_return_address(0),1);u32 r=((u32(WIN*)(void*))table(o)->original[27])(o);return end(&c,o,r);}
static u32 WIN surface_28(void* o,u32 a0){u32 a[6]={a0};struct Context c=begin(o,28,a,1,(u32)__builtin_return_address(0),1);u32 r=((u32(WIN*)(void*,u32))table(o)->original[28])(o,a0);return end(&c,o,r);}
static u32 WIN surface_29(void* o,u32 a0,u32 a1){u32 a[6]={a0,a1};struct Context c=begin(o,29,a,2,(u32)__builtin_return_address(0),1);u32 r=((u32(WIN*)(void*,u32,u32))table(o)->original[29])(o,a0,a1);return end(&c,o,r);}
static u32 WIN surface_30(void* o,u32 a0,u32 a1){u32 a[6]={a0,a1};struct Context c=begin(o,30,a,2,(u32)__builtin_return_address(0),1);u32 r=((u32(WIN*)(void*,u32,u32))table(o)->original[30])(o,a0,a1);return end(&c,o,r);}
static u32 WIN surface_31(void* o,u32 a0){u32 a[6]={a0};struct Context c=begin(o,31,a,1,(u32)__builtin_return_address(0),1);u32 r=((u32(WIN*)(void*,u32))table(o)->original[31])(o,a0);return end(&c,o,r);}
static u32 WIN surface_32(void* o,u32 a0){u32 a[6]={a0};struct Context c=begin(o,32,a,1,(u32)__builtin_return_address(0),1);u32 r=((u32(WIN*)(void*,u32))table(o)->original[32])(o,a0);return end(&c,o,r);}
static u32 WIN surface_33(void* o,u32 a0,u32 a1,u32 a2,u32 a3,u32 a4){u32 a[6]={a0,a1,a2,a3,a4};struct Context c=begin(o,33,a,5,(u32)__builtin_return_address(0),1);u32 r=((u32(WIN*)(void*,u32,u32,u32,u32,u32))table(o)->original[33])(o,a0,a1,a2,a3,a4);return end(&c,o,r);}
static u32 WIN surface_34(void* o,u32 a0){u32 a[6]={a0};struct Context c=begin(o,34,a,1,(u32)__builtin_return_address(0),1);u32 r=((u32(WIN*)(void*,u32))table(o)->original[34])(o,a0);return end(&c,o,r);}
static u32 WIN surface_35(void* o,u32 a0,u32 a1){u32 a[6]={a0,a1};struct Context c=begin(o,35,a,2,(u32)__builtin_return_address(0),1);u32 r=((u32(WIN*)(void*,u32,u32))table(o)->original[35])(o,a0,a1);return end(&c,o,r);}
static void* surface_hooks[36]={(void*)&surface_0,(void*)&surface_1,(void*)&surface_2,(void*)&surface_3,(void*)&surface_4,(void*)&surface_5,(void*)&surface_6,(void*)&surface_7,(void*)&surface_8,(void*)&surface_9,(void*)&surface_10,(void*)&surface_11,(void*)&surface_12,(void*)&surface_13,(void*)&surface_14,(void*)&surface_15,(void*)&surface_16,(void*)&surface_17,(void*)&surface_18,(void*)&surface_19,(void*)&surface_20,(void*)&surface_21,(void*)&surface_22,(void*)&surface_23,(void*)&surface_24,(void*)&surface_25,(void*)&surface_26,(void*)&surface_27,(void*)&surface_28,(void*)&surface_29,(void*)&surface_30,(void*)&surface_31,(void*)&surface_32,(void*)&surface_33,(void*)&surface_34,(void*)&surface_35};
struct ForwardState {u8 fp[512] __attribute__((aligned(16)));u32 error;};
static void forward_saved(struct ForwardState* s){s->error=GetLastError();snapshot_fp(s->fp);}
static void forward_restore(struct ForwardState* s){restore_fp(s->fp);SetLastError(s->error);}
static u32 WIN query(void* o,u32 guid,u32 output){u32 a[6]={guid,output};struct Context c=begin(o,0,a,2,(u32)__builtin_return_address(0),5);u32 r=((u32(WIN*)(void*,u32,u32))table(o)->original[0])(o,guid,output);u32 error=GetLastError();struct ForwardState saved __attribute__((cleanup(forward_restore)));forward_saved(&saved);if((i32)r>=0&&readable((void*)output,4)){u32 k=kind((void*)guid);EnterCriticalSection(journal_lock);acquired(*(void**)output,k,(void*)guid);c.r[4]=identity(*(void**)output);LeaveCriticalSection(journal_lock);}SetLastError(error);return end(&c,o,r);}
static u32 WIN create_surface(void* o,u32 desc,u32 output,u32 outer){u32 a[6]={0};struct Context c=begin(o,6,a,0,(u32)__builtin_return_address(0),5);u32 r=((u32(WIN*)(void*,u32,u32,u32))table(o)->original[6])(o,desc,output,outer);u32 error=GetLastError();struct ForwardState saved __attribute__((cleanup(forward_restore)));forward_saved(&saved);if((i32)r>=0&&readable((void*)output,4)){EnterCriticalSection(journal_lock);install(*(void**)output,table(o)->kind==7?17:table(o)->kind==4?14:11);c.r[4]=identity(*(void**)output);LeaveCriticalSection(journal_lock);}SetLastError(error);return end(&c,o,r);}
static u32 WIN mode1(void* o,u32 w,u32 h,u32 b){u32 a[6]={w,h,b};struct Context c=begin(o,21,a,3,(u32)__builtin_return_address(0),5);u32 r=((u32(WIN*)(void*,u32,u32,u32))table(o)->original[21])(o,w,h,b);return end(&c,o,r);}
static u32 WIN mode2(void* o,u32 w,u32 h,u32 b,u32 rate,u32 flags){u32 a[6]={w,h,b,rate,flags};struct Context c=begin(o,21,a,5,(u32)__builtin_return_address(0),5);u32 r=((u32(WIN*)(void*,u32,u32,u32,u32,u32))table(o)->original[21])(o,w,h,b,rate,flags);return end(&c,o,r);}
static void install_body(void* object,u32 k){
 if(!object||!readable(object,4))return;void** vt=*(void***)object;u32 n=k>=11?36:23;
 for(u32 i=0;i<table_count;++i)if(tables[i].vt==vt){if(tables[i].kind!=k)++gaps;return;}
 if(table_count>=64||!readable(vt,n*4)){++gaps;return;}u32 old,ignored;if(!VirtualProtect(vt,n*4,4,&old)){++gaps;return;}
 struct Table* t=tables+table_count;t->vt=vt;t->kind=k;copy(t->original,vt,n*4);++table_count;
 if(k>=11){for(u32 i=0;i<36;++i)vt[i]=surface_hooks[i];}
 else{vt[0]=(void*)&query;vt[6]=(void*)&create_surface;vt[21]=k==1?(void*)&mode1:(void*)&mode2;}
 if(!VirtualProtect(vt,n*4,old,&ignored))++gaps;
}
static void install(void* object,u32 k){u32 error=GetLastError();u8 fp[512] __attribute__((aligned(16)));snapshot_fp(fp);EnterCriticalSection(journal_lock);install_body(object,k);LeaveCriticalSection(journal_lock);restore_fp(fp);SetLastError(error);}
static u32 WIN create(void* guid,void** result,void* outer){u32 error=GetLastError();u8 install_fp[512] __attribute__((aligned(16)));snapshot_fp(install_fp);if(!wine_submission_install(GetModuleHandleA("ddraw.dll")))ExitProcess(99);restore_fp(install_fp);SetLastError(error);u32 r=((u32(WIN*)(void*,void**,void*))original_create)(guid,result,outer);error=GetLastError();struct ForwardState saved __attribute__((cleanup(forward_restore)));forward_saved(&saved);if((i32)r>=0&&readable(result,4)&&*result)install(*result,1);SetLastError(error);return r;}
static u32 WIN create_ex(void* guid,void** result,void* iid,void* outer){u32 saved_error=GetLastError();u8 install_fp[512] __attribute__((aligned(16)));snapshot_fp(install_fp);if(!wine_submission_install(GetModuleHandleA("ddraw.dll")))ExitProcess(99);restore_fp(install_fp);SetLastError(saved_error);u32 r=((u32(WIN*)(void*,void**,void*,void*))original_create_ex)(guid,result,iid,outer);u32 error=GetLastError();struct ForwardState saved __attribute__((cleanup(forward_restore)));forward_saved(&saved);if((i32)r>=0&&readable(result,4)&&*result){u32 k=kind(iid);if(k)install(*result,k);else ++gaps;}SetLastError(error);return r;}
static void* WIN proc(void* module,const char* name){void* result=((void*(WIN*)(void*,const char*))original_proc)(module,name);u32 error=GetLastError();struct ForwardState saved __attribute__((cleanup(forward_restore)));forward_saved(&saved);if(readable(name,17)&&equal(name,"DirectDrawCreate\0",17)){original_create=result;result=(void*)&create;}else if(readable(name,19)&&equal(name,"DirectDrawCreateEx\0",19)){original_create_ex=result;result=(void*)&create_ex;}SetLastError(error);return result;}
static short WIN async(i32 code){u32 a[6]={(u32)code};struct Context c=begin(0,1,a,1,(u32)__builtin_return_address(0),2);short r=((short(WIN*)(i32))original_async)(code);end(&c,0,(u32)(i32)r);return r;}
static short WIN key(i32 code){u32 a[6]={(u32)code};struct Context c=begin(0,2,a,1,(u32)__builtin_return_address(0),2);short r=((short(WIN*)(i32))original_key)(code);end(&c,0,(u32)(i32)r);return r;}
static i32 WIN cursor(i32* point){u32 a[6]={(u32)point};struct Context c=begin(0,3,a,1,(u32)__builtin_return_address(0),2);c.r[21]=0;i32 r=((i32(WIN*)(i32*))original_cursor)(point);end(&c,0,r);return r;}
static u32 WIN map(u32 code,u32 mode){u32 a[6]={code,mode};struct Context c=begin(0,4,a,2,(u32)__builtin_return_address(0),2);u32 r=((u32(WIN*)(u32,u32))original_map)(code,mode);end(&c,0,r);return r;}
void WIN DrawingActionV1(u32 request,u32 action,u32 screen,u32 generation,u32 argument,u32 phase,u32 status){u32 a[6]={0};struct Context c=begin(0,phase,a,0,0,3);c.r[21]=request;c.r[22]=action;c.r[23]=screen;c.r[24]=generation;c.r[25]=argument;c.r[26]=phase;c.r[27]=status;end(&c,0,status);}
void WIN DrawingAnchor(void){}
#ifdef MNM_DRAWING_SELFTEST
u32 WIN DrawingInstallForTest(void* object,u32 k){install(object,k);return enabled;}
#endif
static int initialize(void){
 InitializeCriticalSection(journal_lock);
 char mode[16];u32 mode_length=GetEnvironmentVariableA("MNM_DRAW_SESSION_MODE",mode,sizeof(mode));int inputs_only=mode_length==6&&equal(mode,"inputs",6);(void)inputs_only;physical_enabled=mode_length==8&&equal(mode,"physical",8);readbacks_enabled=!(mode_length==8&&equal(mode,"metadata",8));
 if(!physical_budget())return 0;
 u32 n=GetEnvironmentVariableA("MNM_DRAW_SESSION_DIRECTORY",directory,sizeof(directory));if(!n)return 1;if(n>=sizeof(directory))return 0;
 char name[600];path(name,"session.bin");journal=CreateFileA(name,0x40000000,3,0,1,0x80,0);if(journal==(HANDLE)-1){journal=0;return 0;}u32 header[16]={0};copy(header,"MNMDAPI1",8);header[2]=1;header[3]=128;header[4]=0x40209ca7;header[5]=262144;header[6]=1024;header[7]=physical_enabled?physical_byte_limit:1024u*1024u*1024u;header[8]=physical_enabled?3:inputs_only?2:readbacks_enabled?0:1;if(!write(journal,header,64)||(physical_enabled&&!interval_open())||!producer_points_open()||!wine_submission_open())return 0;enabled=1;
#ifndef MNM_DRAWING_SELFTEST
 u32 base=(u32)GetModuleHandleA(0);const u8 thunk[]={0xff,0x25,0x14,0x50,0x5c,0};if(base!=0x400000||!readable((void*)0x59755a,6)||!equal((void*)0x59755a,thunk,6))return 0;
 void* user=GetModuleHandleA("user32.dll");original_async=GetProcAddress(user,"GetAsyncKeyState");original_key=GetProcAddress(user,"GetKeyState");original_cursor=GetProcAddress(user,"GetCursorPos");original_map=GetProcAddress(user,"MapVirtualKeyA");
 void** dd=(void**)0x5c5010;void** input=(void**)0x5c51f8;u32 old,ignored;
 if(!readable(dd,0xc8)||!readable(input,43*4)||input[3]!=original_async||input[6]!=original_cursor||input[38]!=original_key||input[37]!=original_map)return 0;
 if(!inputs_only){if(!VirtualProtect(dd,0xc8,4,&old))return 0;original_create=dd[1];original_proc=dd[0x31];dd[1]=(void*)&create;dd[0x31]=(void*)&proc;VirtualProtect(dd,0xc8,old,&ignored);}
 if(!VirtualProtect(input,43*4,4,&old))return 0;input[3]=(void*)&async;input[6]=(void*)&cursor;input[38]=(void*)&key;input[37]=(void*)&map;VirtualProtect(input,43*4,old,&ignored);
#endif
 return 1;
}
int WIN DllMain(void* module,u32 reason,void* reserved){(void)module;(void)reserved;if(reason==1)return initialize();if(reason==0)finish(1);return 1;}
