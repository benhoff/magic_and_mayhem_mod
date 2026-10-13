/* Opt-in installed Wine11.16 metadata only. Never inspect Wine object layouts. */
static u32 wine_submission_enabled,wine_submission_records,wine_submission_calls;
static HANDLE wine_submission_file;
static void* wine_submission_original[2];
static struct {u32 thread,depth,entry[32],submission_depth,submission[32];} wine_submission_parents[32];
static struct {void* pointer;u32 kind,id;} wine_submission_objects[2048];
static u32 wine_submission_ids;
static u32 wine_submission_identity(void* pointer,u32 kind){
 if(!pointer)return 0;
 for(u32 i=0;i<2048;++i)if(wine_submission_objects[i].pointer==pointer&&wine_submission_objects[i].kind==kind)return wine_submission_objects[i].id;
 for(u32 i=0;i<2048;++i)if(!wine_submission_objects[i].pointer){wine_submission_objects[i].pointer=pointer;wine_submission_objects[i].kind=kind;return wine_submission_objects[i].id=++wine_submission_ids;}
 ExitProcess(99);return 0;
}
static u32 wine_submission_slot(void){
 u32 thread=GetCurrentThreadId();
 for(u32 i=0;i<32;++i)if(wine_submission_parents[i].thread==thread)return i;
 for(u32 i=0;i<32;++i)if(!wine_submission_parents[i].thread){wine_submission_parents[i].thread=thread;return i;}
 ExitProcess(99);return 0;
}
static void wine_submission_parent(struct Context* c,u32 phase){
 if(!wine_submission_enabled||!c->observed||c->r[1]!=1)return;
 EnterCriticalSection(journal_lock);u32 i=wine_submission_slot(),n=wine_submission_parents[i].depth;
 if(phase==1){if(n>=32)ExitProcess(99);wine_submission_parents[i].entry[n]=c->r[29];++wine_submission_parents[i].depth;}
 else{if(!n||wine_submission_parents[i].entry[n-1]!=c->r[29])ExitProcess(99);--wine_submission_parents[i].depth;}
 LeaveCriticalSection(journal_lock);
}
#include "wine_lifetime.h"
#include "wine_buffer.h"
#include "wine_client_memory.h"
#include "wine_resource_mapping.h"
#include "wine_source_descriptor.h"
#include "wine_source_map.h"
#include "wine_resource_copy.h"
#include "wine_blit_worker.h"
#include "wine_blit_packet.h"
#include "wine_worker_context.h"
static void wine_submission_emit(u32* r){
 r[0]=++wine_submission_records;if(wine_submission_records>131072||!write(wine_submission_file,r,192))ExitProcess(99);
}
static void wine_submission_rect(u32* r,u32 field,const void* rect,u32 flag){
 if(!rect)return;if(!readable(rect,16)){r[15]|=flag<<16;return;}
 copy(r+field,rect,16);r[15]|=flag;
}
static void wine_submission_entry(u32* r,u32 method,void* first,void* second,const void* src,const void* dst){
 EnterCriticalSection(journal_lock);r[1]=1;r[2]=++wine_submission_calls;r[3]=method;r[4]=GetCurrentThreadId();r[7]=GetTickCount();
 u32 slot=wine_submission_slot(),depth=wine_submission_parents[slot].depth;r[5]=depth?wine_submission_parents[slot].entry[depth-1]:0;r[6]=depth;
 r[10]=wine_submission_identity(first,method==1?1:3);r[11]=wine_submission_identity(second,1);
 if(wine_submission_parents[slot].submission_depth>=32)ExitProcess(99);wine_submission_parents[slot].submission[wine_submission_parents[slot].submission_depth++]=r[2];
 r[27]=wine_lifetime_find(first,method==1?1:3);r[28]=wine_lifetime_find(second,1);r[29]=wine_lifetime_records;
 wine_submission_rect(r,16,src,1);wine_submission_rect(r,20,dst,2);wine_submission_emit(r);LeaveCriticalSection(journal_lock);
}
static void wine_submission_return(u32* r,u32 result,u32 error){
 EnterCriticalSection(journal_lock);r[1]=2;r[8]=result;r[9]=error;r[7]=GetTickCount();u32 slot=wine_submission_slot(),depth=wine_submission_parents[slot].submission_depth;if(!depth||wine_submission_parents[slot].submission[depth-1]!=r[2])ExitProcess(99);--wine_submission_parents[slot].submission_depth;wine_submission_emit(r);LeaveCriticalSection(journal_lock);
}
static u32 __attribute__((cdecl)) wine_submission_blt(void* context,void* dst,u32 dstsub,const void* dstrect,void* src,u32 srcsub,const void* srcrect,u32 flags,const void* fx,u32 filter){
 u32 error=GetLastError(),r[48]={0};u8 fp[512] __attribute__((aligned(16)));snapshot_fp(fp);
 if(wine_submission_enabled){r[12]=dstsub;r[13]=srcsub;r[14]=flags;r[24]=filter;r[25]=fx!=0;wine_submission_entry(r,1,dst,src,srcrect,dstrect);wine_client_source(r,src);wine_retained_source(r,src);wine_source_descriptor(r,src);wine_source_map(r,src);}
 restore_fp(fp);SetLastError(error);
 u32 result=((u32(__attribute__((cdecl))*)(void*,void*,u32,const void*,void*,u32,const void*,u32,const void*,u32))wine_submission_original[0])(context,dst,dstsub,dstrect,src,srcsub,srcrect,flags,fx,filter);
 error=GetLastError();snapshot_fp(fp);if(wine_submission_enabled)wine_submission_return(r,result,error);restore_fp(fp);SetLastError(error);return result;
}
static u32 __attribute__((cdecl)) wine_submission_present(void* swapchain,const void* src,const void* dst,void* window,u32 interval,u32 flags){
 u32 error=GetLastError(),r[48]={0};u8 fp[512] __attribute__((aligned(16)));snapshot_fp(fp);
 if(wine_submission_enabled){r[12]=interval;r[14]=flags;r[26]=window!=0;wine_submission_entry(r,2,swapchain,0,src,dst);}
 restore_fp(fp);SetLastError(error);
 u32 result=((u32(__attribute__((cdecl))*)(void*,const void*,const void*,void*,u32,u32))wine_submission_original[1])(swapchain,src,dst,window,interval,flags);
 error=GetLastError();snapshot_fp(fp);if(wine_submission_enabled)wine_submission_return(r,result,error);restore_fp(fp);SetLastError(error);return result;
}
static int wine_submission_open(void){
 char flag[2];u32 n=GetEnvironmentVariableA("MNM_DRAW_WINE_SUBMISSION",flag,2);if(!n)return 1;
 if(n!=1||flag[0]!='1'||!physical_enabled)return 0;
 char name[600];path(name,"wine-submission.bin");wine_submission_file=CreateFileA(name,0x40000000,3,0,1,0x80,0);if(wine_submission_file==(HANDLE)-1)return 0;
 u32 h[16]={0};copy(h,"MNMWIN02",8);h[2]=2;h[3]=192;h[4]=0x40209ca7;h[5]=131072;h[6]=32;h[7]=2048;
 if(!write(wine_submission_file,h,64)||!wine_lifetime_open()||!wine_client_open()||!wine_buffer_open()||!wine_resource_open()||!wine_source_descriptor_open()||!wine_source_map_open()||!wine_copy_open()||!wine_blit_worker_open()||!wine_blit_packet_open()||!wine_worker_context_open())return 0;wine_submission_enabled=1;return 1;
}
static int wine_submission_install(void* ddraw){
 if(!wine_submission_enabled||wine_submission_original[0])return 1;
 u32 base=(u32)ddraw;void* module=GetModuleHandleA("wined3d.dll");if(!base||!module)return 0;
 const u32 iat[2]={0x6d58c,0x6d6ac},target[2]={0x1de1d0,0x213200};
 const char* names[2]={"wined3d_device_context_blt","wined3d_swapchain_present"};void* hooks[2]={(void*)&wine_submission_blt,(void*)&wine_submission_present};
 for(u32 i=0;i<2;++i){void* expected=GetProcAddress(module,names[i]);void** slot=(void**)(base+iat[i]);if(expected!=(void*)((u32)module+target[i])||!readable(slot,4)||*slot!=expected)return 0;}
 if(!wine_lifetime_install(ddraw,module)||!wine_client_install(ddraw,module)||!wine_buffer_install(ddraw,module)||!wine_resource_install(ddraw,module)||!wine_source_descriptor_install(module)||!wine_source_map_install()||!wine_copy_install(ddraw,module)||!wine_blit_worker_install(module)||!wine_blit_packet_install(module))return 0;
 for(u32 i=0;i<2;++i){void** slot=(void**)(base+iat[i]);u32 old,unused;if(!VirtualProtect(slot,4,4,&old))return 0;wine_submission_original[i]=*slot;*slot=hooks[i];if(!VirtualProtect(slot,4,old,&unused))return 0;}
 return 1;
}
static void wine_submission_close(u32 reason){
 if(!wine_submission_file)return;wine_blit_packet_close(reason);wine_blit_worker_close(reason);wine_buffer_close(reason);wine_copy_close(reason);wine_source_map_close(reason);wine_source_descriptor_close(reason);wine_resource_close(reason);wine_client_close(reason);wine_lifetime_close(reason);u32 r[48]={0};r[1]=4;r[2]=reason;r[3]=wine_submission_calls;r[4]=GetCurrentThreadId();r[7]=GetTickCount();r[10]=wine_submission_ids;wine_submission_emit(r);if(!CloseHandle(wine_submission_file))ExitProcess(99);wine_submission_file=0;wine_submission_enabled=0;
}
