/* Process-local native history admission before an original CPU-drawing menu.
 * Legacy rendering has no native owner. A loaded native owner must explicitly
 * admit this quiescent boundary; missing/refused services never dispatch it. */
static int mini_prepare_native_panel(u32 consumer){
 if(consumer>2)return 0;
 u32 error=GetLastError(),ok=1;
 void* renderer=GetModuleHandleA("MnmRender.dll");
 if(renderer){
  if(consumer==2){
   u32 (WIN *prepare)(void)=(u32 (WIN *)(void))GetProcAddress(renderer,"RenderPreparePreferencesV1");
   ok=prepare?prepare():0;
  }else{
   u32 (WIN *prepare)(u32)=(u32 (WIN *)(u32))GetProcAddress(renderer,"RenderPrepareStorageV1");
   ok=prepare?prepare(consumer):0;
  }
 }
 SetLastError(error);return ok!=0;
}
