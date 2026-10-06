#include "../../protocols/include/mnm/frame_v1.h"
/* Mixed ordered operations, with independent engine storage on both buffers. */
static char os_mode[32];static u32 os_event,os_subject,os_status,os_blt_calls,os_flip_calls,os_query_calls,os_fail;
static u32 os_partial;
static HANDLE os_events;static struct PlSurface os_alias;
static int os_mode_is(const char* name){u32 i=0;while(name[i] && name[i]==os_mode[i])++i;return name[i]==os_mode[i];}
static void* os_normalize(void* object){return object==&os_alias?&pl_surface:object;}
static i32 WIN os_lock(void* object,void* rect,u32* d,u32 flags,HANDLE event){
    os_event=1;os_partial=rect!=0;os_subject=object==&pl_target?2:1;
    i32 status=pl_lock(os_normalize(object),rect,d,flags,event);os_status=(u32)status;
    d[26]=object==&pl_target?0x238:0x1c;if(object==&pl_target){d[1]|=0x20;d[5]=1;}return status;
}
static i32 WIN os_unlock(void* object,void* argument){os_event=2;os_subject=object==&pl_target?2:1;
    i32 status=pl_unlock(os_normalize(object),argument);os_status=(u32)status;return status;}
static i32 WIN os_desc(void* object,u32* d){i32 status=pl_description(os_normalize(object),d);d[26]=object==&pl_target?0x238:0x1c;
    if(object==&pl_target){d[1]|=0x20;d[5]=1;}return status;}
static i32 WIN os_blt(void* object,void* dst,void* source,void* rect,u32 flags,void* effects){
    ++os_blt_calls;os_event=3;os_subject=2;if(os_fail){os_fail=0;if(GetLastError()!=0x77)ExitProcess(250);SetLastError(0x88);os_status=(u32)-1;return -1;}
    i32 status=pl_blt(object,dst,os_normalize(source),rect,flags,effects);os_status=(u32)status;return status;
}
static i32 WIN os_attached(void* object,u32* caps,void** output){
    if(object!=&pl_target || GetLastError()!=0x77 || caps[0]!=4)ExitProcess(251);*output=&pl_surface;SetLastError(0x88);return 23;
}
static i32 WIN os_flip(void* object,void* target,u32 flags){
    if(object!=&pl_target || target || flags!=1 || GetLastError()!=0x77 || pl_surface.held || pl_target.held)ExitProcess(252);
    ++os_flip_calls;os_event=4;os_subject=2;SetLastError(0x88);if(os_fail){os_fail=0;os_status=(u32)-1;return -1;}
    for(u32 i=0;i<32;++i){u8 pixel=pl_target.native[i];pl_target.native[i]=pl_surface.native[i];pl_surface.native[i]=pixel;}os_status=23;return 23;
}
static i32 WIN os_query(void* object,const u8* guid,void** result){
    if(object!=&pl_surface || guid[0]!=0x30 || GetLastError()!=0x77)ExitProcess(253);++os_query_calls;*result=&os_alias;SetLastError(0x88);return 23;
}
static void os_record(void){
    char front[]="front-00000000.bin",back[]="back-00000000.bin";static const char hex[]="0123456789abcdef";
    for(u32 i=0;i<8;++i){front[6+i]=hex[(pl_step>>(28-i*4))&15];back[5+i]=front[6+i];}
    char delay[4];if(GetEnvironmentVariableA("MNM_COMMAND_TEST_DELAY",delay,sizeof(delay)))Sleep(delay[0]=='2'?2:30);
    pl_file(front,pl_target.native,32);pl_file(back,pl_surface.native,32);
    u32 event[6]={pl_step,os_event,os_subject,os_status,bs_stream[MNM_FRAME_V1_FRAME_COUNT_OFFSET/4],os_partial},written;
    if(!WriteFile(os_events,event,24,&written,0) || written!=24)ExitProcess(254);
}
static void os_copy(void){u32 failed=os_fail;SetLastError(0x77);
    if(((i32 (WIN *)(void*,void*,void*,void*,u32,void*))pl_target.table[5])(&pl_target,0,os_mode_is("alias")?(void*)&os_alias:&pl_surface,0,0x1000000,0)!=(failed?-1:17) || GetLastError()!=0x88)ExitProcess(255);pl_record();}
static void os_swap(void){u32 failed=os_fail;SetLastError(0x77);
    if(((i32 (WIN *)(void*,void*,u32))pl_target.table[11])(&pl_target,0,1)!=(failed?-1:23) || GetLastError()!=0x88)ExitProcess(256);pl_record();}
static int os_startup_case(void){return os_mode[0]=='s' && os_mode[1]=='t' && os_mode[2]=='a';}
static int os_sequence_case(void){return os_mode[0]=='s' && os_mode[1]=='e' && os_mode[2]=='q';}
static void os_sequence_test(void){
    u32 d[31]={124};SetLastError(0x77);
    if(((i32 (WIN *)(void*,void*))pl_target.table[22])(&pl_target,d)!=23 || GetLastError()!=0x88)ExitProcess(269);
    SetLastError(0x77);
    if(((i32 (WIN *)(void*,void*))pl_target.table[28])(&pl_target,0)!=23 || GetLastError()!=0x88)ExitProcess(270);
    for(u32 i=0;i<17;++i)pl_cycle(0,1,0x100+i);
    u32 frames=os_mode_is("sequence-max")?32:os_mode_is("sequence-exit")?2:
        (os_mode_is("sequence-budget") || os_mode_is("sequence-invalidate"))?1:3;
    for(u32 i=0;i<frames;++i){
        pl_cycle(0,1,0x700+i*0x41);
        if(os_mode_is("sequence-failed")){os_fail=1;os_copy();}
        os_copy();
    }
    if(os_mode_is("sequence-budget"))for(u32 i=0;i<237;++i)pl_cycle(0,1,0x1f+i);
    if(os_mode_is("sequence-invalidate")){
        SetLastError(0x77);
        if(((i32 (WIN *)(void*))pl_surface.table[27])(&pl_surface)!=23 || GetLastError()!=0x88)ExitProcess(271);
    }
    if(os_mode_is("sequence-three") || os_mode_is("sequence-max")){
        /* Completed streams stay ended while original drawing continues. */
        pl_cycle(0,1,0xffff);os_copy();
    }
    u32 extra=os_mode_is("sequence-three") || os_mode_is("sequence-max");
    if(pl_locks!=17+frames+extra+(os_mode_is("sequence-budget")?237u:0u) || pl_unlocks!=pl_locks ||
       os_blt_calls!=frames*(os_mode_is("sequence-failed")?2u:1u)+extra || os_flip_calls || os_query_calls)ExitProcess(272);
    CloseHandle(os_events);ExitProcess(0);
}
static void os_startup_test(void){
    u32 d[31]={124};SetLastError(0x77);
    if(((i32 (WIN *)(void*,void*))pl_target.table[22])(&pl_target,d)!=23 || GetLastError()!=0x88)ExitProcess(265);
    SetLastError(0x77);
    if(((i32 (WIN *)(void*,void*))pl_target.table[28])(&pl_target,0)!=23 || GetLastError()!=0x88)ExitProcess(266);
    u32 setup=os_mode_is("startup-boundary")?15:
        (os_mode_is("startup-last") || os_mode_is("startup-failed"))?63:
        (os_mode_is("startup-missing") || os_mode_is("startup-too-late"))?64:17;
    for(u32 i=0;i<setup;++i)pl_cycle(0,1,0x100+i);
    if(os_mode_is("startup-invalidate")){
        SetLastError(0x77);
        if(((i32 (WIN *)(void*))pl_surface.table[27])(&pl_surface)!=23 || GetLastError()!=0x88)ExitProcess(267);
    }
    int missing=os_mode_is("startup-missing") || os_mode_is("startup-exit");
    if(!missing){if(os_mode_is("startup-failed")){os_fail=1;os_copy();}os_copy();}
    /* Application work after a completed session must not reopen publication. */
    if(os_mode_is("startup-delayed")){pl_cycle(0,1,0x7e0);os_copy();}
    if(pl_locks!=setup+(os_mode_is("startup-delayed")?1u:0u) || pl_unlocks!=pl_locks ||
       os_blt_calls!=(missing?0u:os_mode_is("startup-delayed")?2u:os_mode_is("startup-failed")?2u:1u) ||
       os_flip_calls || os_query_calls)ExitProcess(268);
    CloseHandle(os_events);ExitProcess(0);
}
static void test_owned_session(void){
    char path[512];GetEnvironmentVariableA("MNM_RENDER_STREAM",path,512);HANDLE f=CreateFileA(path,0xc0000000,3,0,3,0x80,0);
    HANDLE mapping=CreateFileMappingA(f,0,4,0,MNM_FRAME_V1_SIZE,0);CloseHandle(f);bs_stream=MapViewOfFile(mapping,2,0,0,MNM_FRAME_V1_SIZE);CloseHandle(mapping);if(!bs_stream)ExitProcess(257);
    static void* table[33];table[0]=(void*)&os_query;table[5]=(void*)&os_blt;table[11]=(void*)&os_flip;table[12]=(void*)&os_attached;
    table[22]=(void*)&os_desc;table[25]=(void*)&os_lock;table[27]=(void*)&pl_restore;table[28]=(void*)&pl_clipper;table[32]=(void*)&os_unlock;
    static void* alias_table[33];for(u32 i=0;i<33;++i)alias_table[i]=table[i];os_alias.table=alias_table;
    pl_surface.table=pl_target.table=table;pl_surface.bits=pl_target.bits=16;pl_surface.kind=pl_target.kind=14;pl_surface.pitch=pl_target.pitch=16;
    for(u32 i=0;i<32;++i)pl_target.native[i]=0xcc;
    RenderInstallForTest(&pl_surface,14);RenderInstallForTest(&pl_target,14);
    if(os_mode_is("alias")){static const u8 guid[16]={0x30,0x86,0x2b,0x0b,0x35,0xad,0xd0,0x11,0x8e,0xa6,0,0x60,0x97,0x97,0xea,0x5b};void* result=0;SetLastError(0x77);
        if(((i32 (WIN *)(void*,const void*,void**))table[0])(&pl_surface,guid,&result)!=23 || result!=&os_alias || GetLastError()!=0x88)ExitProcess(258);}
    os_events=CreateFileA("session-events.bin",0x40000000,1,0,1,0x80,0);pl_record_hook=&os_record;pl_record();
    if(os_sequence_case())os_sequence_test();
    if(os_startup_case())os_startup_test();
    pl_cycle(0,1,0xf800);i32 rect[4]={1,1,3,3};pl_cycle(rect,1,0x7e0);
    u32 d[31]={124};SetLastError(0x77);if(((i32 (WIN *)(void*,void*))table[22])(&pl_target,d)!=23 || GetLastError()!=0x88)ExitProcess(259);
    SetLastError(0x77);if(((i32 (WIN *)(void*,void*))table[28])(&pl_target,0)!=23 || GetLastError()!=0x88)ExitProcess(260);
    u32 caps[4]={4};void* back;SetLastError(0x77);if(((i32 (WIN *)(void*,void*,void**))table[12])(&pl_target,caps,&back)!=23 || back!=&pl_surface || GetLastError()!=0x88)ExitProcess(261);
    if(os_mode_is("failed")){os_fail=1;os_copy();}os_copy();
    if(os_mode_is("restore")){SetLastError(0x77);if(((i32 (WIN *)(void*))table[27])(&pl_target)!=23 || GetLastError()!=0x88)ExitProcess(262);}
    pl_cycle(0,1,0x1f);pl_cycle(rect,1,0xffff);
    if(os_mode_is("failed")){os_fail=1;os_swap();}os_swap();os_copy();pl_cycle(rect,1,0xf800);os_swap();
    if(os_mode_is("limit"))for(u32 i=0;i<8;++i)pl_cycle(0,1,0x7e0+i);
    if(os_mode_is("held")){u32 desc[31]={124};pl_expected_rect=rect;pl_expected_flags=1;SetLastError(0x77);
        if(((i32 (WIN *)(void*,void*,void*,u32,HANDLE))table[25])(&pl_surface,rect,desc,1,0)!=13 || GetLastError()!=0x88)ExitProcess(263);}
    if(pl_locks!=5u+(os_mode_is("limit")?8:os_mode_is("held")?1:0) || pl_unlocks!=5u+(os_mode_is("limit")?8:0) ||
       os_blt_calls!=2u+(u32)os_mode_is("failed") || os_flip_calls!=2u+(u32)os_mode_is("failed") || os_query_calls!=(u32)os_mode_is("alias"))ExitProcess(264);
    CloseHandle(os_events);ExitProcess(0);
}
