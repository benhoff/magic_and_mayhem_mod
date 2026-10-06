/* Optional control recovery serializes observer callbacks without peer waits.
 * Exclusive admission is bounded. A timed-out callback forwards the original
 * method and records collision, so recovery cannot publish a usable checkpoint. */
#include "../../protocols/include/mnm/render_control_v1.h"
#define COMMAND_GATE_EXCLUSIVE 0x80000000u
static u32 command_control_configured,command_control_invalid;
static u32 command_gate,command_gate_collision;
static int command_observers_quiet(void);
struct CommandLease {u32 counted,observe;};
static struct CommandLease command_gate_enter(void){
    u32 error=GetLastError(),start=GetTickCount();struct CommandLease lease={0,1};
    if(!command_control_configured)goto done;
    for(;;){
        u32 state=__atomic_load_n(&command_gate,__ATOMIC_ACQUIRE);
        if((state&0x7fffffffu)==0x7fffffffu){lease.observe=0;goto done;}
        if(!(state&COMMAND_GATE_EXCLUSIVE)){
            if(__sync_bool_compare_and_swap(&command_gate,state,state+1)){lease.counted=1;break;}
        }else if(GetTickCount()-start>=MNM_RENDER_CONTROL_V1_CALLBACK_WAIT_MS){
            if(__sync_bool_compare_and_swap(&command_gate,state,state+1)){
                __atomic_store_n(&command_gate_collision,1,__ATOMIC_RELEASE);lease.counted=1;lease.observe=0;break;
            }
        }else Sleep(1);
    }
 done:SetLastError(error);return lease;
}
static void command_gate_leave(struct CommandLease* lease){
    u32 error=GetLastError();
    if(lease->counted)__atomic_fetch_sub(&command_gate,1,__ATOMIC_RELEASE);
    SetLastError(error);
}
#define COMMAND_CALLBACK(fallback) \
    struct CommandLease command_lease __attribute__((cleanup(command_gate_leave)))=command_gate_enter(); \
    if(!command_lease.observe)return (fallback)
static int command_gate_close(void){
    u32 start=GetTickCount();
    __atomic_store_n(&command_gate_collision,0,__ATOMIC_RELEASE);
    do{
        if(__sync_bool_compare_and_swap(&command_gate,0,COMMAND_GATE_EXCLUSIVE)){
            if(command_observers_quiet() && !__atomic_load_n(&command_gate_collision,__ATOMIC_ACQUIRE))return 1;
            __atomic_fetch_and(&command_gate,0x7fffffffu,__ATOMIC_RELEASE);
            if(__atomic_load_n(&command_gate_collision,__ATOMIC_ACQUIRE))return 0;
        }
        Sleep(1);
    }
    while(GetTickCount()-start<MNM_RENDER_CONTROL_V1_GATE_TIMEOUT_MS);
    return 0;
}
