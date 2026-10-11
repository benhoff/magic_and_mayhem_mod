/* Optional process-local observer ABI. No observer means no callback, no new
 * game state or menu behavior. The enabled experiment requires both DLLs. */
API void* WIN GetProcAddress(HANDLE,const char*);
typedef void(WIN *DrawingActionV1)(u32,u32,u32,u32,u32,u32,u32);
static DrawingActionV1 drawing_action;
static int drawing_action_init(void){
 char session[512];u32 length=GetEnvironmentVariableA("MNM_DRAW_SESSION_DIRECTORY",session,sizeof(session));
 if(length){if(length>=sizeof(session))return 0;HANDLE module=GetModuleHandleA("MnmDrawing.dll");if(!module)return 0;drawing_action=(DrawingActionV1)GetProcAddress(module,"DrawingActionV1");return drawing_action!=0;}
 char flag[2];SetLastError(0);u32 n=GetEnvironmentVariableA("MNM_DRAW02_PUBLICATION",flag,2);
 if(!n)return GetLastError()==203;
 if(n!=1||flag[0]!='1')return 0;
 HANDLE module=GetModuleHandleA("MnmScene.dll");if(!module)return 0;
 drawing_action=(DrawingActionV1)GetProcAddress(module,"SceneDrawingActionV1");return drawing_action!=0;
}
static void drawing_action_record(u32 request,u32 action,u32 screen,u32 generation,u32 argument,u32 phase,u32 status){
 if(drawing_action){u32 error=GetLastError();drawing_action(request,action,screen,generation,argument,phase,status);SetLastError(error);}
}
