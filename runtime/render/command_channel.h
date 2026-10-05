/* Opt-in append-only native command channel. Supported x86 mapped-file IPC.
 * Called only under the existing owned-session tracker; no peer wait or overwrite. */
#include "../../protocols/include/mnm/render_commands_v1.h"
API i32 WIN UnmapViewOfFile(const void*);
static u32* command_channel;
static u32 command_channel_session,command_channel_bytes;
static int command_channel_identity(void){
    return command_channel && same(command_channel,MNM_RENDER_COMMANDS_V1_MAGIC,8) &&
        command_channel[2]==MNM_RENDER_COMMANDS_V1_VERSION && command_channel[3]==MNM_RENDER_COMMANDS_V1_SIZE &&
        command_channel[4]==command_channel_session;
}
static void command_channel_fail(u32 reason){
    if(!command_channel_identity())return;
    if(__atomic_load_n(command_channel+6,__ATOMIC_ACQUIRE)!=MNM_RENDER_COMMANDS_V1_STATE_WRITING)return;
    command_channel[7]=reason;
    __atomic_store_n(command_channel+6,MNM_RENDER_COMMANDS_V1_STATE_FAILED,__ATOMIC_RELEASE);
}
static int command_channel_append(const void* a,u32 an,const void* b,u32 bn,const void* c,u32 cn){
    if(!command_channel)return 1; /* Optional channel absent: existing file policy. */
    if(!command_channel_identity() || __atomic_load_n(command_channel+6,__ATOMIC_ACQUIRE)!=MNM_RENDER_COMMANDS_V1_STATE_WRITING)return 0;
    if(__atomic_load_n(command_channel+8,__ATOMIC_ACQUIRE)){command_channel_fail(MNM_RENDER_COMMANDS_V1_REASON_CANCELLED);return 0;}
    u32 remaining=MNM_RENDER_COMMANDS_V1_CAPACITY-command_channel_bytes;
    if(an>remaining || bn>remaining-an || cn>remaining-an-bn){command_channel_fail(MNM_RENDER_COMMANDS_V1_REASON_OVERFLOW);return 0;}
    u8* to=(u8*)command_channel+MNM_RENDER_COMMANDS_V1_COMMANDS_OFFSET+command_channel_bytes;
    if(an)copy(to,a,an);if(bn)copy(to+an,b,bn);if(cn)copy(to+an+bn,c,cn);
    command_channel_bytes+=an+bn+cn;
    __atomic_store_n(command_channel+5,command_channel_bytes,__ATOMIC_RELEASE);return 1;
}
static int command_channel_record(u32 sequence,u32 op,const void* fields,u32 fl,const void* bytes,u32 bl){
    u32 record[3]={op,sequence,fl+bl};return command_channel_append(record,12,fields,fl,bytes,bl);
}
static void command_channel_end(void){
    if(!command_channel_identity())return;
    if(__atomic_load_n(command_channel+6,__ATOMIC_ACQUIRE)==MNM_RENDER_COMMANDS_V1_STATE_WRITING)
        __atomic_store_n(command_channel+6,MNM_RENDER_COMMANDS_V1_STATE_ENDED,__ATOMIC_RELEASE);
}
static void command_channel_init(void){
    char path[512];u32 length=GetEnvironmentVariableA("MNM_RENDER_COMMAND_CHANNEL",path,sizeof(path));
    if(!length || length>=sizeof(path))return;
    HANDLE file=CreateFileA(path,0xc0000000,3,0,3,0x80,0);
    if(file==(HANDLE)-1)return;
    if(GetFileSize(file,0)!=MNM_RENDER_COMMANDS_V1_SIZE){CloseHandle(file);return;}
    HANDLE mapping=CreateFileMappingA(file,0,4,0,MNM_RENDER_COMMANDS_V1_SIZE,0);CloseHandle(file);if(!mapping)return;
    u32* p=MapViewOfFile(mapping,2,0,0,MNM_RENDER_COMMANDS_V1_SIZE);CloseHandle(mapping);if(!p)return;
    int valid=same(p,MNM_RENDER_COMMANDS_V1_MAGIC,8) && p[2]==1 && p[3]==MNM_RENDER_COMMANDS_V1_SIZE && p[4] && !p[5] && !p[7] && !p[8];
    for(u32 i=9;i<16;++i)if(p[i])valid=0;
    if(!valid || !__sync_bool_compare_and_swap(p+6,MNM_RENDER_COMMANDS_V1_STATE_READY,MNM_RENDER_COMMANDS_V1_STATE_WRITING)){UnmapViewOfFile(p);return;}
    command_channel=p;command_channel_session=p[4];command_channel_bytes=0;
}
static void command_channel_close(void){
    command_channel_fail(MNM_RENDER_COMMANDS_V1_REASON_INTERRUPTED);
    if(command_channel)UnmapViewOfFile(command_channel);command_channel=0;
}
