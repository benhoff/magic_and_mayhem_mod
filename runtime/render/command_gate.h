/* Continuous direct/control recovery serializes observer callbacks.
 * Exclusive admission is bounded. A timed-out callback forwards the original
 * method and records collision, so recovery cannot publish a usable checkpoint. */
#include "../../protocols/include/mnm/render_control_v1.h"
#define COMMAND_GATE_EXCLUSIVE 0x80000000u
static u32 command_control_configured,command_control_invalid;
static u32 command_gate,command_gate_collision,command_gate_enabled;
static void command_callback_missed(void);
/* Opaque ownership for successful bypassed Lock/GetDC, independent of pixels
 * and tracker generations. Exact receiver/handle release only; uncertain aliases
 * or capacity exhaustion conservatively require process restart. */
struct CommandBorrow {u32 kind;void *object,*handle;};
static struct CommandBorrow command_borrows[64];
static u32 command_borrow_uncertain;
static void command_borrow_add(void* object,u32 kind,void* handle){
    u32 error=GetLastError();
    if(kind==2 && !handle)goto uncertain;
    for(u32 i=0;i<64;++i)if(__sync_bool_compare_and_swap(&command_borrows[i].kind,0,3)){
        __atomic_store_n(&command_borrows[i].object,object,__ATOMIC_RELAXED);__atomic_store_n(&command_borrows[i].handle,handle,__ATOMIC_RELAXED);
        __atomic_store_n(&command_borrows[i].kind,kind,__ATOMIC_RELEASE);SetLastError(error);return;
    }
 uncertain:__atomic_store_n(&command_borrow_uncertain,1,__ATOMIC_RELEASE);SetLastError(error);
}
static i32 command_borrow_release(void* object,u32 kind,void* handle,i32 result){
    if(result>=0)for(u32 i=0;i<64;++i){struct CommandBorrow* b=command_borrows+i;
        if(__atomic_load_n(&b->kind,__ATOMIC_ACQUIRE)==kind && __atomic_load_n(&b->object,__ATOMIC_RELAXED)==object && __atomic_load_n(&b->handle,__ATOMIC_RELAXED)==handle &&
           __sync_bool_compare_and_swap(&b->kind,kind,3)){__atomic_store_n(&b->object,0,__ATOMIC_RELAXED);__atomic_store_n(&b->handle,0,__ATOMIC_RELAXED);__atomic_store_n(&b->kind,0,__ATOMIC_RELEASE);break;}
    }
    return result;
}
static int command_borrows_quiet(void){
    if(__atomic_load_n(&command_borrow_uncertain,__ATOMIC_ACQUIRE))return 0;
    for(u32 i=0;i<64;++i)if(__atomic_load_n(&command_borrows[i].kind,__ATOMIC_ACQUIRE))return 0;
    return 1;
}
static int command_observers_quiet(void);
struct CommandLease {u32 counted,observe,retired;};
static struct CommandLease command_gate_enter(void){
    u32 error=GetLastError(),start=GetTickCount();struct CommandLease lease={0,1,0};
    if(__atomic_load_n(&command_application_closed,__ATOMIC_ACQUIRE)){lease.observe=0;lease.retired=1;goto done;}
    if(!command_gate_enabled)goto done;
    for(;;){
        if(__atomic_load_n(&command_application_closed,__ATOMIC_ACQUIRE)){lease.observe=0;lease.retired=1;goto done;}
        u32 state=__atomic_load_n(&command_gate,__ATOMIC_ACQUIRE);
        if((state&0x7fffffffu)==0x7fffffffu){lease.observe=0;goto done;}
        if(!(state&COMMAND_GATE_EXCLUSIVE)){
            if(__sync_bool_compare_and_swap(&command_gate,state,state+1)){lease.counted=1;if(__atomic_load_n(&command_application_closed,__ATOMIC_ACQUIRE)){lease.observe=0;lease.retired=1;}break;}
        }else if(GetTickCount()-start>=MNM_RENDER_CONTROL_V1_CALLBACK_WAIT_MS){
            if(__sync_bool_compare_and_swap(&command_gate,state,state+1)){
                __atomic_store_n(&command_gate_collision,1,__ATOMIC_RELEASE);lease.counted=1;lease.observe=0;break;
            }
        }else Sleep(1);
    }
 done:if(!lease.observe && !lease.retired)command_callback_missed();SetLastError(error);return lease;
}
static void command_gate_leave(struct CommandLease* lease){
    u32 error=GetLastError();
    if(!lease->observe && !lease->retired)command_callback_missed();
    if(lease->counted)__atomic_fetch_sub(&command_gate,1,__ATOMIC_RELEASE);
    SetLastError(error);
}
#define COMMAND_CALLBACK(fallback) \
    struct CommandLease command_lease __attribute__((cleanup(command_gate_leave)))=command_gate_enter(); \
    if(!command_lease.observe)return (fallback)
/* Direct recovery and the administrative worker share this single owner.
 * Contending closers must not clear another owner's collision evidence. */
static u32 command_gate_closing;
static int command_gate_close(void){
    if(!__sync_bool_compare_and_swap(&command_gate_closing,0,1))return 0;
    u32 start=GetTickCount();
    if(__atomic_load_n(&command_gate,__ATOMIC_ACQUIRE)&COMMAND_GATE_EXCLUSIVE){__sync_lock_release(&command_gate_closing);return 0;}
    __atomic_store_n(&command_gate_collision,0,__ATOMIC_RELEASE);
    do{
        if(__sync_bool_compare_and_swap(&command_gate,0,COMMAND_GATE_EXCLUSIVE)){
            if(command_observers_quiet() && !__atomic_load_n(&command_gate_collision,__ATOMIC_ACQUIRE)){__sync_lock_release(&command_gate_closing);return 1;}
            __atomic_fetch_and(&command_gate,0x7fffffffu,__ATOMIC_RELEASE);
            if(__atomic_load_n(&command_gate_collision,__ATOMIC_ACQUIRE)){__sync_lock_release(&command_gate_closing);return 0;}
        }
        Sleep(1);
    }
    while(GetTickCount()-start<MNM_RENDER_CONTROL_V1_GATE_TIMEOUT_MS);
    __sync_lock_release(&command_gate_closing);return 0;
}
