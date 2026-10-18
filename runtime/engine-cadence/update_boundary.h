API void* WIN GetProcAddress(void*,const char*);
typedef u32(WIN* SourceUpdatePrepare)(u32,u32,u32,u32);
typedef u32(WIN* SourceUpdateCommit)(u32,u32,u32,u32,u32);
typedef u32(WIN* SourceUpdateCancel)(u32);
static u32 update_boundary_enabled,update_boundary_token;
static SourceUpdatePrepare update_boundary_source_prepare;
static SourceUpdateCommit update_boundary_source_commit;
static SourceUpdateCancel update_boundary_source_cancel;
static int update_boundary_configure(void){
 char flag[2];SetLastError(0);u32 n=GetEnvironmentVariableA("MNM_DRAW_WHOLE_UPDATES",flag,2);if(!n)return GetLastError()==203;if(n!=1||flag[0]!='1')return 0;update_boundary_enabled=1;return 1;
}
/* Called before the accepted event's QPC sample. Scene retains its private
 * recorder lock until exact commit/cancel; no drawing journal lock is held. */
static void update_boundary_prepare(const u32* r){
 if(!update_boundary_enabled||r[1]!=2)return;
 if(update_boundary_token)ExitProcess(99);
 if(!update_boundary_source_prepare||!update_boundary_source_commit||!update_boundary_source_cancel){
  void* module=GetModuleHandleA("MnmScene.dll");
  update_boundary_source_prepare=module?(SourceUpdatePrepare)GetProcAddress(module,"SceneDrawingUpdatePrepareV1"):0;
  update_boundary_source_commit=module?(SourceUpdateCommit)GetProcAddress(module,"SceneDrawingUpdateCommitV1"):0;
  update_boundary_source_cancel=module?(SourceUpdateCancel)GetProcAddress(module,"SceneDrawingUpdateCancelV1"):0;
  if(!update_boundary_source_prepare||!update_boundary_source_commit||!update_boundary_source_cancel)ExitProcess(99);
 }
 update_boundary_token=update_boundary_source_prepare(r[0],r[6],r[5],r[10]);if(!update_boundary_token)ExitProcess(99);
}
static void update_boundary_cancel(void){
 if(!update_boundary_token)return;
 if(update_boundary_source_cancel(update_boundary_token)!=1)ExitProcess(99);update_boundary_token=0;
}
static void update_boundary_observe(const u32* r){
 if(!update_boundary_enabled||r[1]!=2)return;
 if(!update_boundary_token||update_boundary_source_commit(update_boundary_token,r[0],r[6],r[5],r[10])!=1)ExitProcess(99);update_boundary_token=0;
}
