/* Preserve original surface wrappers; replace their admitted BitBlt pixels. */
#include "../shadow/win32_min.h"
API void* WIN GetProcAddress(void*,const char*);
API u32 WIN GetCurrentThreadId(void);
typedef i32 (WIN *Copy)(void*,i32,i32,i32,i32,void*,i32,i32,u32);
static Copy native_copy;
static volatile u32 owner;
static u32 native_calls,original_calls;
static i32 WIN copy(void* dc,i32 x,i32 y,i32 w,i32 h,void* source,i32 sx,i32 sy,u32 rop){
 const u32 error=GetLastError(),thread=GetCurrentThreadId();i32 result=(i32)0x80000000u;
 if(!owner)__sync_bool_compare_and_swap(&owner,0,thread);
 if(owner==thread&&native_copy)result=native_copy(dc,x,y,w,h,source,sx,sy,rop);
 SetLastError(error);
 if(result!=(i32)0x80000000u){++native_calls;return result;}
 ++original_calls;return (*(Copy*)0x5c51fc)(dc,x,y,w,h,source,sx,sy,rop);
}
static const u32 calls[]={0x58bfe2,0x58c1d6,0x58c3f5,0x58c534,0x58c6f8,0x58c922,0x58cb17,0x58cc16,0x58cdd1,0x58cfa1};
static int install(void){
 char flag[4];const u32 n=GetEnvironmentVariableA("MNM_DC_COPY_TAKEOVER",flag,4);if(!n)return 1;
 if(n!=1||flag[0]!='1'||(u32)GetModuleHandleA(0)!=0x400000)return 0;
 const u8 expected[6]={0xff,0x15,0xfc,0x51,0x5c,0};
 for(u32 i=0;i<sizeof(calls)/sizeof(calls[0]);++i)for(u32 j=0;j<6;++j)if(*(const u8*)(calls[i]+j)!=expected[j])return 0;
 native_copy=(Copy)GetProcAddress(GetModuleHandleA("MnmDraw.dll"),"NativeDrawBitBltV1");if(!native_copy)return 0;
 u32 pages[10],protect[10],count=0;
 for(u32 i=0;i<10;++i){const u32 page=calls[i]&~4095u;u32 j=0;while(j<count&&pages[j]!=page)++j;if(j<count)continue;
  if(!VirtualProtect((void*)page,4096,0x40,protect+count)){while(count){--count;u32 ignored;if(!VirtualProtect((void*)pages[count],4096,protect[count],&ignored))ExitProcess(123);}return 0;}pages[count++]=page;}
 for(u32 i=0;i<10;++i){u8 *at=(u8*)calls[i];at[0]=0xe8;*(u32*)(at+1)=(u32)copy-calls[i]-5;at[5]=0x90;}
 while(count){--count;u32 ignored;if(!VirtualProtect((void*)pages[count],4096,protect[count],&ignored)||!FlushInstructionCache(GetCurrentProcess(),(void*)pages[count],4096))ExitProcess(123);}
 return 1;
}
__declspec(dllexport) void DcCopyAnchor(void){}
__declspec(dllexport) u32 WIN NativeDCCopiesV1(void){return native_calls;}
__declspec(dllexport) u32 WIN OriginalDCCopiesV1(void){return original_calls;}
#ifdef MNM_DC_COPY_SELFTEST
__declspec(dllexport) int WIN DcCopyInstallForTest(void){return install();}
__declspec(dllexport) i32 WIN DcCopyForTest(void* dc,i32 x,i32 y,i32 w,i32 h,void* source,i32 sx,i32 sy,u32 rop){
 if(!native_copy)native_copy=(Copy)GetProcAddress(GetModuleHandleA("MnmDraw.dll"),"NativeDrawBitBltV1");return copy(dc,x,y,w,h,source,sx,sy,rop);
}
#endif
int WIN DllMain(HANDLE module,u32 reason,void *reserved){(void)module;(void)reserved;if(reason!=1)return 1;
#ifdef MNM_DC_COPY_SELFTEST
 return 1;
#else
 const u32 error=GetLastError();const int ok=install();SetLastError(error);return ok;
#endif
}
