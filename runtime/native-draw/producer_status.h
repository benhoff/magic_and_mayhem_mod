/* Launch-local v2 diagnostic, fixed DWORD wire layout: header[4], 6*32 slots of
 * installed/native/forwarded counts. No loader lookup, file IO or waits on calls.
 * Readers sample after producer exit; concurrent snapshots are approximate. */
#define NP_STATUS_WORDS (4+6*32*3)
static HANDLE np_status_file,np_status_mapping;
static volatile u32* np_status_view;
__declspec(dllexport) void WIN NativeDrawProducerCountV1(u32 family,u32 route,u32 event){
 if(family>=6 || route>=32 || event>=3)ExitProcess(126);
 if(np_status_view)__atomic_add_fetch(np_status_view+4+(family*32+route)*3+event,1,__ATOMIC_RELAXED);
}
static void np_status_start(void){
 char flag[4];if(GetEnvironmentVariableA("MNM_NATIVE_PIXEL_COUNTERS",flag,4)!=1 || flag[0]!='1')return;
 np_status_file=CreateFileA("native-pixel-producers.bin",0xc0000000,3,0,2,0x80,0);
 if(np_status_file==(HANDLE)-1)ExitProcess(126);
 np_status_mapping=CreateFileMappingA(np_status_file,0,4,0,NP_STATUS_WORDS*4,0);
 if(!np_status_mapping)ExitProcess(126);
 np_status_view=(volatile u32*)MapViewOfFile(np_status_mapping,2,0,0,NP_STATUS_WORDS*4);
 if(!np_status_view)ExitProcess(126);
 np_status_view[0]=0x50504e4d;np_status_view[1]=2;np_status_view[2]=NP_STATUS_WORDS*4;np_status_view[3]=6;
}
static void np_status_finish(void){
 if(np_status_view){FlushViewOfFile((const void*)np_status_view,NP_STATUS_WORDS*4);UnmapViewOfFile((const void*)np_status_view);np_status_view=0;}
 if(np_status_mapping)CloseHandle(np_status_mapping);if(np_status_file)CloseHandle(np_status_file);
}
