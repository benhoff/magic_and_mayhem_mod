/* Actual production opaque ownership policy with a deterministic Win32 clock.
 * No callback or thread exit can invent an exact successful borrow return. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
typedef uint32_t u32;typedef int32_t i32;
static u32 tick,last_error=0x77,command_application_closed,missed;
static u32 GetLastError(void){return last_error;}
static void SetLastError(u32 v){last_error=v;}
static u32 GetTickCount(void){return tick;}
static void Sleep(u32 v){tick+=v;}
static void command_callback_missed(void){++missed;}
static int command_observers_quiet(void);
#include "../runtime/render/command_gate.h"
static int command_observers_quiet(void){return command_borrows_quiet();}
static void reset(void){memset(command_borrows,0,sizeof(command_borrows));command_borrow_uncertain=command_gate=command_gate_closing=tick=0;command_gate_enabled=1;}
int main(void){
 assert(!command_control_configured&&!command_control_invalid);
 reset();for(uintptr_t i=1;i<=64;++i)command_borrow_add((void*)i,1,0);
 assert(!command_borrow_uncertain&&!command_borrows_quiet());
 command_borrow_add((void*)65,1,0);assert(command_borrow_uncertain);
 for(uintptr_t i=1;i<=64;++i)assert(command_borrow_release((void*)i,1,0,0)==0);
 assert(!command_borrows_quiet()&&!command_gate_close()&&!command_gate&&last_error==0x77);
 puts("{\"case\":\"capacity-65-sticky\",\"success\":true,\"candidate_claimed\":false}");
 reset();command_borrow_add((void*)17,2,(void*)29);
 struct CommandLease callback=command_gate_enter();command_gate_leave(&callback);
 struct CommandLease observation=command_observation_enter();command_gate_leave(&observation);
 assert(!command_borrows_quiet()&&!command_gate_close()&&!command_gate);
 command_borrow_release((void*)18,2,(void*)29,0);command_borrow_release((void*)17,2,(void*)30,0);command_borrow_release((void*)17,2,(void*)29,-1);
 assert(!command_borrows_quiet()&&!command_gate_close()&&!command_gate);
 assert(command_borrow_release((void*)17,2,(void*)29,23)==23&&command_borrows_quiet());
 assert(command_gate_close()&&command_gate==COMMAND_GATE_EXCLUSIVE&&!missed&&last_error==0x77);
 puts("{\"case\":\"abandoned-and-failed-return\",\"success\":true,\"candidate_claimed_before_exact_return\":false,\"retry_after_exact_return\":true}");return 0;
}
