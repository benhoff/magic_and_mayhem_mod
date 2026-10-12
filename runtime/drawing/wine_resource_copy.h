/* Public cdecl10argument ABI only; no private resource layouts or pixel reads. */
static HANDLE wine_copy_file;
static u32 wine_copy_enabled,wine_copy_records,wine_copy_calls;
static void* wine_copy_original;
static void wine_copy_emit(u32* r){r[0]=++wine_copy_records;if(wine_copy_records>131072||!write(wine_copy_file,r,192))ExitProcess(99);}
static void wine_copy_resource(u32* r,u32 at,void* resource){
 r[at]=wine_resource_id(resource,1);
 for(u32 i=0;i<2048;++i)if(wine_resources[i].id==r[at]){r[at+1]=wine_resources[i].texture;r[at+2]=wine_resources[i].generation;break;}
}
static u32 __attribute__((cdecl)) wine_copy_region(void* context,void* dst,u32 dstsub,u32 x,u32 y,u32 z,void* src,u32 srcsub,const u32* box,u32 flags){
 u32 error=GetLastError(),r[48]={0};u8 fp[512] __attribute__((aligned(16)));snapshot_fp(fp);
 if(wine_copy_enabled){EnterCriticalSection(journal_lock);r[1]=1;r[2]=++wine_copy_calls;r[3]=GetCurrentThreadId();u32 slot=wine_submission_slot(),depth=wine_submission_parents[slot].depth;r[4]=depth?wine_submission_parents[slot].entry[depth-1]:0;r[5]=depth;r[6]=GetTickCount();
  wine_copy_resource(r,10,dst);wine_copy_resource(r,13,src);r[16]=dstsub;r[17]=srcsub;r[18]=x;r[19]=y;r[20]=z;r[21]=flags;r[22]=box!=0;
  if(box){if(readable(box,24))copy(r+23,box,24);else r[22]=2;}
  r[29]=wine_resource_records;r[30]=wine_lifetime_records;wine_copy_emit(r);LeaveCriticalSection(journal_lock);
 }
 restore_fp(fp);SetLastError(error);
 u32 result=((u32(__attribute__((cdecl))*)(void*,void*,u32,u32,u32,u32,void*,u32,const u32*,u32))wine_copy_original)(context,dst,dstsub,x,y,z,src,srcsub,box,flags);
 error=GetLastError();snapshot_fp(fp);
 if(wine_copy_enabled){EnterCriticalSection(journal_lock);r[1]=2;r[6]=GetTickCount();r[7]=result;r[8]=error;r[31]=wine_resource_records;r[32]=wine_lifetime_records;wine_copy_emit(r);LeaveCriticalSection(journal_lock);}
 restore_fp(fp);SetLastError(error);return result;
}
static int wine_copy_open(void){
 char flag[2];u32 n=GetEnvironmentVariableA("MNM_DRAW_WINE_RESOURCE_COPY",flag,2);if(!n)return 1;if(n!=1||flag[0]!='1'||!wine_resource_enabled)return 0;
 char name[600];path(name,"wine-resource-copy.bin");wine_copy_file=CreateFileA(name,0x40000000,3,0,1,0x80,0);if(wine_copy_file==(HANDLE)-1)return 0;
 u32 h[16]={0};copy(h,"MNMWCP01",8);h[2]=1;h[3]=192;h[4]=0x40209ca7;h[5]=131072;if(!write(wine_copy_file,h,64))return 0;wine_copy_enabled=1;return 1;
}
static int wine_copy_install(void* ddraw,void* module){
 if(!wine_copy_enabled)return 1;void** slot=(void**)((u32)ddraw+0x6d598);void* expected=GetProcAddress(module,"wined3d_device_context_copy_sub_resource_region");
 if(expected!=(void*)((u32)module+0x1df0d0)||!readable(slot,4)||*slot!=expected)return 0;
 const u8 prefix[16]={0x55,0x89,0xe5,0x57,0x56,0x53,0x83,0xe4,0xf8,0x81,0xec,0x90,0,0,0,0x8b};if(!readable(expected,16)||!equal(expected,prefix,16))return 0;
 u32 old,unused;if(!VirtualProtect(slot,4,4,&old))return 0;wine_copy_original=*slot;*slot=(void*)&wine_copy_region;return VirtualProtect(slot,4,old,&unused)!=0;
}
static void wine_copy_close(u32 reason){
 if(!wine_copy_file)return;u32 r[48]={0};r[1]=4;r[2]=reason;r[3]=wine_copy_calls;r[4]=GetCurrentThreadId();r[6]=GetTickCount();wine_copy_emit(r);if(!CloseHandle(wine_copy_file))ExitProcess(99);wine_copy_file=0;wine_copy_enabled=0;
}
