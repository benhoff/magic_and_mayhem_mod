/* Native policy, separate from recovered Save/Load behavior. Only a native
 * Region Enter which launches World arms a checkpoint. A Main Load never does.
 * Capture on the engine thread before its first initialized gameplay update.
 */
#include "../../protocols/include/mnm/menu_v23.h"
API int WIN CreateDirectoryA(const char*,void*);
API u32 WIN GetFileSize(HANDLE,u32*);
static int campaign_restart_enabled,campaign_restart_armed,campaign_restart_ready;
static int campaign_restart_loading;
static const char campaign_restart_stem[]="__mnm_restart\\level";
static int campaign_restart_file(void){
    HANDLE file=CreateFileA("Save\\__mnm_restart\\level.sav",0x80000000,1,0,3,0x80,0);
    if(file==(HANDLE)-1)return 0;
    u32 size=GetFileSize(file,0);CloseHandle(file);
    return size>12&&size<=256u*1024u*1024u;
}
static void campaign_restart_arm(void){
    if(!campaign_restart_enabled)return;
    campaign_restart_ready=0;campaign_restart_armed=1;campaign_restart_loading=0;
}
static void campaign_restart_capture(void* object){
    u8* p=object;
    if(!campaign_restart_enabled||!campaign_restart_armed||p!=(u8*)0x6cbb78||
       !readable(p,0x100a5)||get(p)!=0x5c5dd8||get(p+4)!=2||get(p+8)!=1||p[0xc]||
       get((void*)0x6f34e0)!=(u32)p||get((void*)0x68991c)||get((void*)0x689920)!=5)return;
    campaign_restart_armed=0;
    if(!CreateDirectoryA("Save\\__mnm_restart",0)&&GetLastError()!=183)return;
    const u8 save_bytes[]={0x6a,0xff,0x68,0x5b,0x22,0x5c,0};
    if(!readable((void*)0x4ea5a0,sizeof(save_bytes))||!equal((void*)0x4ea5a0,save_bytes,sizeof(save_bytes)))return;
    typedef u32 (__attribute__((fastcall)) *SaveFn)(const char*,u32);
    u32 result=((SaveFn)0x4ea5a0)(campaign_restart_stem,1);
    campaign_restart_ready=result==1&&campaign_restart_file();
    if(campaign_restart_ready)checkpoint_storage_sample(MNM_MENU_SAVE_SCREEN,1);
    record(32,object,get((void*)0x6c4830),campaign_restart_ready);
}
