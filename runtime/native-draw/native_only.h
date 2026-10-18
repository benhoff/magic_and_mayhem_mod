/* Native-only refusal is terminal before an original error dialog can run.
 * Expected application errors (busy, bad rectangles/keys) retain their ABI. */
static u32 ns_native_only;
static void ns_native_only_initialize(void){
 char flag[2];SetLastError(0);u32 n=GetEnvironmentVariableA("MNM_NATIVE_ONLY",flag,2);
 if(!n){if(GetLastError()==203)return;ExitProcess(124);}
 if(n!=1 || flag[0]!='1' || !ns_ownership_required)ExitProcess(124);
 ns_native_only=1;
}
static i32 ns_unsupported_at(u32 site,void* caller){
 if(ns_native_only){
  u32 r[8]={0x434d4e4d,0x31305041,1,32,0x80004001u,site,(u32)caller,GetCurrentThreadId()},done=0;
  HANDLE file=CreateFileA("native-render-capability-refusal.bin",0x40000000,0,0,1,0x80,0);
  if(file!=(HANDLE)-1){WriteFile(file,r,sizeof(r),&done,0);CloseHandle(file);}
  ExitProcess(124);
 }
 return (i32)0x80004001u;
}
#define NS_UNSUPPORTED ns_unsupported_at(__LINE__,__builtin_return_address(0))
