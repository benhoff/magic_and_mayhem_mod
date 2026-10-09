/* Included by world_producer_bypass.c. Opt-in owned queue completion with a
 * mandatory inaccessible original destination until the World return boundary. */
#include "../../protocols/include/mnm/world_raster_batch_v3.h"
API void* WIN AddVectoredExceptionHandler(u32,void*);
API HANDLE WIN HeapCreate(u32,u32,u32);
static u32 batch_enabled,batch_count,batch_transfers,batch_pixel_base,batch_pixel_bytes;
static u32 batch_guard_base,batch_guard_bytes,batch_guard_old,batch_guard_open;
static u32 batch_reject;
static u32 batch_rows[MNM_WORLD_BATCH_MAX][4];
static struct Surface* batch_surface;
struct BatchExceptionRecord {u32 code,flags,record,address,count,information[15];};
struct BatchExceptionPointers {struct BatchExceptionRecord* record;void* context;};
static i32 WIN batch_fault(struct BatchExceptionPointers* pointers){
 struct BatchExceptionRecord* e=pointers?pointers->record:0;
 if(!e||e->code!=0xc0000005u||e->count<2||!batch_guard_open||e->information[1]<batch_guard_base||e->information[1]-batch_guard_base>=batch_guard_bytes)return 0;
 u32 r[8]={0};copy(r,"MNMBFLT1",8);r[2]=e->address;r[3]=e->information[1];r[4]=e->information[0];r[5]=queue;r[6]=batch_count;r[7]=batch_guard_bytes;
 char name[260];path(name,"world-batch-fault-",queue,".bin");HANDLE f=CreateFileA(name,0x40000000,0,0,1,0x80,0);if(f!=(HANDLE)-1){write(f,r,32);CloseHandle(f);}fail(44,e->address);ExitProcess(96);return 0;
}
static int batch_init(void){
 char value[2];u32 n=GetEnvironmentVariableA("MNM_WORLD_RASTER_BATCH",value,2);
 if(!n)return 1;if(n!=1||value[0]!='1'||!raster_first||bypass_limit)return 0;
 producer_record_heap=HeapCreate(0,0,0);if(!producer_record_heap)return 0;
 batch_enabled=1;return AddVectoredExceptionHandler(1,(void*)batch_fault)!=0;
}
static int batch_start(struct Surface* s){
 batch_reject=1;if(!s||batch_guard_open||s->width!=800||s->height!=600||s->stride!=800)return 0;
 batch_reject=2;if(get((void*)0x6e1f88))return 0;
 const u32 n=s->stride*s->height*2,p=s->pixels;batch_reject=3;if(!readable(p,n))return 0;
 u32 m[7],end[7];batch_reject=4;if(VirtualQuery((void*)p,m,28)!=28||VirtualQuery((void*)(p+n-1),end,28)!=28||m[1]!=end[1]||m[5]!=end[5]||(m[5]!=4&&m[5]!=8))return 0;
 batch_guard_base=p&~4095u;batch_guard_bytes=((p+n+4095u)&~4095u)-batch_guard_base;
 batch_reject=5;if(batch_guard_base<m[0]||batch_guard_bytes>m[3]-(batch_guard_base-m[0]))return 0;
 batch_count=0;batch_surface=s;batch_pixel_base=p;batch_pixel_bytes=n;
 batch_reject=6;if(!VirtualProtect((void*)batch_guard_base,batch_guard_bytes,1,&batch_guard_old))return 0;
 batch_guard_open=1;batch_reject=0;return 1;
}
static int batch_identity(struct Surface* s){return s&&s==batch_surface&&s->pixels==batch_pixel_base&&s->width==800&&s->height==600&&s->stride==800;}
static int batch_accept(u32* r,struct Surface* s,u32 entry){
 if(!batch_guard_open||!batch_identity(s)||batch_count>=MNM_WORLD_BATCH_MAX||r[21]>1||r[3]!=s->id)return 0;
 u32* row=batch_rows[batch_count++];row[0]=r[1];row[1]=entry;row[2]=r[21];row[3]=bypass_hash((u8*)r,r[0]);++bypass_completed;return 1;
}
static void batch_header(u32* h,struct Surface* s){
 zero(h,64);copy(h,MNM_WORLD_BATCH_REQUEST,8);h[2]=3;h[3]=64;h[4]=batch_transfers+1;h[5]=queue;h[6]=sequence;h[7]=s->id;h[8]=s->width;h[9]=s->height;h[10]=s->width;h[11]=batch_count*16;h[12]=bypass_hash((u8*)batch_rows,h[11]);h[13]=1;h[14]=batch_count;h[15]=bypass_completed;
}
static int batch_restore(void){u32 prior;if(!batch_guard_open)return 0;
 u32 at=batch_guard_base,end=at+batch_guard_bytes;while(at<end){u32 m[7];if(VirtualQuery((void*)at,m,28)!=28||m[4]!=0x1000||m[5]!=1||m[0]+m[3]<=at)return 0;at=m[0]+m[3];}
 if(!VirtualProtect((void*)batch_guard_base,batch_guard_bytes,batch_guard_old,&prior)||prior!=1)return 0;batch_guard_open=0;return 1;}
static int batch_reply(struct Surface* s,const u32* request){
 if(!batch_identity(s)||!bypass_writable(s->pixels,batch_pixel_bytes))return 0;
 char name[260];path(name,"world-batch-",queue,".reply");u8* pixels=HeapAlloc(GetProcessHeap(),0,batch_pixel_bytes);if(!pixels)return 0;
 const u32 started=GetTickCount();int okay=0;
 while((u32)(GetTickCount()-started)<MNM_WORLD_BYPASS_TIMEOUT){
  HANDLE f=CreateFileA(name,0x80000000,1,0,3,0x80,0);if(f==(HANDLE)-1){Sleep(1);continue;}
  u32 h[16]={0},high=0;okay=GetFileSize(f,&high)==64+batch_pixel_bytes&&!high&&bypass_read(f,h,64);
  if(okay){for(u32 i=4;i<=10;++i)if(h[i]!=request[i])okay=0;for(u32 i=14;i<=15;++i)if(h[i]!=request[i])okay=0;
   if(!bypass_equal(h,MNM_WORLD_BATCH_REPLY,8)||h[2]!=3||h[3]!=64||h[11]!=batch_pixel_bytes||h[13]!=1)okay=0;}
  if(okay)okay=bypass_read(f,pixels,batch_pixel_bytes)&&bypass_hash(pixels,batch_pixel_bytes)==h[12];CloseHandle(f);
  if(okay){copy((void*)s->pixels,pixels,batch_pixel_bytes);++batch_transfers;}break;
 }
 HeapFree(GetProcessHeap(),0,pixels);return okay;
}
static int batch_finish(struct Surface* s){
 if(!batch_guard_open||!batch_count||!batch_identity(s))return 0;
 u32 h[16];batch_header(h,s);if(!batch_restore())return 0;
 char name[260];path(name,"world-batch-",queue,".request");HANDLE f=CreateFileA(name,0x40000000,0,0,1,0x80,0);
 if(f==(HANDLE)-1)return 0;int okay=write(f,h,64)&&write(f,batch_rows,h[11]);if(!CloseHandle(f))okay=0;
 return okay&&batch_reply(s,h);
}
