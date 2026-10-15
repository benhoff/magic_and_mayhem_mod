/* Selected imported lifetimes only; Wine-internal aliases/releases stay open. */
static HANDLE wine_lifetime_file;
static u32 wine_lifetime_records,wine_lifetime_calls,wine_lifetime_generations;
static void* wine_lifetime_original[4];
static struct {void* pointer;u32 kind,generation;} wine_lifetime_objects[2048];
static u32 wine_lifetime_find(void* pointer,u32 kind){
 for(u32 i=0;i<2048;++i)if(wine_lifetime_objects[i].pointer==pointer&&wine_lifetime_objects[i].kind==kind)return wine_lifetime_objects[i].generation;
 return 0;
}
static u32 wine_lifetime_create(void* pointer,u32 kind){
 if(!pointer||wine_lifetime_find(pointer,kind))ExitProcess(99);
 for(u32 i=0;i<2048;++i)if(!wine_lifetime_objects[i].pointer){wine_lifetime_objects[i].pointer=pointer;wine_lifetime_objects[i].kind=kind;return wine_lifetime_objects[i].generation=++wine_lifetime_generations;}
 ExitProcess(99);return 0;
}
static void wine_lifetime_emit(u32* r){
 r[0]=++wine_lifetime_records;if(wine_lifetime_records>131072||!write(wine_lifetime_file,r,96))ExitProcess(99);
}
static void wine_lifetime_entry(u32* r,u32 method,void* object){
 EnterCriticalSection(journal_lock);r[1]=1;r[2]=++wine_lifetime_calls;r[3]=method;r[4]=GetCurrentThreadId();r[7]=GetTickCount();u32 s=wine_submission_slot(),d=wine_submission_parents[s].depth;r[5]=d?wine_submission_parents[s].entry[d-1]:0;r[6]=d;
 if(object){r[10]=wine_submission_identity(object,method==2?1:3);r[11]=wine_lifetime_find(object,method==2?1:3);}
 wine_lifetime_emit(r);LeaveCriticalSection(journal_lock);
}
static void wine_lifetime_return(u32* r,u32 result,u32 error,void** output){
 EnterCriticalSection(journal_lock);r[1]=2;r[7]=GetTickCount();r[8]=result;r[9]=error;
 u32 kind=r[3]<=2?1:3;
 if(r[3]==1||r[3]==3){if((i32)result>=0){if(!readable(output,4)||!*output)r[12]=1;else{r[10]=wine_submission_identity(*output,kind);r[11]=wine_lifetime_create(*output,kind);}}}
 else if(!result&&r[11]){for(u32 i=0;i<2048;++i)if(wine_lifetime_objects[i].generation==r[11]){wine_lifetime_objects[i].pointer=0;break;}r[12]=2;}
 wine_lifetime_emit(r);LeaveCriticalSection(journal_lock);
}
static u32 __attribute__((cdecl)) wine_lifetime_texture_create(void* device,const void* desc,u32 layers,u32 levels,u32 flags,const void* data,void* parent,const void* ops,void** output){
 u32 error=GetLastError(),r[24]={0};u8 fp[512] __attribute__((aligned(16)));snapshot_fp(fp);if(wine_submission_enabled)wine_lifetime_entry(r,1,0);restore_fp(fp);SetLastError(error);
 u32 result=((u32(__attribute__((cdecl))*)(void*,const void*,u32,u32,u32,const void*,void*,const void*,void**))wine_lifetime_original[0])(device,desc,layers,levels,flags,data,parent,ops,output);
 error=GetLastError();snapshot_fp(fp);if(wine_submission_enabled)wine_lifetime_return(r,result,error,output);restore_fp(fp);SetLastError(error);return result;
}
static u32 __attribute__((cdecl)) wine_lifetime_swapchain_create(void* device,const void* desc,void* state,void* parent,const void* ops,void** output){
 u32 error=GetLastError(),r[24]={0};u8 fp[512] __attribute__((aligned(16)));snapshot_fp(fp);if(wine_submission_enabled)wine_lifetime_entry(r,3,0);restore_fp(fp);SetLastError(error);
 u32 result=((u32(__attribute__((cdecl))*)(void*,const void*,void*,void*,const void*,void**))wine_lifetime_original[2])(device,desc,state,parent,ops,output);
 error=GetLastError();snapshot_fp(fp);if(wine_submission_enabled)wine_lifetime_return(r,result,error,output);restore_fp(fp);SetLastError(error);return result;
}
static u32 wine_lifetime_decref(void* object,u32 index,u32 method){
 u32 error=GetLastError(),r[24]={0};u8 fp[512] __attribute__((aligned(16)));snapshot_fp(fp);if(wine_submission_enabled)wine_lifetime_entry(r,method,object);restore_fp(fp);SetLastError(error);
 u32 result=((u32(__attribute__((cdecl))*)(void*))wine_lifetime_original[index])(object);
 error=GetLastError();snapshot_fp(fp);if(wine_submission_enabled)wine_lifetime_return(r,result,error,0);restore_fp(fp);SetLastError(error);return result;
}
static u32 __attribute__((cdecl)) wine_lifetime_texture_decref(void* object){return wine_lifetime_decref(object,1,2);}
static u32 __attribute__((cdecl)) wine_lifetime_swapchain_decref(void* object){return wine_lifetime_decref(object,3,4);}
#include "wine_texture_entry.h"
static int wine_lifetime_open(void){
 if(!wine_texture_entry_mode())return 0;
 char name[600];path(name,"wine-lifetime.bin");wine_lifetime_file=CreateFileA(name,0x40000000,3,0,1,0x80,0);if(wine_lifetime_file==(HANDLE)-1)return 0;
 u32 h[16]={0};copy(h,"MNMWLF01",8);h[2]=1;h[3]=96;h[4]=0x40209ca7;h[5]=131072;h[6]=2048;return write(wine_lifetime_file,h,64);
}
static int wine_lifetime_install(void* ddraw,void* module){
 const u32 iat[4]={0x6d6bc,0x6d6c0,0x6d690,0x6d694},target[4]={0x217480,0x217e10,0x211940,0x211a00};
 const char* names[4]={"wined3d_texture_create","wined3d_texture_decref","wined3d_swapchain_create","wined3d_swapchain_decref"};void* hooks[4]={(void*)&wine_lifetime_texture_create,(void*)&wine_lifetime_texture_decref,(void*)&wine_lifetime_swapchain_create,(void*)&wine_lifetime_swapchain_decref};
 for(u32 i=0;i<4;++i){void* expected=GetProcAddress(module,names[i]);void** slot=(void**)((u32)ddraw+iat[i]);if(expected!=(void*)((u32)module+target[i])||!readable(slot,4)||*slot!=expected)return 0;}
 if(wine_texture_entry_enabled&&!wine_texture_entry_patch(module))return 0;
 for(u32 i=wine_texture_entry_enabled?2:0;i<4;++i){void** slot=(void**)((u32)ddraw+iat[i]);u32 old,unused;if(!VirtualProtect(slot,4,4,&old))return 0;wine_lifetime_original[i]=*slot;*slot=hooks[i];if(!VirtualProtect(slot,4,old,&unused))return 0;}return 1;
}
static void wine_lifetime_close(u32 reason){
 if(!wine_lifetime_file)return;u32 r[24]={0};r[1]=4;r[2]=reason;r[3]=wine_lifetime_calls;r[4]=GetCurrentThreadId();r[7]=GetTickCount();r[11]=wine_lifetime_generations;wine_lifetime_emit(r);if(!CloseHandle(wine_lifetime_file))ExitProcess(99);wine_lifetime_file=0;
}
