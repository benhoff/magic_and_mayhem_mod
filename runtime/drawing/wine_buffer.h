/* Getter origins are metadata, not texture creation or storage ownership. */
static HANDLE wine_buffer_file;
static u32 wine_buffer_records,wine_buffer_calls;
static void* wine_buffer_original[2];
static void wine_buffer_emit(u32* r){r[0]=++wine_buffer_records;if(wine_buffer_records>131072||!write(wine_buffer_file,r,96))ExitProcess(99);}
static void* wine_buffer_call(void* swapchain,u32 index,u32 method){
 u32 error=GetLastError(),r[24]={0};u8 fp[512] __attribute__((aligned(16)));snapshot_fp(fp);
 if(wine_submission_enabled){EnterCriticalSection(journal_lock);r[1]=1;r[2]=++wine_buffer_calls;r[3]=method;r[4]=GetCurrentThreadId();u32 s=wine_submission_slot(),d=wine_submission_parents[s].depth;r[5]=d?wine_submission_parents[s].entry[d-1]:0;r[6]=d;r[7]=GetTickCount();r[10]=wine_submission_identity(swapchain,3);r[11]=wine_lifetime_find(swapchain,3);r[12]=index;r[15]=wine_lifetime_records;wine_buffer_emit(r);LeaveCriticalSection(journal_lock);}
 restore_fp(fp);SetLastError(error);
 void* result=method==1?((void*(__attribute__((cdecl))*)(void*))wine_buffer_original[0])(swapchain):((void*(__attribute__((cdecl))*)(void*,u32))wine_buffer_original[1])(swapchain,index);
 error=GetLastError();snapshot_fp(fp);
 if(wine_submission_enabled){EnterCriticalSection(journal_lock);r[1]=2;r[7]=GetTickCount();r[8]=result!=0;r[9]=error;r[13]=wine_submission_identity(result,1);r[14]=wine_lifetime_find(result,1);r[16]=wine_lifetime_records;wine_buffer_emit(r);LeaveCriticalSection(journal_lock);}
 restore_fp(fp);SetLastError(error);return result;
}
static void* __attribute__((cdecl)) wine_buffer_front(void* swapchain){return wine_buffer_call(swapchain,0,1);}
static void* __attribute__((cdecl)) wine_buffer_back(void* swapchain,u32 index){return wine_buffer_call(swapchain,index,2);}
static int wine_buffer_open(void){
 char name[600];path(name,"wine-buffer.bin");wine_buffer_file=CreateFileA(name,0x40000000,3,0,1,0x80,0);if(wine_buffer_file==(HANDLE)-1)return 0;u32 h[16]={0};copy(h,"MNMWBF01",8);h[2]=1;h[3]=96;h[4]=0x40209ca7;h[5]=131072;return write(wine_buffer_file,h,64);
}
static int wine_buffer_install(void* ddraw,void* module){
 const u32 iat[2]={0x6d6a4,0x6d698},target[2]={0x211f80,0x211b40};const char* names[2]={"wined3d_swapchain_get_front_buffer","wined3d_swapchain_get_back_buffer"};void* hooks[2]={(void*)&wine_buffer_front,(void*)&wine_buffer_back};
 for(u32 i=0;i<2;++i){void* expected=GetProcAddress(module,names[i]);void** slot=(void**)((u32)ddraw+iat[i]);if(expected!=(void*)((u32)module+target[i])||!readable(slot,4)||*slot!=expected)return 0;}
 for(u32 i=0;i<2;++i){void** slot=(void**)((u32)ddraw+iat[i]);u32 old,unused;if(!VirtualProtect(slot,4,4,&old))return 0;wine_buffer_original[i]=*slot;*slot=hooks[i];if(!VirtualProtect(slot,4,old,&unused))return 0;}return 1;
}
static void wine_buffer_close(u32 reason){if(!wine_buffer_file)return;u32 r[24]={0};r[1]=4;r[2]=reason;r[3]=wine_buffer_calls;r[4]=GetCurrentThreadId();r[7]=GetTickCount();wine_buffer_emit(r);if(!CloseHandle(wine_buffer_file))ExitProcess(99);wine_buffer_file=0;}
