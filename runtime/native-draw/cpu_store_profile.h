/* Optional memory-only accepted-store census. No calls or waits in the handler;
 * a bounded diagnostic file is flushed only after the handler is retired. */
#define NS_STORE_PROFILE_SITES 64u
#define NS_STORE_PROFILE_LIMIT 10000000u
static HANDLE ns_store_profile_file;
static u32 ns_store_profile_rows[NS_STORE_PROFILE_SITES][10];
static u32 ns_store_profile_used,ns_store_profile_total,ns_store_profile_flags;
static volatile u32 ns_store_profile_busy;
static void ns_store_profile_initialize(u32 mode){
 u32 error=GetLastError();char path[2048];
 u32 n=GetEnvironmentVariableA("MNM_NATIVE_CPU_STORE_PROFILE",path,sizeof(path));
 if(!n){SetLastError(error);return;}
 if(n>=sizeof(path) || mode!=3)ExitProcess(124);
 ns_store_profile_file=CreateFileA(path,0x40000000,1,0,1,0x80,0);
 if(ns_store_profile_file==(HANDLE)-1){ns_store_profile_file=0;ExitProcess(124);}
 SetLastError(error);
}
static void ns_store_profile_record(u32 pc,u32 width,u32 length){
 if(!ns_store_profile_file)return;
 if(!__sync_bool_compare_and_swap(&ns_store_profile_busy,0,1)){
  __sync_fetch_and_or(&ns_store_profile_flags,1);return;
 }
 if(ns_store_profile_total>=NS_STORE_PROFILE_LIMIT){__sync_fetch_and_or(&ns_store_profile_flags,8);goto done;}
 if((width!=1 && width!=2 && width!=4) || !length || length>15){__sync_fetch_and_or(&ns_store_profile_flags,4);goto done;}
 u32 code[4]={0},index=0;
 for(u32 i=0;i<length;++i)code[i/4]|=(u32)((const u8*)pc)[i]<<((i%4)*8);
 while(index<ns_store_profile_used && ns_store_profile_rows[index][0]!=pc)++index;
 if(index==ns_store_profile_used){
  if(index==NS_STORE_PROFILE_SITES){__sync_fetch_and_or(&ns_store_profile_flags,2);goto done;}
  u32* r=ns_store_profile_rows[index];r[0]=pc;r[3]=width;r[4]=length;
  for(u32 i=0;i<4;++i)r[5+i]=code[i];++ns_store_profile_used;
 }
 u32* r=ns_store_profile_rows[index];
 if(r[3]!=width || r[4]!=length){__sync_fetch_and_or(&ns_store_profile_flags,4);goto done;}
 for(u32 i=0;i<4;++i)if(r[5+i]!=code[i]){__sync_fetch_and_or(&ns_store_profile_flags,4);goto done;}
 ++r[1];r[2]+=width;++ns_store_profile_total;
 done:__sync_lock_release(&ns_store_profile_busy);
}
static void ns_store_profile_finish(void){
 if(!ns_store_profile_file)return;
 u32 error=GetLastError(),wrote=0;HANDLE file=ns_store_profile_file;ns_store_profile_file=0;
 u32 quiet=__sync_bool_compare_and_swap(&ns_store_profile_busy,0,1);
 u32 header[10]={0x534d4e4d,0x31305054,1,40,40,NS_STORE_PROFILE_SITES,quiet?ns_store_profile_used:0,
  __atomic_load_n(&ns_store_profile_flags,__ATOMIC_ACQUIRE)|(quiet?0u:1u),quiet?ns_store_profile_total:0,0};
 if(WriteFile(file,header,sizeof(header),&wrote,0) && wrote==sizeof(header) && quiet && ns_store_profile_used)
  WriteFile(file,ns_store_profile_rows,ns_store_profile_used*40,&wrote,0);
 CloseHandle(file);if(quiet)__sync_lock_release(&ns_store_profile_busy);SetLastError(error);
}
