/* Public client-memory setter plus exact submitted source point; no byte reads. */
typedef u32(WIN* ClientStoragePoint)(void*,u32,u32*,u32);
static ClientStoragePoint wine_client_getter;
static HANDLE wine_client_file;
static u32 wine_client_enabled,wine_client_records,wine_client_calls,wine_client_queries,wine_client_versions,wine_client_ids;
static void* wine_client_original;
static struct {void* pointer;u32 id;} wine_client_data[2048];
static struct {void* texture;void* memory;u32 generation,subresource,pitch,data,version;} wine_client_bindings[2048];
static u32 wine_client_data_id(void* pointer){if(!pointer)return 0;for(u32 i=0;i<2048;++i)if(wine_client_data[i].pointer==pointer)return wine_client_data[i].id;for(u32 i=0;i<2048;++i)if(!wine_client_data[i].pointer){wine_client_data[i].pointer=pointer;wine_client_data[i].id=++wine_client_ids;return wine_client_data[i].id;}ExitProcess(99);return 0;}
static void wine_client_emit(u32* r){r[0]=++wine_client_records;if(wine_client_records>131072||!write(wine_client_file,r,192))ExitProcess(99);}
static void wine_client_point(void* memory,u32 pitch,u32* out){
 if(!wine_client_getter){void* scene=GetModuleHandleA("MnmScene.dll");wine_client_getter=scene?(ClientStoragePoint)GetProcAddress(scene,"SceneDrawingClientStoragePointV1"):0;if(!wine_client_getter)ExitProcess(99);}
 if(!wine_client_getter(memory,pitch,out,64))memset(out,0,64);
}
static u32 __attribute__((cdecl)) wine_client_update(void* texture,u32 subresource,void* memory,u32 pitch){
 u32 error=GetLastError(),r[48]={0};u8 fp[512] __attribute__((aligned(16)));snapshot_fp(fp);
 if(wine_client_enabled){EnterCriticalSection(journal_lock);r[1]=1;r[2]=++wine_client_calls;r[3]=1;r[4]=GetCurrentThreadId();u32 slot=wine_submission_slot(),depth=wine_submission_parents[slot].depth;r[5]=depth?wine_submission_parents[slot].entry[depth-1]:0;r[6]=depth;r[7]=GetTickCount();r[10]=wine_submission_identity(texture,1);r[11]=wine_lifetime_find(texture,1);r[12]=subresource;r[13]=pitch;r[14]=wine_client_data_id(memory);LeaveCriticalSection(journal_lock);wine_client_point(memory,pitch,r+16);EnterCriticalSection(journal_lock);wine_client_emit(r);LeaveCriticalSection(journal_lock);}
 restore_fp(fp);SetLastError(error);
 u32 result=((u32(__attribute__((cdecl))*)(void*,u32,void*,u32))wine_client_original)(texture,subresource,memory,pitch);error=GetLastError();snapshot_fp(fp);
 if(wine_client_enabled){wine_client_point(memory,pitch,r+32);EnterCriticalSection(journal_lock);r[1]=2;r[7]=GetTickCount();r[8]=result;r[9]=error;
  if((i32)result>=0){u32 at=2048;for(u32 i=0;i<2048;++i)if(wine_client_bindings[i].texture==texture&&wine_client_bindings[i].subresource==subresource){at=i;break;}if(at==2048)for(u32 i=0;i<2048;++i)if(!wine_client_bindings[i].texture){at=i;break;}if(at==2048||!texture)ExitProcess(99);
   wine_client_bindings[at].texture=texture;wine_client_bindings[at].memory=memory;wine_client_bindings[at].generation=r[11];wine_client_bindings[at].subresource=subresource;wine_client_bindings[at].pitch=pitch;wine_client_bindings[at].data=r[14];wine_client_bindings[at].version=++wine_client_versions;r[15]=wine_client_versions;
  }wine_client_emit(r);LeaveCriticalSection(journal_lock);
 }restore_fp(fp);SetLastError(error);return result;
}
static void wine_client_source(const u32* submission,void* texture){
 if(!wine_client_enabled)return;u32 r[48]={0},at=2048;void* memory=0;
 EnterCriticalSection(journal_lock);r[1]=3;r[2]=submission[2];r[3]=2;r[4]=GetCurrentThreadId();r[5]=submission[5];r[6]=submission[6];r[7]=GetTickCount();r[10]=submission[11];r[11]=submission[28];r[12]=submission[13];
 if(r[11])for(u32 i=0;i<2048;++i)if(wine_client_bindings[i].texture==texture&&wine_client_bindings[i].generation==r[11]&&wine_client_bindings[i].subresource==r[12]){at=i;memory=wine_client_bindings[i].memory;r[13]=wine_client_bindings[i].pitch;r[14]=wine_client_bindings[i].data;r[15]=wine_client_bindings[i].version;break;}
 LeaveCriticalSection(journal_lock);if(memory)wine_client_point(memory,r[13],r+16);
 EnterCriticalSection(journal_lock);if(at!=2048&&(wine_client_bindings[at].version!=r[15]||wine_client_bindings[at].generation!=r[11]))ExitProcess(99);++wine_client_queries;wine_client_emit(r);LeaveCriticalSection(journal_lock);
}
static int wine_client_open(void){
 char flag[2];u32 n=GetEnvironmentVariableA("MNM_DRAW_WINE_CLIENT_MEMORY",flag,2);if(!n)return 1;if(n!=1||flag[0]!='1'||!producer_points_enabled)return 0;
 char name[600];path(name,"wine-client-memory.bin");wine_client_file=CreateFileA(name,0x40000000,3,0,1,0x80,0);if(wine_client_file==(HANDLE)-1)return 0;u32 h[16]={0};copy(h,"MNMWCM01",8);h[2]=1;h[3]=192;h[4]=0x40209ca7;h[5]=131072;h[6]=2048;if(!write(wine_client_file,h,64))return 0;wine_client_enabled=1;return 1;
}
static int wine_client_install(void* ddraw,void* module){
 if(!wine_client_enabled)return 1;void** slot=(void**)((u32)ddraw+0x6d700);void* expected=GetProcAddress(module,"wined3d_texture_update_desc");static const u8 prefix[16]={0x8d,0x4c,0x24,0x04,0x83,0xe4,0xf8,0xff,0x71,0xfc,0x55,0x89,0xe5,0x57,0x56,0x53};
 if(expected!=(void*)((u32)module+0x221460)||!readable(expected,16)||!equal(expected,prefix,16)||!readable(slot,4)||*slot!=expected)return 0;u32 old,unused;if(!VirtualProtect(slot,4,4,&old))return 0;wine_client_original=*slot;*slot=(void*)&wine_client_update;if(!VirtualProtect(slot,4,old,&unused))return 0;return 1;
}
static void wine_client_close(u32 reason){if(!wine_client_file)return;u32 r[48]={0};r[1]=4;r[2]=reason;r[3]=wine_client_calls;r[4]=GetCurrentThreadId();r[5]=wine_client_queries;r[7]=GetTickCount();r[10]=wine_client_versions;r[14]=wine_client_ids;wine_client_emit(r);if(!CloseHandle(wine_client_file))ExitProcess(99);wine_client_file=0;wine_client_enabled=0;}
