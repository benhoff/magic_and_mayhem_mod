/* Separate opt-in PE32 observation: original IAT calls run once, no added wait. */
#include "../shadow/win32_min.h"
API void* WIN GetProcAddress(void*,const char*);
API u32 WIN GetCurrentThreadId(void);
API u32 WIN GetTickCount(void);
API void WIN InitializeCriticalSection(void*);
API void WIN EnterCriticalSection(void*);
API void WIN LeaveCriticalSection(void*);
void* memset(void* out,int value,u32 size){u8* p=out;while(size--)*p++=(u8)value;return out;}
static u32 mutex_lock[6],mutex_count,mutex_calls,mutex_dropped,mutex_pending;
static u32 mutex_rows[65536][32];
static HANDLE mutex_file;
typedef HANDLE(WIN *CreateFn)(void*,u32,const char*);
typedef u32(WIN *WaitFn)(HANDLE,u32);
typedef u32(WIN *HandleFn)(HANDLE);
static CreateFn original_create;
static WaitFn original_wait;
static HandleFn original_release,original_close;
typedef u32(WIN *PointFn)(u32,void*,u32);
static PointFn original_point;
struct MutexIdentity{HANDLE handle;u32 id,live;};
static struct MutexIdentity mutex_ids[64];
static u32 next_mutex_id,identity_overflow;
static void mutex_copy(void* a,const void* b,u32 n){u8* x=a;const u8* y=b;while(n--)*x++=*y++;}
static u32 mutex_get(const void* p){const u8* b=p;return b[0]|((u32)b[1]<<8)|((u32)b[2]<<16)|((u32)b[3]<<24);}
static int mutex_readable(const void* pointer,u32 size){u32 at=(u32)pointer,end=at+size;if(!at||end<at)return 0;while(at<end){u32 m[7];if(VirtualQuery((void*)at,m,28)!=28||m[4]!=0x1000||(m[5]&0x101))return 0;u32 next=m[0]+m[3];if(next<=at)return 0;at=next;}return 1;}
static void mutex_fp_save(void* out){__asm__ volatile("fxsave (%0)"::"r"(out):"memory");}
static void mutex_fp_restore(void* in){__asm__ volatile("fxrstor (%0)"::"r"(in):"memory");}
static u32 mutex_identity(HANDLE handle,int create){
 if(!handle)return 0;
 for(u32 i=0;i<64;++i)if(mutex_ids[i].live&&mutex_ids[i].handle==handle)return mutex_ids[i].id;
 if(create)for(u32 i=0;i<64;++i)if(!mutex_ids[i].live){mutex_ids[i].handle=handle;mutex_ids[i].live=1;return mutex_ids[i].id=++next_mutex_id;}
 if(create)identity_overflow=1;return 0;
}
static void mutex_point(u32* row){
 // Query outside the recorder lock. The point export performs no input poll.
 if(original_point&&original_point(GetCurrentThreadId(),row+16,64))row[15]=1;else row[15]=2;
}
static void mutex_emit(u32* row){
 if(mutex_count>=65536){++mutex_dropped;return;}
 row[0]=++mutex_count;mutex_copy(mutex_rows+mutex_count-1,row,128);
}
static u32 mutex_begin(u32* row,u32 operation,HANDLE handle,u32 parameter,u32 caller,u32 error){
 row[2]=operation;row[3]=1;row[4]=GetCurrentThreadId();row[5]=GetTickCount();row[6]=caller>=0x400000&&caller<0x5c0000?caller-0x400000:0;row[8]=parameter;row[10]=error;
 mutex_point(row);EnterCriticalSection(mutex_lock);row[1]=++mutex_calls;row[7]=mutex_identity(handle,0);++mutex_pending;mutex_emit(row);LeaveCriticalSection(mutex_lock);return row[1];
}
static void mutex_end(u32* row,HANDLE handle,u32 result,u32 error){
 row[3]=2;row[5]=GetTickCount();row[9]=result;row[10]=error;
 for(u32 i=15;i<32;++i)row[i]=0;mutex_point(row);
 EnterCriticalSection(mutex_lock);
 if(row[2]==1&&handle)row[7]=mutex_identity(handle,1);
 if(row[2]==4&&result&&row[7])for(u32 i=0;i<64;++i)if(mutex_ids[i].live&&mutex_ids[i].id==row[7])mutex_ids[i].live=0;
 --mutex_pending;mutex_emit(row);LeaveCriticalSection(mutex_lock);
}
static HANDLE WIN observe_create(void* security,u32 owned,const char* name){
 u32 row[32]={0},error=GetLastError();u8 fp[512]__attribute__((aligned(16)));mutex_fp_save(fp);row[11]=security!=0;row[12]=name!=0;mutex_begin(row,1,0,owned,(u32)__builtin_return_address(0),error);mutex_fp_restore(fp);SetLastError(error);
 HANDLE result=original_create(security,owned,name);error=GetLastError();mutex_fp_save(fp);mutex_end(row,result,result!=0,error);mutex_fp_restore(fp);SetLastError(error);return result;
}
static u32 mutex_handle_call(HANDLE handle,u32 parameter,u32 operation,u32 caller){
 u32 row[32]={0},error=GetLastError();u8 fp[512]__attribute__((aligned(16)));mutex_fp_save(fp);mutex_begin(row,operation,handle,parameter,caller,error);mutex_fp_restore(fp);SetLastError(error);
 u32 result=operation==2?original_wait(handle,parameter):operation==3?original_release(handle):original_close(handle);error=GetLastError();mutex_fp_save(fp);mutex_end(row,handle,result,error);mutex_fp_restore(fp);SetLastError(error);return result;
}
static u32 WIN observe_wait(HANDLE handle,u32 timeout){return mutex_handle_call(handle,timeout,2,(u32)__builtin_return_address(0));}
static u32 WIN observe_release(HANDLE handle){return mutex_handle_call(handle,0,3,(u32)__builtin_return_address(0));}
static u32 WIN observe_close(HANDLE handle){return mutex_handle_call(handle,0,4,(u32)__builtin_return_address(0));}
static int mutex_install(void){
 char name[1024];u32 size=GetEnvironmentVariableA("MNM_DRAW_CURSOR_MUTEX_FILE",name,sizeof(name));if(!size)return GetLastError()==203;if(size>=sizeof(name))return 0;
 if((u32)GetModuleHandleA(0)!=0x400000||!mutex_readable((void*)0x5c50a0,0xa4))return 0;
 void* kernel=GetModuleHandleA("kernel32.dll");original_create=(CreateFn)GetProcAddress(kernel,"CreateMutexA");original_wait=(WaitFn)GetProcAddress(kernel,"WaitForSingleObject");original_release=(HandleFn)GetProcAddress(kernel,"ReleaseMutex");original_close=(HandleFn)GetProcAddress(kernel,"CloseHandle");
 if(!original_create||!original_wait||!original_release||!original_close||mutex_get((void*)0x5c50b8)!=(u32)original_create||mutex_get((void*)0x5c5140)!=(u32)original_wait||mutex_get((void*)0x5c513c)!=(u32)original_release||mutex_get((void*)0x5c50a0)!=(u32)original_close)return 0;
 void* drawing=GetModuleHandleA("MnmDrawing.dll");original_point=(PointFn)GetProcAddress(drawing,"DrawingInputPointV1");if(!original_point)return 0;
 InitializeCriticalSection(mutex_lock);mutex_file=CreateFileA(name,0x40000000,3,0,1,0x80,0);if(mutex_file==(HANDLE)-1){mutex_file=0;return 0;}
 u32 old,restore;if(!VirtualProtect((void*)0x5c50a0,0xa4,4,&old)){CloseHandle(mutex_file);mutex_file=0;return 0;}
 *(void**)0x5c50b8=(void*)&observe_create;*(void**)0x5c5140=(void*)&observe_wait;*(void**)0x5c513c=(void*)&observe_release;*(void**)0x5c50a0=(void*)&observe_close;VirtualProtect((void*)0x5c50a0,0xa4,old,&restore);return 1;
}
static void mutex_finish(void){
 if(!mutex_file)return;u32 header[16]={0};mutex_copy(header,"MNMDMX01",8);header[2]=1;header[3]=128;header[4]=0x40209ca7;header[5]=65536;header[6]=mutex_count;header[7]=mutex_dropped;header[8]=mutex_pending;header[9]=mutex_calls;header[10]=identity_overflow;header[11]=next_mutex_id;
 u32 wrote=0;if(!WriteFile(mutex_file,header,64,&wrote,0)||wrote!=64)ExitProcess(99);u32 bytes=mutex_count*128,at=0;while(at<bytes){wrote=0;if(!WriteFile(mutex_file,(u8*)mutex_rows+at,bytes-at,&wrote,0)||!wrote||wrote>bytes-at)ExitProcess(99);at+=wrote;}if(!CloseHandle(mutex_file))ExitProcess(99);mutex_file=0;
}
__declspec(dllexport) void CursorMutexAnchor(void){}
int WIN DllMain(void* module,u32 reason,void* reserved){(void)module;(void)reserved;u32 error=GetLastError();u8 fp[512]__attribute__((aligned(16)));mutex_fp_save(fp);int result=1;if(reason==1)result=mutex_install();if(reason==0)mutex_finish();mutex_fp_restore(fp);SetLastError(error);return result;}
