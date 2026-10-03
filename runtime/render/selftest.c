#include "../shadow/win32_min.h"
API void WIN RenderInstallForTest(void*,u32);
API i32 WIN RenderCreateForTest(void*,void*,void**,void*);
static char startup_mode[16];
static i32 WIN startup_create(void* guid,void** result,void* outer){
    if(guid!=(void*)0x1234 || outer!=(void*)0x5678 || GetLastError()!=0x77)ExitProcess(80);
    static void* table[7];static void** draw=table;
    SetLastError(0x88);*result=startup_mode[0]=='o'?&draw:0;
    return startup_mode[0]=='f'?(i32)0x887600ff:0;
}
static u16 pixels[8]={0xf800,0xf800,0x07e0,0x07e0,0x001f,0x001f,0xffff,0xffff};
static u32 locks,unlocks,calls,fail_primary_lock;
static i32 WIN query(void* object,const void* guid,void** output){(void)object;(void)guid;(void)output;return -1;}
static i32 WIN description(void* object,u32* desc){(void)object;desc[26]=0x200;return 0;}
static i32 WIN lock(void* object,void* rect,u32* desc,u32 flags,HANDLE event){
    (void)object;(void)rect;(void)flags;(void)event;++locks;
    if(fail_primary_lock){SetLastError(0x99);return (i32)0x887601ae;}
    desc[2]=2;desc[3]=4;desc[4]=8;desc[9]=(u32)pixels;desc[19]=0x40;
    desc[21]=16;desc[22]=0xf800;desc[23]=0x07e0;desc[24]=0x001f;return 0;
}
static i32 WIN unlock(void* object,void* rect){(void)object;if(rect)ExitProcess(3);++unlocks;return 0;}
static i32 WIN blt(void* object,void* dst,void* src,void* rect,u32 flags,void* effects){
    (void)object;(void)dst;(void)src;(void)rect;(void)flags;(void)effects;++calls;SetLastError(0x88);return 17;
}
static i32 WIN flip(void* object,void* dst,u32 flags){(void)object;(void)dst;(void)flags;++calls;SetLastError(0x88);return 23;}
#include "draw_selftest.h"
#include "history_selftest.h"
#include "palette_selftest.h"
#include "flip_selftest.h"
void start(void){
    if(GetEnvironmentVariableA("MNM_STARTUP_SELFTEST",startup_mode,sizeof(startup_mode))){
        void* result=0;SetLastError(0x77);
        i32 status=RenderCreateForTest((void*)&startup_create,(void*)0x1234,&result,(void*)0x5678);
        if(status!=(startup_mode[0]=='f'?(i32)0x887600ff:0) || GetLastError()!=0x88 ||
           (startup_mode[0]=='o'?result==0:result!=0))ExitProcess(81);
        ExitProcess(0);
    }
    char failure_mode[8];fail_primary_lock=GetEnvironmentVariableA("MNM_PRIMARY_LOCK_FAILURE_SELFTEST",failure_mode,sizeof(failure_mode))!=0;
    void* table[33]={0};table[0]=(void*)&query;table[5]=(void*)&blt;table[11]=(void*)&flip;
    table[22]=(void*)&description;table[25]=(void*)&lock;table[32]=(void*)&unlock;
    void** object=table;RenderInstallForTest(&object,14);
    typedef i32 (WIN *Blt)(void*,void*,void*,void*,u32,void*);
    typedef i32 (WIN *Flip)(void*,void*,u32);
    SetLastError(0x77);
    if(((Blt)table[5])(&object,0,0,0,0,0)!=17 || GetLastError()!=0x88)ExitProcess(1);
    if(((Flip)table[11])(&object,0,0)!=23 || GetLastError()!=0x88)ExitProcess(2);
    char no_readback[8];
    if(GetEnvironmentVariableA("MNM_RENDER_NO_READBACK",no_readback,sizeof(no_readback))){
        if(locks||unlocks||calls!=2)ExitProcess(4);
        ExitProcess(0);
    }
    if(locks!=2||unlocks!=(fail_primary_lock?0u:2u)||calls!=2)ExitProcess(4);
    if(fail_primary_lock)ExitProcess(0);
    char capture_path[512];
    if(GetEnvironmentVariableA("MNM_RENDER_CAPTURE_DIR",capture_path,sizeof(capture_path))){
        char history[32];if(GetEnvironmentVariableA("MNM_FLIP_SELFTEST",history,sizeof(history)))test_flip_history();
        else if(GetEnvironmentVariableA("MNM_PALETTE_SELFTEST",history,sizeof(history)))test_palette_history();
        else if(GetEnvironmentVariableA("MNM_HISTORY_SELFTEST",history,sizeof(history)))test_surface_history();
        else test_draw_capture();
    }
    ExitProcess(0);
}
