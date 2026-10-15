/* The former ten sites call USER32 ClientToScreen, not GDI BitBlt.
 * No production patch is valid. Keep the standalone native service fixture
 * separate, and refuse old installations requesting the unsafe takeover. */
#include "../shadow/win32_min.h"
__declspec(dllexport) void DcCopyAnchor(void){}
#ifdef MNM_DC_COPY_SELFTEST
API void* WIN GetProcAddress(void*,const char*);
API i32 WIN BitBlt(void*,i32,i32,i32,i32,void*,i32,i32,u32);
typedef i32 (WIN *Copy)(void*,i32,i32,i32,i32,void*,i32,i32,u32);
static Copy native_copy;
static u32 native_calls,original_calls;
__declspec(dllexport) u32 WIN NativeDCCopiesV1(void){return native_calls;}
__declspec(dllexport) u32 WIN OriginalDCCopiesV1(void){return original_calls;}
__declspec(dllexport) i32 WIN DcCopyForTest(void* dc,i32 x,i32 y,i32 w,i32 h,void* source,i32 sx,i32 sy,u32 rop){
 const u32 error=GetLastError();
 if(!native_copy)native_copy=(Copy)GetProcAddress(GetModuleHandleA("MnmDraw.dll"),"NativeDrawBitBltV1");
 const i32 result=native_copy?native_copy(dc,x,y,w,h,source,sx,sy,rop):(i32)0x80000000u;
 SetLastError(error);
 if(result!=(i32)0x80000000u){++native_calls;return result;}
 ++original_calls;return BitBlt(dc,x,y,w,h,source,sx,sy,rop);
}
#endif
int WIN DllMain(HANDLE module,u32 reason,void *reserved){(void)module;(void)reserved;if(reason!=1)return 1;
#ifdef MNM_DC_COPY_SELFTEST
 return 1;
#else
 const u32 error=GetLastError();char flag[4];const u32 n=GetEnvironmentVariableA("MNM_DC_COPY_TAKEOVER",flag,4);
 const int ok=!n||(n==1&&flag[0]=='0');SetLastError(error);return ok;
#endif
}
