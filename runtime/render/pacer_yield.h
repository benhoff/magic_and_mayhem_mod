/* Native session policy: cooperate only inside the guarded original wait.
 * Forward the original clock; never synthesize timestamps or change its target. */
typedef u32 (WIN *PacerTickFn)(void);
static PacerTickFn pacer_original_tick;
static u32 pacer_enabled,pacer_yield_seen;
static int pacer_entry_matches(const u8* load,const u8* loop,u32 current,u32 expected,u32 base){
    const u8 load_bytes[6]={0x8b,0x35,0x64,0x51,0x5c,0};
    const u8 loop_bytes[12]={0xff,0xd6,0x2b,0x05,0x00,0x81,0x6e,0x00,0x3b,0xc7,0x72,0xf4};
    return base==0x400000&&expected&&current==expected&&same(load,load_bytes,6)&&same(loop,loop_bytes,12);
}
static u32 pacer_read(u32 caller,int active){
    u32 error=GetLastError();
    if(pacer_enabled&&active&&caller==0x4e3f8b){
        if(!pacer_yield_seen){pacer_yield_seen=1;PACER_FIRST_YIELD(caller);}
        Sleep(0);
    }
    SetLastError(error);return pacer_original_tick();
}
#ifndef MNM_PACER_POLICY_TEST
static u32 WIN pacer_tick(void){
    /* Lifetime flags are process-owned atomics; do not dereference mappings
     * concurrently unmapped during native cancellation/recovery. */
    int active=!__atomic_load_n(&command_application_closed,__ATOMIC_ACQUIRE)&&
        !__atomic_load_n(&command_channel_refused,__ATOMIC_ACQUIRE)&&
        !__atomic_load_n(&command_queue_failure,__ATOMIC_ACQUIRE)&&
        !__atomic_load_n(&command_queue_end,__ATOMIC_ACQUIRE)&&
        !__atomic_load_n(&command_worker_stop,__ATOMIC_ACQUIRE);
    return pacer_read((u32)__builtin_return_address(0),active);
}
static int pacer_install(u32 base){
    char option[8];u32 n=GetEnvironmentVariableA("MNM_RENDER_PACER_YIELD",option,sizeof(option));
    if(!n)return 1;
    if(n!=1||option[0]!='1'||!command_channel||!command_queue||command_channel_refused)return 0;
    u32* slot=(u32*)0x5c5164;void* kernel=GetModuleHandleA("kernel32.dll");
    PacerTickFn original=(PacerTickFn)GetProcAddress(kernel,"GetTickCount");
    if(!readable((void*)0x4e3e5b,6)||!readable((void*)0x4e3f89,12)||!readable(slot,4)||
        !pacer_entry_matches((void*)0x4e3e5b,(void*)0x4e3f89,*slot,(u32)original,base))return 0;
    u32 protection,ignored;
    if(!VirtualProtect(slot,4,4,&protection))return 0;
    pacer_original_tick=original;pacer_enabled=1;
    __atomic_store_n(slot,(u32)&pacer_tick,__ATOMIC_RELEASE);
    VirtualProtect(slot,4,protection,&ignored);return 1;
}
#endif
