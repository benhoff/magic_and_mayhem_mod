#include "../../protocols/include/mnm/frame_v1.h"
/* Startup observations only: forward callbacks/context/flags unchanged. */
typedef i32 (WIN *EnumerateDraw)(void*,void*);
typedef i32 (WIN *EnumerateDrawEx)(void*,void*,u32);
typedef void* (WIN *ProcAddress)(void*,const char*);
static EnumerateDraw original_enumerate;
static EnumerateDrawEx original_enumerate_ex;
static ProcAddress original_proc_address;
static void enumeration_begin(u32 kind){
    __atomic_store_n(stream+MNM_FRAME_V1_ENUMERATION_KIND_OFFSET/4,kind,__ATOMIC_RELAXED);
    __atomic_add_fetch(stream+MNM_FRAME_V1_ENUMERATION_COUNT_OFFSET/4,1,__ATOMIC_RELAXED);
    __atomic_store_n(stream+MNM_FRAME_V1_STATUS_OFFSET/4,MNM_FRAME_V1_STATUS_ENUMERATING,__ATOMIC_RELEASE);
}
static void enumeration_end(i32 status){
    __atomic_store_n(stream+MNM_FRAME_V1_ENUMERATION_HRESULT_OFFSET/4,(u32)status,__ATOMIC_RELAXED);
    __atomic_store_n(stream+MNM_FRAME_V1_STATUS_OFFSET/4,status<0?MNM_FRAME_V1_STATUS_ENUMERATION_FAILED:MNM_FRAME_V1_STATUS_ENUMERATION_COMPLETE,__ATOMIC_RELEASE);
}
static i32 WIN enumerate_draw(void* callback,void* context){
    enumeration_begin(MNM_FRAME_V1_ENUMERATION_KIND_ENUMERATE_A);i32 status=original_enumerate(callback,context);u32 error=GetLastError();
    enumeration_end(status);SetLastError(error);return status;
}
static i32 WIN enumerate_draw_ex(void* callback,void* context,u32 flags){
    enumeration_begin(MNM_FRAME_V1_ENUMERATION_KIND_ENUMERATE_EX_A);i32 status=__atomic_load_n(&original_enumerate_ex,__ATOMIC_ACQUIRE)(callback,context,flags);u32 error=GetLastError();
    enumeration_end(status);SetLastError(error);return status;
}
static void* WIN proc_address(void* module,const char* name){
    void* result=original_proc_address(module,name);u32 error=GetLastError();
    /* Ordinals and unrelated exports always retain the original pointer. */
    static const char ex[]="DirectDrawEnumerateExA";
    if(result && (u32)name>65535 && readable(name,sizeof(ex)) && same(name,ex,sizeof(ex)) &&
       module==GetModuleHandleA("ddraw.dll")){
        __atomic_store_n(&original_enumerate_ex,(EnumerateDrawEx)result,__ATOMIC_RELEASE);result=(void*)&enumerate_draw_ex;
    }
    SetLastError(error);return result;
}
