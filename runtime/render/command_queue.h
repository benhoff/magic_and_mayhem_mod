/* Producer calls are serialized by the game tracker. Drains run only after
 * release, with their own nonblocking guard. Queue storage owns every byte. */
#include "../../protocols/include/mnm/render_command_ring.h"
#define COMMAND_QUEUE_CAPACITY (32u*1024u*1024u)
static struct mnm_ring_writer command_ring;
static u8* command_queue;
static u32 command_queue_written,command_queue_read,command_queue_end,command_queue_failure;
static volatile i32 command_queue_draining;
static void command_queue_refuse(u32 reason){
    u32 empty=0;__atomic_compare_exchange_n(&command_queue_failure,&empty,reason,0,__ATOMIC_RELEASE,__ATOMIC_RELAXED);
}
static int command_queue_append(const void* a,u32 an,const void* b,u32 bn,const void* c,u32 cn){
    if(__atomic_load_n(&command_queue_failure,__ATOMIC_ACQUIRE) || __atomic_load_n(&command_queue_end,__ATOMIC_ACQUIRE))return 0;
    if(!mnm_ring_identity(command_ring.map,command_ring.session) || mnm_ring_load(command_ring.map+6)!=1){
        command_queue_refuse(MNM_RENDER_COMMANDS_V2_REASON_INVALID);return 0;
    }
    if(mnm_ring_load(command_ring.map+8)){
        command_queue_refuse(MNM_RENDER_COMMANDS_V2_REASON_CANCELLED);return 0;
    }
    u32 read=__atomic_load_n(&command_queue_read,__ATOMIC_ACQUIRE),written=command_queue_written;
    u32 remaining=COMMAND_QUEUE_CAPACITY-(written-read);
    if(an>remaining || bn>remaining-an || cn>remaining-an-bn || an+bn+cn>0xffffffffu-written){
        command_queue_refuse(MNM_RENDER_COMMANDS_V2_REASON_OVERFLOW);return 0;
    }
    const void* parts[3]={a,b,c};u32 sizes[3]={an,bn,cn};
    for(u32 i=0;i<3;++i)if(sizes[i]){
        u32 offset=written%COMMAND_QUEUE_CAPACITY,first=COMMAND_QUEUE_CAPACITY-offset;if(first>sizes[i])first=sizes[i];
        copy(command_queue+offset,parts[i],first);copy(command_queue,(const u8*)parts[i]+first,sizes[i]-first);written+=sizes[i];
    }
    __atomic_store_n(&command_queue_written,written,__ATOMIC_RELEASE);return 1;
}
static void command_queue_drain(void){
    u32 failure=__atomic_load_n(&command_queue_failure,__ATOMIC_ACQUIRE);
    if(failure){mnm_ring_fail(&command_ring,failure);return;}
    u32 budget=1024u*1024u;
    while(budget){
        u32 read=command_queue_read,written=__atomic_load_n(&command_queue_written,__ATOMIC_ACQUIRE);
        if(read==written)break;
        u32 offset=read%COMMAND_QUEUE_CAPACITY,count=written-read;
        if(count>65536)count=65536;
        if(count>COMMAND_QUEUE_CAPACITY-offset)count=COMMAND_QUEUE_CAPACITY-offset;
        if(count>budget)count=budget;
        int result=mnm_ring_write(&command_ring,command_queue+offset,count);
        if(result<=0){if(result<0)command_queue_refuse(MNM_RENDER_COMMANDS_V2_REASON_INVALID);return;}
        __atomic_store_n(&command_queue_read,read+count,__ATOMIC_RELEASE);budget-=count;
    }
    if(__atomic_load_n(&command_queue_end,__ATOMIC_ACQUIRE) && command_queue_read==__atomic_load_n(&command_queue_written,__ATOMIC_ACQUIRE))mnm_ring_end(&command_ring);
}
static void command_channel_pump(void){
    if(!command_queue || !__sync_bool_compare_and_swap(&command_queue_draining,0,1))return;
    u32 error=GetLastError();command_queue_drain();SetLastError(error);__sync_lock_release(&command_queue_draining);
}
