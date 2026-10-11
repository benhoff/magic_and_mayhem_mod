/* Opt-in call metadata only. Original arguments, results and work are forwarded. */
#include "../shadow/win32_min.h"
#include "../../protocols/include/mnm/drawing_census_v1.h"
API u32 WIN GetCurrentThreadId(void);
API u32 WIN GetTickCount(void);
API void WIN Sleep(u32);
API HANDLE WIN CreateFileMappingA(HANDLE,void*,u32,u32,u32,const char*);
API void* WIN MapViewOfFile(HANDLE,u32,u32,u32,u32);
API int WIN UnmapViewOfFile(const void*);
__declspec(dllexport) void census_leave(void);
#include "hooks.inc"
u32 census_trampolines[CENSUS_HOOKS];
static struct Frame {u32 sp,thread,caller,tag,id,phase;} frames[2048];
static HANDLE journal;
static const volatile u32* control;
static volatile u32 mutex;
static u32 sequence,serial,lost,active,io_failed,used;
static u8 buffer[65536];
static void copy(void* d,const void* s,u32 n){u8* a=d;const u8* b=s;while(n--)*a++=*b++;}
static void zero(void* d,u32 n){u8* a=d;while(n--)*a++=0;}
static int same(const void* a,const void* b,u32 n){const u8* x=a;const u8* y=b;while(n--)if(*x++!=*y++)return 0;return 1;}
static int readable(u32 p,u32 n){u32 end=p+n;if(!p||end<p)return 0;while(p<end){u32 m[7];if(VirtualQuery((void*)p,m,28)!=28||m[4]!=0x1000||(m[5]&0x101)||!(m[5]&0xee))return 0;u32 next=m[0]+m[3];if(next<=p)return 0;p=next<end?next:end;}return 1;}
static void lock(void){while(__sync_lock_test_and_set(&mutex,1))Sleep(0);}
static void unlock(void){__sync_lock_release(&mutex);}
static int write(const void* p,u32 n){u32 at=0;while(at<n){u32 done=0;if(!WriteFile(journal,(const u8*)p+at,n-at,&done,0)||!done||done>n-at){io_failed=1;return 0;}at+=done;}return 1;}
static void flush(void){if(used&&!io_failed)write(buffer,used);used=0;}
static u32 phase(void){
 /* A host phase change is a seqlock transaction, not a missing call. Retry
  * boundedly; a persistently invalid/unavailable host still records loss. */
 for(u32 attempt=0;attempt<64;++attempt){
  u32 before=control[4],result=control[5],alive=control[6];
  __atomic_thread_fence(__ATOMIC_ACQUIRE);
  if(!(before&1)&&before==control[4]){
   if(result<=14&&alive==1)return result;
   break;
  }
  if((attempt&15)==15)Sleep(0);
 }
 ++lost;return 0;
}
static void emit(u32 type,const struct Frame* f,u32* regs,u32 error){
 if(io_failed||sequence>=MNM_DRAW_CENSUS_MAX_RECORDS){++lost;return;}
 u32 row[32]={0};row[0]=++sequence;row[1]=type;row[2]=f->id;row[3]=f->tag;row[4]=f->phase;row[5]=f->thread;row[6]=hooks[f->tag].address;row[7]=f->caller;row[8]=f->sp;row[9]=regs[6];row[10]=regs[5];row[11]=regs[7];row[12]=regs[8];row[13]=error;row[14]=GetTickCount();
 if(type!=MNM_DRAW_CENSUS_RETURN&&readable((u32)(regs+10),24))copy(row+15,regs+10,24);
 row[21]=*(u32*)0x5e1404;row[22]=*(u32*)0x6cbb6c;row[23]=*(u32*)0x6e1f88;row[24]=*(u32*)0x6f349c;
 if(type==MNM_DRAW_CENSUS_ENTER&&hooks[f->tag].address==0x5002a0&&readable(regs[6],24)){u32* q=(u32*)regs[6];row[25]=q[2];row[26]=q[3];row[27]=q[5];}
 row[28]=active;
 if(used+128>sizeof(buffer))flush();if(!io_failed){copy(buffer+used,row,128);used+=128;}
}
void census_enter_observe(u32* regs,u32 tag){
 u32 error=GetLastError(),thread=GetCurrentThreadId();lock();
 struct Frame f={(u32)(regs+9)+4+hooks[tag].pop,thread,regs[9],tag,++serial,phase()};
 if(hooks[tag].site){f.caller=0;emit(MNM_DRAW_CENSUS_SITE,&f,regs,error);unlock();SetLastError(error);return;}
 for(u32 i=0;i<2048;++i)if(!frames[i].sp){frames[i]=f;++active;emit(MNM_DRAW_CENSUS_ENTER,&f,regs,error);regs[9]=(u32)census_leave;unlock();SetLastError(error);return;}
 ++lost;unlock();SetLastError(error);
}
void census_leave_observe(u32* regs){
 u32 error=GetLastError(),thread=GetCurrentThreadId(),sp=(u32)regs+40;lock();
 u32 match=2048;for(u32 i=0;i<2048;++i)if(frames[i].sp==sp&&frames[i].thread==thread&&(match==2048||frames[i].id>frames[match].id))match=i;
 if(match!=2048){struct Frame f=frames[match];zero(frames+match,sizeof(frames[match]));--active;regs[9]=f.caller;emit(MNM_DRAW_CENSUS_RETURN,&f,regs,error);unlock();SetLastError(error);return;}
 /* There is no safe guessed return address after bookkeeping corruption. */
 ++lost;flush();unlock();ExitProcess(94);
}
static int path(char* out,const char* dir,const char* suffix){u32 at=0;while(dir[at]){if(at>=220)return 0;out[at]=dir[at];++at;}while(*suffix){if(at>=259)return 0;out[at++]=*suffix++;}out[at]=0;return 1;}
static int install(void){
 char directory[220],name[260];u32 n=GetEnvironmentVariableA("MNM_DRAW_CENSUS_DIR",directory,sizeof(directory));if(!n||n>=sizeof(directory)||(u32)GetModuleHandleA(0)!=0x400000)return 0;
 u32 a,b,c,d;__asm__ volatile("cpuid":"=a"(a),"=b"(b),"=c"(c),"=d"(d):"a"(1));if(!(d&(1u<<24)))return 0;
 if(!path(name,directory,"\\census-control.bin"))return 0;
 HANDLE file=CreateFileA(name,0x80000000,3,0,3,0x80,0);if(file==(HANDLE)-1)return 0;
 HANDLE mapping=CreateFileMappingA(file,0,2,0,64,0);CloseHandle(file);if(!mapping)return 0;
 control=MapViewOfFile(mapping,4,0,0,64);CloseHandle(mapping);if(!control||!same((const void*)control,MNM_DRAW_CENSUS_CONTROL,8)||control[2]!=1||control[3]!=64)return 0;
 for(u32 i=0;i<CENSUS_HOOKS;++i){const struct Hook* h=hooks+i;if(!readable(h->address,h->length)||!same((void*)h->address,h->bytes,h->length))return 0;
  u8* t=VirtualAlloc(0,h->length+5,0x3000,0x40);if(!t)return 0;copy(t,h->bytes,h->length);
  for(u32 j=0;j<h->relocs;++j){u32 at=h->reloc[j];*(u32*)(t+at+1)=h->address+at+5+*(i32*)(h->bytes+at+1)-(u32)t-at-5;}
  t[h->length]=0xe9;*(u32*)(t+h->length+1)=h->address+h->length-(u32)t-h->length-5;census_trampolines[i]=(u32)t;
  if(!FlushInstructionCache(GetCurrentProcess(),t,h->length+5))return 0;
 }
 if(!path(name,directory,"\\census.bin"))return 0;journal=CreateFileA(name,0x40000000,1,0,1,0x80,0);if(journal==(HANDLE)-1){journal=0;return 0;}
 u32 header[16]={0};copy(header,MNM_DRAW_CENSUS_MAGIC,8);header[2]=1;header[3]=64;header[4]=128;header[5]=(u32)GetModuleHandleA(0);header[6]=CENSUS_HOOKS;header[7]=MNM_DRAW_CENSUS_MAX_RECORDS;copy(header+8,original_sha256,32);if(!write(header,64))return 0;
 for(u32 i=0;i<CENSUS_HOOKS;++i){const struct Hook* h=hooks+i;u32 old,unused;if(!VirtualProtect((void*)h->address,h->length,0x40,&old))return 0;u8 bytes[16];zero(bytes,16);bytes[0]=0xe9;*(u32*)(bytes+1)=(u32)h->entry-h->address-5;for(u32 j=5;j<h->length;++j)bytes[j]=0x90;copy((void*)h->address,bytes,h->length);int flushed=FlushInstructionCache(GetCurrentProcess(),(void*)h->address,h->length);int protected=VirtualProtect((void*)h->address,h->length,old,&unused);if(!flushed||!protected)return 0;}
 return 1;
}
__declspec(dllexport) void DrawingCensusAnchor(void){}
int WIN DllMain(void* instance,u32 reason,void* reserved){
 (void)instance;(void)reserved;if(reason==1)return install();if(reason!=0)return 1;
 if(journal){/* Process detach cannot wait on a terminated worker. Busy means no valid terminal. */
 if(__sync_lock_test_and_set(&mutex,1)){CloseHandle(journal);journal=0;return 1;}flush();u32 end[16]={0};copy(end,MNM_DRAW_CENSUS_END,8);end[2]=1;end[3]=sequence;end[4]=serial;end[5]=lost;end[6]=active;end[7]=io_failed;if(!io_failed)write(end,64);CloseHandle(journal);journal=0;unlock();}if(control){UnmapViewOfFile((const void*)control);control=0;}return 1;
}
