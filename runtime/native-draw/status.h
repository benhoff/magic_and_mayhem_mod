/* Local diagnostic snapshot v2 (96 bytes), not a shared host-pointer contract.
 * Startup owns a file-backed view; drawing only stores into its resident page.
 * Explicit file IO/flush/unmap/closure stays outside drawing callbacks. External
 * concurrent readers have no atomic snapshot guarantee. Module checks and
 * terminal refusals remain mandatory; diagnostics never wait for admission. */
API u32 WIN GetEnvironmentVariableA(const char*,char*,u32);
API HANDLE WIN CreateFileMappingA(HANDLE,void*,u32,u32,u32,const char*);
API void* WIN MapViewOfFile(HANDLE,u32,u32,u32,u32);
API i32 WIN FlushViewOfFile(const void*,u32);
API i32 WIN UnmapViewOfFile(const void*);
static char nd_status_path[4096];
static HANDLE nd_status_file,nd_status_mapping;
static volatile u32* nd_status_view;
static volatile i32 nd_status_busy;
static u32 nd_status_exports,nd_peak_surfaces,nd_peak_bytes,nd_peak_dcs,nd_peak_clippers;
static void nd_status(void){
 u32 error=GetLastError();__atomic_store_n(&nd_status_phase,1,__ATOMIC_RELAXED);
 u32 loaded=GetModuleHandleA("ddraw.dll")!=0;
 __atomic_store_n(&nd_status_phase,2,__ATOMIC_RELAXED);
 /* Diagnostics may skip a contended snapshot, never wait or bypass refusal. */
 if(!__sync_bool_compare_and_swap(&nd_status_busy,0,1)){
  SetLastError(error);if(loaded)ExitProcess(101);return;
 }
 if(nd_surfaces>nd_peak_surfaces)nd_peak_surfaces=nd_surfaces;
 if(ns_bytes>nd_peak_bytes)nd_peak_bytes=ns_bytes;
 if(ns_dcs>nd_peak_dcs)nd_peak_dcs=ns_dcs;
 if(nc_objects>nd_peak_clippers)nd_peak_clippers=nc_objects;
 if(nd_status_view){
  u32 record[24]={0x444d4e4d,0x31305752,2,96,loaded,nd_objects,nd_surfaces,ns_dcs,nc_objects,
   nd_creates,ns_creates,ns_locks,ns_copies,ns_flips,++nd_status_exports,nd_peak_surfaces,nd_peak_bytes,nd_peak_dcs,nd_peak_clippers,0,nd_attempts,nd_enumerations,nd_last_create,0};
  __atomic_store_n(&nd_status_phase,3,__ATOMIC_RELAXED);
  for(u32 i=0;i<24;++i)nd_status_view[i]=record[i];
 }
 __atomic_store_n(&nd_status_phase,0,__ATOMIC_RELAXED);
 __sync_lock_release(&nd_status_busy);
 SetLastError(error);if(loaded)ExitProcess(101);
}
static void nd_status_start(void){
 u32 count=GetEnvironmentVariableA("MNM_NATIVE_DRAW_STATUS",nd_status_path,sizeof(nd_status_path));
 if(count>=sizeof(nd_status_path))ExitProcess(102);
 if(count){
  nd_status_file=CreateFileA(nd_status_path,0xc0000000,3,0,2,0x80,0);
  if(nd_status_file==(HANDLE)-1)ExitProcess(100);
  nd_status_mapping=CreateFileMappingA(nd_status_file,0,4,0,96,0);
  if(!nd_status_mapping)ExitProcess(100);
  nd_status_view=(volatile u32*)MapViewOfFile(nd_status_mapping,2,0,0,96);
  if(!nd_status_view)ExitProcess(100);
 }
 nd_status();
}
static void nd_status_finish(void){
 if(nd_status_view){
  __atomic_store_n(&nd_status_phase,4,__ATOMIC_RELAXED);
  if(!FlushViewOfFile((const void*)nd_status_view,96))ExitProcess(100);
  if(!UnmapViewOfFile((const void*)nd_status_view))ExitProcess(100);
  nd_status_view=0;
 }
 if(nd_status_mapping){CloseHandle(nd_status_mapping);nd_status_mapping=0;}
 if(nd_status_file){__atomic_store_n(&nd_status_phase,5,__ATOMIC_RELAXED);CloseHandle(nd_status_file);nd_status_file=0;}
}
