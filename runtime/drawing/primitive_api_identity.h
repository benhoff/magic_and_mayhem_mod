/* Query outside journal_lock: never invert the scene/journal lock ordering. */
#include "../../protocols/include/mnm/drawing_primitive_point_v1.h"
typedef u32(WIN* PrimitivePoint)(u32*,u32);
static PrimitivePoint primitive_api_getter;
static HANDLE primitive_api_file;
static u32 primitive_api_records,primitive_api_entries;
static int primitive_api_open(void){
 char flag[2];SetLastError(0);u32 n=GetEnvironmentVariableA("MNM_DRAW_PRIMITIVE_LIFETIMES",flag,2);
 if(!n)return GetLastError()==203;
 if(n!=1||flag[0]!='1'||!producer_points_enabled)return 0;
 char name[600];path(name,"api-primitive-identities.bin");primitive_api_file=CreateFileA(name,0x40000000,3,0,1,0x80,0);
 if(primitive_api_file==(HANDLE)-1){primitive_api_file=0;return 0;}
 u32 h[16]={0};copy(h,MNM_PRIMITIVE_API_MAGIC,8);h[2]=MNM_PRIMITIVE_API_VERSION;h[3]=MNM_PRIMITIVE_API_ROW_BYTES;h[4]=0x40209ca7;h[5]=MNM_PRIMITIVE_API_MAX_ROWS;h[6]=MNM_PRIMITIVE_POINT_BYTES;
 return write(primitive_api_file,h,64);
}
static void primitive_api_capture(struct Context* c,u32 phase){
 if(!primitive_api_file||!c->observed||c->r[1]!=1)return;
 u32 error=GetLastError();PrimitivePoint getter=__atomic_load_n(&primitive_api_getter,__ATOMIC_ACQUIRE);
 if(!getter){void* scene=GetModuleHandleA("MnmScene.dll");getter=scene?(PrimitivePoint)GetProcAddress(scene,"SceneDrawingPrimitivePointV1"):0;if(!getter)ExitProcess(99);__atomic_store_n(&primitive_api_getter,getter,__ATOMIC_RELEASE);}
 u32 r[64]={0};r[1]=phase;r[2]=c->r[29];r[3]=c->r[6];r[4]=c->r[2];r[5]=c->r[3];r[6]=c->interval_related;
 if(phase>=2)r[7]=c->r[8];if(phase==3)r[8]=c->r[0];
 if(!getter(r+16,MNM_PRIMITIVE_POINT_BYTES)||r[17]!=r[3])ExitProcess(99);
 EnterCriticalSection(journal_lock);if(phase==1)++primitive_api_entries;r[0]=++primitive_api_records;
 if(primitive_api_records>=MNM_PRIMITIVE_API_MAX_ROWS||!write(primitive_api_file,r,256))ExitProcess(99);
 LeaveCriticalSection(journal_lock);SetLastError(error);
}
/* Caller owns journal_lock, and no pending observed API calls remain. */
static void primitive_api_close(u32 reason){
 if(!primitive_api_file)return;u32 r[64]={0};r[0]=++primitive_api_records;r[1]=4;r[2]=reason;r[3]=GetCurrentThreadId();r[4]=primitive_api_entries;
 if(primitive_api_records>MNM_PRIMITIVE_API_MAX_ROWS||!write(primitive_api_file,r,256)||!CloseHandle(primitive_api_file))ExitProcess(99);primitive_api_file=0;
}
