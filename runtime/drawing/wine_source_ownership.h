/* Finite opt-in policy for the pinned private RGB565 source boundary.
 * Admission facts are separate from physical mutation/fence proof journals. */
static u32 wine_source_ownership_capabilities(void){
 return (wine_submission_enabled?1u:0u)|(wine_source_map_enabled&&wine_source_map_bounded?2u:0u)
 |(wine_source_version_enabled?4u:0u)|(wine_gpu_enabled?8u:0u)|(wine_reserve_enabled?16u:0u)
 |(wine_phys_enabled?32u:0u)|(wine_handoff_enabled?64u:0u)|(wine_blit_packet_enabled&&wine_blit_worker_enabled?128u:0u)
 |(wine_phys_installed?256u:0u)|(wine_mutator_entry_installed==6?512u:0u);
}
static int wine_source_ownership_configure(void){
 char flag[2];u32 n=GetEnvironmentVariableA("MNM_DRAW_WINE_SOURCE_OWNERSHIP",flag,2);
 if(!n)return 1;if(n!=1||flag[0]!='1')return 0;wine_source_ownership_required=1;return 1;
}
static void wine_source_ownership_emit(u32 phase){
 u32 r[16]={0};r[0]=++wine_source_ownership_records;r[1]=phase;r[2]=GetCurrentThreadId();r[3]=wine_source_ownership_capabilities();r[4]=wine_source_ownership_copies;r[5]=wine_handoff_count;
 if(!wine_source_ownership_file||!write(wine_source_ownership_file,r,64))ExitProcess(99);
}
static int wine_source_ownership_open(void){
 if(!wine_source_ownership_required)return 1;
 if(!physical_enabled||(wine_source_ownership_capabilities()&255u)!=255u)return 0;
 char name[600];path(name,"wine-source-ownership.bin");wine_source_ownership_file=CreateFileA(name,0x40000000,3,0,1,0x80,0);if(wine_source_ownership_file==(HANDLE)-1)return 0;
 u32 h[16]={0};copy(h,"MNMSOW01",8);h[2]=1;h[3]=64;h[4]=0x40209ca7;h[5]=1023;h[6]=2048;return write(wine_source_ownership_file,h,64);
}
static int wine_source_ownership_install(void){
 if(!wine_source_ownership_required)return 1;
 if(wine_source_ownership_capabilities()!=1023)return 0;
 wine_source_ownership_ready=1;wine_source_ownership_emit(1);return 1;
}
static void wine_source_ownership_require(void){
 if(!wine_source_ownership_required)return;
 if(!wine_source_ownership_ready||wine_source_ownership_capabilities()!=1023){
  if(wine_source_ownership_file)wine_source_ownership_emit(3);ExitProcess(99);
 }
}
static void wine_source_ownership_close(void){
 if(!wine_source_ownership_required)return;wine_source_ownership_require();
 if(wine_handoff_source_active()||wine_source_ownership_copies!=wine_handoff_count){wine_source_ownership_emit(3);ExitProcess(99);}
 wine_source_ownership_emit(2);wine_source_ownership_ready=0;
 if(!CloseHandle(wine_source_ownership_file))ExitProcess(99);wine_source_ownership_file=0;
 /* Required remains set: cached COPY/worker calls after shutdown must refuse. */
}
