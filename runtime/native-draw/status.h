/* Local diagnostic snapshot v2 (96 bytes), not a shared host-pointer contract.
 * No periodic World pixel export. Resource transitions and every256 copies
 * publish counters; the optional path is bound to the private experiment. */
API u32 WIN GetEnvironmentVariableA(const char*,char*,u32);
static char nd_status_path[4096];
static u32 nd_status_exports,nd_peak_surfaces,nd_peak_bytes,nd_peak_dcs,nd_peak_clippers;
static void nd_status(void){
 u32 error=GetLastError(),loaded=GetModuleHandleA("ddraw.dll")!=0;
 if(nd_surfaces>nd_peak_surfaces)nd_peak_surfaces=nd_surfaces;
 if(ns_bytes>nd_peak_bytes)nd_peak_bytes=ns_bytes;
 if(ns_dcs>nd_peak_dcs)nd_peak_dcs=ns_dcs;
 if(nc_objects>nd_peak_clippers)nd_peak_clippers=nc_objects;
 if(nd_status_path[0]){
  u32 record[24]={0x444d4e4d,0x31305752,2,96,loaded,nd_objects,nd_surfaces,ns_dcs,nc_objects,
   nd_creates,ns_creates,ns_locks,ns_copies,ns_flips,++nd_status_exports,nd_peak_surfaces,nd_peak_bytes,nd_peak_dcs,nd_peak_clippers,0,nd_attempts,nd_enumerations,nd_last_create,0},written=0;
  HANDLE out=CreateFileA(nd_status_path,0x40000000,3,0,2,0x80,0);
  if(out==(HANDLE)-1 || !WriteFile(out,record,sizeof(record),&written,0) || written!=sizeof(record))ExitProcess(100);
  CloseHandle(out);
 }
 SetLastError(error);if(loaded)ExitProcess(101);
}
static void nd_status_start(void){
 u32 count=GetEnvironmentVariableA("MNM_NATIVE_DRAW_STATUS",nd_status_path,sizeof(nd_status_path));
 if(count>=sizeof(nd_status_path))ExitProcess(102);nd_status();
}
