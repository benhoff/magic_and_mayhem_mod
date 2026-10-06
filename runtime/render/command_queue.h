/* Producer calls are serialized by the game tracker. Drains run only after
 * release, with their own nonblocking guard. Queue storage owns every byte. */
#include "../../protocols/include/mnm/render_command_ring.h"
#define COMMAND_QUEUE_CAPACITY (32u*1024u*1024u)
static struct mnm_ring_writer command_ring;
static u8* command_queue;
static u32 command_queue_written,command_queue_read,command_queue_end,command_queue_failure;
static volatile i32 command_queue_draining;
static u32 command_queue_peak,command_queue_full,command_queue_blocked;
/* Only the drain guard owns deadline state. No clock or peer wait in append. */
static u32 command_queue_timeout,command_queue_armed,command_queue_since,command_queue_ack,command_queue_timed_out;
static void command_queue_configure(void){
    char value[16];u32 n=GetEnvironmentVariableA("MNM_RENDER_CONTINUOUS",value,sizeof(value));
    if(n!=1 || value[0]!='1')return;
    command_queue_timeout=5000;
    n=GetEnvironmentVariableA("MNM_RENDER_STALL_TIMEOUT_MS",value,sizeof(value));
    if(!n || n>=sizeof(value))return;
    u32 timeout=0;for(u32 i=0;i<n;++i){if(value[i]<'0' || value[i]>'9' || timeout>5000)return;timeout=timeout*10+(u32)(value[i]-'0');}
    if(timeout>=10 && timeout<=5000)command_queue_timeout=timeout;
}
static void command_queue_peak_update(u32 pending){
    u32 old=__atomic_load_n(&command_queue_peak,__ATOMIC_RELAXED);
    while(pending>old && !__atomic_compare_exchange_n(&command_queue_peak,&old,pending,0,__ATOMIC_RELAXED,__ATOMIC_RELAXED)){}
}
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
    command_queue_peak_update(written-read);
    __atomic_store_n(&command_queue_written,written,__ATOMIC_RELEASE);return 1;
}
static int command_queue_watch(void){
    /* Validate even when no bytes remain queued: an idle producer still owns
     * cancellation, monotonic ACKs and the outstanding ring bytes. */
    if(mnm_ring_identity(command_ring.map,command_ring.session) && mnm_ring_load(command_ring.map+6)==2 &&
       __atomic_load_n(&command_queue_end,__ATOMIC_ACQUIRE) && command_queue_read==__atomic_load_n(&command_queue_written,__ATOMIC_ACQUIRE))return 0;
    if(!mnm_ring_writer_valid(&command_ring)){
        u32 reason=mnm_ring_identity(command_ring.map,command_ring.session) && mnm_ring_load(command_ring.map+6)==3?
            mnm_ring_load(command_ring.map+7):MNM_RENDER_COMMANDS_V2_REASON_INVALID;
        command_queue_refuse(reason?reason:MNM_RENDER_COMMANDS_V2_REASON_INVALID);return 0;
    }
    if(!command_queue_timeout)return 1;
    u32 now=GetTickCount(),ack=command_ring.acknowledged;
    int pending=command_queue_read!=__atomic_load_n(&command_queue_written,__ATOMIC_ACQUIRE) || command_ring.published!=ack;
    if(!pending){command_queue_armed=0;return 1;}
    if(!command_queue_armed || ack!=command_queue_ack){command_queue_armed=1;command_queue_since=now;command_queue_ack=ack;return 1;}
    if(now-command_queue_since>=command_queue_timeout){
        __atomic_store_n(&command_queue_timed_out,1,__ATOMIC_RELEASE);
        command_queue_refuse(MNM_RENDER_COMMANDS_V2_REASON_INTERRUPTED);
        mnm_ring_fail(&command_ring,MNM_RENDER_COMMANDS_V2_REASON_INTERRUPTED);return 0;
    }
    return 1;
}
static void command_queue_drain(void){
    u32 failure=__atomic_load_n(&command_queue_failure,__ATOMIC_ACQUIRE);
    if(failure){mnm_ring_fail(&command_ring,failure);return;}
    if(!command_queue_watch()){failure=__atomic_load_n(&command_queue_failure,__ATOMIC_ACQUIRE);if(failure)mnm_ring_fail(&command_ring,failure);return;}
    __atomic_store_n(&command_queue_blocked,0,__ATOMIC_RELEASE);
    u32 budget=1024u*1024u;
    while(budget){
        u32 read=command_queue_read,written=__atomic_load_n(&command_queue_written,__ATOMIC_ACQUIRE);
        if(read==written)break;
        u32 offset=read%COMMAND_QUEUE_CAPACITY,count=written-read;
        if(count>65536)count=65536;
        if(count>COMMAND_QUEUE_CAPACITY-offset)count=COMMAND_QUEUE_CAPACITY-offset;
        if(count>budget)count=budget;
        int result=mnm_ring_write(&command_ring,command_queue+offset,count);
        if(result<=0){
            if(result<0)command_queue_refuse(MNM_RENDER_COMMANDS_V2_REASON_INVALID);
            else {__atomic_store_n(&command_queue_blocked,1,__ATOMIC_RELEASE);
                if(command_queue_full!=0xffffffffu)__atomic_add_fetch(&command_queue_full,1,__ATOMIC_RELAXED);}
            return;
        }
        __atomic_store_n(&command_queue_read,read+count,__ATOMIC_RELEASE);budget-=count;
    }
    if(__atomic_load_n(&command_queue_end,__ATOMIC_ACQUIRE) && command_queue_read==__atomic_load_n(&command_queue_written,__ATOMIC_ACQUIRE))mnm_ring_end(&command_ring);
}
static void command_channel_pump(void){
    if(!__sync_bool_compare_and_swap(&command_queue_draining,0,1))return;
    u32 error=GetLastError();if(command_queue)command_queue_drain();SetLastError(error);__sync_lock_release(&command_queue_draining);
}
