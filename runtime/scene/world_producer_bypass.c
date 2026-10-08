/* Included by canvas_producers.c so synthetic forwarding exercises the same
 * entry callback and freestanding imports. Native output is the only writeback. */
#include "../../protocols/include/mnm/world_producer_bypass_v1.h"
API int WIN ReadFile(HANDLE,void*,u32,u32*,void*);
API u32 WIN GetFileSize(HANDLE,u32*);
API void WIN Sleep(u32);
API u32 WIN GetTickCount(void);
static u32 bypass_limit,bypass_queue,bypass_completed;
static u32 raster_first,raster_last,raster_preparations,raster_world_open;
#include "world_raster_callers.h"
static int raster_active(void){return raster_world_open&&raster_first&&queue>=raster_first&&queue<=raster_last;}
static u32 raster_number(const char* name){char value[8];u32 n=GetEnvironmentVariableA(name,value,8),v=0;if(!n)return 0;if(n>=8)return 99;for(u32 i=0;i<n;++i){if(value[i]<'0'||value[i]>'9')return 99;v=v*10+value[i]-'0';}return v;}
static int raster_init(u32 queues){u32 selected=raster_number("MNM_WORLD_RASTER_QUEUE"),prefix=raster_number("MNM_WORLD_RASTER_PREFIX");if(selected&&prefix)return 0;if(selected>queues||prefix>queues)return 0;raster_first=selected?selected:prefix?1:0;raster_last=selected?selected:prefix;return 1;}
static void bypass_path(char* out,u32 ordinal,const char* suffix){if(!raster_active()){path(out,"world-bypass-",ordinal,suffix);return;}u32 i=0;while(directory[i]){out[i]=directory[i];++i;}out[i++]='\\';const char* stem="world-raster-";while(*stem)out[i++]=*stem++;for(u32 d=100000;d;d/=10)out[i++]=(char)('0'+ordinal/d%10);while(*suffix)out[i++]=*suffix++;out[i]=0;}
static int bypass_init(u32 queues){
 char value[8];u32 n=GetEnvironmentVariableA("MNM_WORLD_PRODUCER_BYPASS",value,8);
 if(!n)return 1;if(n>=8)return 0;
 for(u32 i=0;i<n;++i){if(value[i]<'0'||value[i]>'9')return 0;bypass_limit=bypass_limit*10+value[i]-'0';}
 if(!bypass_limit||bypass_limit>MNM_WORLD_BYPASS_MAX)return 0;bypass_queue=queues;return 1;
}
static int bypass_equal(const void* a,const void* b,u32 n){const u8* x=a;const u8* y=b;while(n--)if(*x++!=*y++)return 0;return 1;}
static int raster_caller(u32 caller,u32 tag){
#ifdef MNM_SCENE_SELFTEST
 (void)caller;(void)tag;return 1;
#else
 for(u32 i=0;i<sizeof(raster_callers)/sizeof(raster_callers[0]);++i)if(caller==raster_callers[i].address&&(raster_callers[i].tags&(1u<<tag))){if(!readable(caller,10))return 0;return bypass_equal((void*)caller,raster_callers[i].bytes,10);}return 0;
#endif
}

static u32 bypass_hash(const u8* b,u32 n){u32 h=2166136261u;while(n--)h=(h^*b++)*16777619u;return h;}
static int bypass_read(HANDLE f,void* destination,u32 size){u8* p=destination;u32 at=0;while(at<size){u32 n=0;if(!ReadFile(f,p+at,size-at,&n,0)||!n||n>size-at)return 0;at+=n;}return 1;}
static int bypass_writable(u32 p,u32 n){u32 end=p+n;if(!p||end<p)return 0;while(p<end){u32 m[7];if(VirtualQuery((void*)p,m,28)!=28||m[4]!=0x1000||(m[5]&0x101)||!(m[5]&0xcc))return 0;u32 next=m[0]+m[3];if(next<=p)return 0;p=next<end?next:end;}return 1;}
static void bypass_header(u32* request,u32* r,u32 entry){
 zero(request,64);copy(request,raster_active()?MNM_WORLD_RASTER_REQUEST:MNM_WORLD_BYPASS_REQUEST,8);request[2]=raster_active()?2:1;request[3]=64;request[4]=bypass_completed+1;request[5]=queue;request[6]=r[1];request[7]=r[3];request[8]=r[5];request[9]=r[6];request[10]=r[5];request[14]=entry;request[15]=raster_active()?r[21]:0;
}
static int bypass_notify(u32* r,u32 entry){
 u32 request[16];bypass_header(request,r,entry);char name[260];bypass_path(name,bypass_completed+1,".request");HANDLE file=CreateFileA(name,0x40000000,0,0,1,0x80,0);if(file==(HANDLE)-1)return 0;int okay=write(file,request,64);return CloseHandle(file)&&okay;
}
static int bypass_reply(u32* r,struct Surface* surface,u32 entry){
 const u32 ordinal=bypass_completed+1,width=r[5],height=r[6],n=width*height*2;
 if(!width||width>2048||!height||height>2048||surface->stride<width||surface->stride>4096||!bypass_writable(surface->pixels,surface->stride*height*2))return 0;
 u32 request[16];bypass_header(request,r,entry);
 char name[260];HANDLE f;int okay;
 const u32 started=GetTickCount();bypass_path(name,ordinal,".reply");u8* pixels=HeapAlloc(GetProcessHeap(),0,n);if(!pixels)return 0;
 while((u32)(GetTickCount()-started)<MNM_WORLD_BYPASS_TIMEOUT){
  f=CreateFileA(name,0x80000000,1,0,3,0x80,0);
  if(f==(HANDLE)-1){Sleep(1);continue;}
  u32 header[16]={0},high=0,size=GetFileSize(f,&high);
  okay=!high&&size==64+n&&bypass_read(f,header,64);
  if(okay){for(u32 i=4;i<=10;++i)if(header[i]!=request[i])okay=0;
   if(!bypass_equal(header,raster_active()?MNM_WORLD_RASTER_REPLY:MNM_WORLD_BYPASS_REPLY,8)||header[2]!=request[2]||header[3]!=64||header[11]!=n||header[13]!=1||header[14]!=entry||header[15]!=request[15])okay=0;
  }
  if(okay)okay=bypass_read(f,pixels,n)&&bypass_hash(pixels,n)==header[12];CloseHandle(f);
  if(okay){for(u32 y=0;y<height;++y)copy((void*)(surface->pixels+y*surface->stride*2),pixels+y*width*2,width*2);++bypass_completed;}
  HeapFree(GetProcessHeap(),0,pixels);return okay;
 }
 HeapFree(GetProcessHeap(),0,pixels);return 0;
}
