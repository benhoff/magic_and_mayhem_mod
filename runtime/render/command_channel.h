/* Opt-in append-only native command channel. Supported x86 mapped-file IPC.
 * Called only under the existing owned-session tracker; no peer wait or overwrite. */
#include "../../protocols/include/mnm/render_commands_v1.h"
API i32 WIN UnmapViewOfFile(const void*);
API i32 WIN GetFileInformationByHandle(HANDLE,void*);
#include "command_queue.h"
static u32* command_channel;
static u32 command_channel_session,command_channel_bytes,command_channel_refused;
/* Finite per-process recovery history. Never recycle an earlier mapped file,
 * including hard-link aliases, or a previously spent session identity. */
struct CommandFile {u32 volume,high,low,session;};
static struct CommandFile command_files[16];
static u32 command_file_count;
static int command_file_identity(HANDLE file,struct CommandFile* id){
    u32 info[13];if(!GetFileInformationByHandle(file,info) || info[8] || info[9]!=MNM_RENDER_COMMANDS_V2_SIZE || !(info[11]|info[12]))return 0;
    id->volume=info[7];id->high=info[11];id->low=info[12];id->session=0;return 1;
}
static int command_channel_identity(void){
    if(command_queue)return mnm_ring_identity(command_channel,command_channel_session);
    return command_channel && same(command_channel,MNM_RENDER_COMMANDS_V1_MAGIC,8) &&
        command_channel[2]==MNM_RENDER_COMMANDS_V1_VERSION && command_channel[3]==MNM_RENDER_COMMANDS_V1_SIZE &&
        command_channel[4]==command_channel_session;
}
static void command_channel_fail(u32 reason){
    if(command_queue){command_queue_refuse(reason);return;}
    if(!command_channel_identity())return;
    if(__atomic_load_n(command_channel+6,__ATOMIC_ACQUIRE)!=MNM_RENDER_COMMANDS_V1_STATE_WRITING)return;
    command_channel[7]=reason;
    __atomic_store_n(command_channel+6,MNM_RENDER_COMMANDS_V1_STATE_FAILED,__ATOMIC_RELEASE);
}
static int command_channel_append(const void* a,u32 an,const void* b,u32 bn,const void* c,u32 cn){
    if(command_channel_refused)return 0;
    if(!command_channel)return 1; /* Optional channel absent: existing file policy. */
    if(command_queue)return command_queue_append(a,an,b,bn,c,cn);
    if(!command_channel_identity() || __atomic_load_n(command_channel+6,__ATOMIC_ACQUIRE)!=MNM_RENDER_COMMANDS_V1_STATE_WRITING)return 0;
    if(__atomic_load_n(command_channel+8,__ATOMIC_ACQUIRE)){command_channel_fail(MNM_RENDER_COMMANDS_V1_REASON_CANCELLED);return 0;}
    u32 remaining=MNM_RENDER_COMMANDS_V1_CAPACITY-command_channel_bytes;
    if(an>remaining || bn>remaining-an || cn>remaining-an-bn){command_channel_fail(MNM_RENDER_COMMANDS_V1_REASON_OVERFLOW);return 0;}
    u8* to=(u8*)command_channel+MNM_RENDER_COMMANDS_V1_COMMANDS_OFFSET+command_channel_bytes;
    if(an)copy(to,a,an);
    if(bn)copy(to+an,b,bn);
    if(cn)copy(to+an+bn,c,cn);
    command_channel_bytes+=an+bn+cn;
    __atomic_store_n(command_channel+5,command_channel_bytes,__ATOMIC_RELEASE);return 1;
}
static int command_channel_record(u32 sequence,u32 op,const void* fields,u32 fl,const void* bytes,u32 bl){
    u32 record[3]={op,sequence,fl+bl};return command_channel_append(record,12,fields,fl,bytes,bl);
}
static void command_channel_end(void){
    if(command_queue){__atomic_store_n(&command_queue_end,1,__ATOMIC_RELEASE);return;}
    if(!command_channel_identity())return;
    if(__atomic_load_n(command_channel+6,__ATOMIC_ACQUIRE)==MNM_RENDER_COMMANDS_V1_STATE_WRITING)
        __atomic_store_n(command_channel+6,MNM_RENDER_COMMANDS_V1_STATE_ENDED,__ATOMIC_RELEASE);
}
static void command_channel_init(void){
    char path[512];u32 length=GetEnvironmentVariableA("MNM_RENDER_COMMAND_CHANNEL",path,sizeof(path));
    if(!length)return;
    if(length>=sizeof(path)){command_channel_refused=1;return;}
    /* A configured channel must not silently become optional after failure. */
    command_channel_refused=1;
    HANDLE file=CreateFileA(path,0xc0000000,3,0,3,0x80,0);
    if(file==(HANDLE)-1)return;
    u32 size=GetFileSize(file,0);
    if(size!=MNM_RENDER_COMMANDS_V1_SIZE && size!=MNM_RENDER_COMMANDS_V2_SIZE){CloseHandle(file);return;}
    struct CommandFile id;
    if(size==MNM_RENDER_COMMANDS_V2_SIZE && !command_file_identity(file,&id)){CloseHandle(file);return;}
    HANDLE mapping=CreateFileMappingA(file,0,4,0,size,0);CloseHandle(file);if(!mapping)return;
    u32* p=MapViewOfFile(mapping,2,0,0,size);CloseHandle(mapping);if(!p)return;
    if(size==MNM_RENDER_COMMANDS_V2_SIZE){
        if(!mnm_ring_writer_bind(&command_ring,p,size)){command_channel_refused=1;UnmapViewOfFile(p);return;}
        id.session=p[4];command_files[0]=id;command_file_count=1;
        command_queue=HeapAlloc(GetProcessHeap(),0,COMMAND_QUEUE_CAPACITY);
        if(!command_queue){command_channel_refused=1;mnm_ring_fail(&command_ring,MNM_RENDER_COMMANDS_V2_REASON_OVERFLOW);UnmapViewOfFile(p);return;}
        command_channel=p;command_channel_session=p[4];command_channel_refused=0;command_queue_configure();return;
    }
    int valid=same(p,MNM_RENDER_COMMANDS_V1_MAGIC,8) && p[2]==1 && p[3]==MNM_RENDER_COMMANDS_V1_SIZE && p[4] && !p[5] && !p[7] && !p[8];
    for(u32 i=9;i<16;++i)if(p[i])valid=0;
    if(!valid || !__sync_bool_compare_and_swap(p+6,MNM_RENDER_COMMANDS_V1_STATE_READY,MNM_RENDER_COMMANDS_V1_STATE_WRITING)){UnmapViewOfFile(p);return;}
    command_channel=p;command_channel_session=p[4];command_channel_bytes=0;command_channel_refused=0;
}
static void command_channel_close(void){
    if(command_queue){
        if(!__sync_bool_compare_and_swap(&command_queue_draining,0,1)){
            command_queue_refuse(MNM_RENDER_COMMANDS_V2_REASON_INTERRUPTED);return;
        }
        command_queue_drain();mnm_ring_fail(&command_ring,MNM_RENDER_COMMANDS_V2_REASON_INTERRUPTED);
        HeapFree(GetProcessHeap(),0,command_queue);command_queue=0;UnmapViewOfFile(command_channel);command_channel=0;
        __sync_lock_release(&command_queue_draining);return;
    }
    command_channel_fail(MNM_RENDER_COMMANDS_V1_REASON_INTERRUPTED);
    if(command_channel)UnmapViewOfFile(command_channel);
    command_channel=0;
}
