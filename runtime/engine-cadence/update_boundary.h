API void* WIN GetProcAddress(void*,const char*);
typedef u32(WIN* SourceUpdateBoundary)(u32,u32,u32,u32);
static u32 update_boundary_enabled;
static SourceUpdateBoundary update_boundary_source;
static int update_boundary_configure(void){
 char flag[2];SetLastError(0);u32 n=GetEnvironmentVariableA("MNM_DRAW_WHOLE_UPDATES",flag,2);if(!n)return GetLastError()==203;if(n!=1||flag[0]!='1')return 0;update_boundary_enabled=1;return 1;
}
static void update_boundary_observe(const u32* r){
 if(!update_boundary_enabled||r[1]!=2)return;if(!update_boundary_source){void* module=GetModuleHandleA("MnmScene.dll");update_boundary_source=module?(SourceUpdateBoundary)GetProcAddress(module,"SceneDrawingUpdateBoundaryV1"):0;if(!update_boundary_source)ExitProcess(99);}
 if(update_boundary_source(r[0],r[6],r[5],r[10])!=1)ExitProcess(99);
}
