/* Resident callbacks pace before copy/tracker ownership, never in append.
 * Read private counters only: the publication worker owns mapped peer access. */
#define COMMAND_PRESSURE_HIGH (8u*1024u*1024u)
#define COMMAND_PRESSURE_LOW (4u*1024u*1024u)
#define COMMAND_PRESSURE_WAIT_MS 5000u
static u32 command_pressure_waits,command_pressure_resumes,command_pressure_max_ms;
static u32 command_pressure_pending(void){
    u32 read=__atomic_load_n(&command_queue_read,__ATOMIC_ACQUIRE);
    return __atomic_load_n(&command_queue_written,__ATOMIC_ACQUIRE)-read;
}
static int command_pressure_admit(u32 wait,u32 started){
    if(!game_resident_world || !game_copy_order_enabled || !command_queue ||
       !__atomic_load_n(&command_publication_worker_active,__ATOMIC_ACQUIRE))return 1;
    if(__atomic_load_n(&command_queue_failure,__ATOMIC_ACQUIRE) || __atomic_load_n(&command_queue_end,__ATOMIC_ACQUIRE))return 0;
    u32 pending=command_pressure_pending();
    if(!wait)return pending<COMMAND_PRESSURE_HIGH;
    __atomic_add_fetch(&command_pressure_waits,1,__ATOMIC_RELAXED);
    u32 began=GetTickCount();
    lock_diagnostic("command_pressure_begin",0,0,pending,COMMAND_PRESSURE_HIGH,0,COMMAND_PRESSURE_LOW,0);
    while(pending>COMMAND_PRESSURE_LOW){
        if(__atomic_load_n(&command_queue_failure,__ATOMIC_ACQUIRE) || __atomic_load_n(&command_queue_end,__ATOMIC_ACQUIRE))return 0;
        if(GetTickCount()-started>=COMMAND_PRESSURE_WAIT_MS){
            command_queue_refuse(MNM_RENDER_COMMANDS_V2_REASON_INTERRUPTED);
            lock_diagnostic("command_pressure_timeout",0,0,pending,GetTickCount()-started,0,0,0);return 0;
        }
        Sleep(1);pending=command_pressure_pending();
    }
    if(__atomic_load_n(&command_queue_failure,__ATOMIC_ACQUIRE) || __atomic_load_n(&command_queue_end,__ATOMIC_ACQUIRE))return 0;
    u32 elapsed=GetTickCount()-began,old=__atomic_load_n(&command_pressure_max_ms,__ATOMIC_RELAXED);
    while(elapsed>old && !__atomic_compare_exchange_n(&command_pressure_max_ms,&old,elapsed,0,__ATOMIC_RELAXED,__ATOMIC_RELAXED)){}
    __atomic_add_fetch(&command_pressure_resumes,1,__ATOMIC_RELAXED);
    lock_diagnostic("command_pressure_resume",0,0,pending,elapsed,0,0,0);return 1;
}
