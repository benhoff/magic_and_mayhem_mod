/* Optional native-owned diagnostic getter, resolved once during adapter load. */
static u32 (WIN *game_copy_native_stage)(void);
static void (WIN *game_copy_native_defer)(u32);
/* Intentional opt-in native scheduling, not recovered driver ordering.
 * Never hold the tracker while waiting or calling the original. Same-thread
 * callbacks keep generation guards; timeout forwards and invalidates both sides
 * of the unobserved call. Other mutation families retain generation checks. */
#define GAME_COPY_WAIT_MS 50u
enum { GAME_COPY_BIND=0, GAME_COPY_MUTATION=1, GAME_COPY_OBSERVATION=2 };
static u32 game_copy_owner,game_copy_started,game_copy_site,game_copy_phase;
static struct GameCopyLease game_copy_enter_policy(void* target,u32 site,u32 original_call){
    u32 error=GetLastError(),thread=GetCurrentThreadId(),start=GetTickCount(),waited=0;
    struct GameCopyLease lease={0,1,0};
    struct CopyWaitTiming timing;copy_wait_begin(&timing,start);
    if(!game_copy_order_enabled)goto done;
    for(;;){
        if(__sync_bool_compare_and_swap(&game_copy_owner,0,thread)){lease.held=1;__atomic_store_n(&game_copy_started,GetTickCount(),__ATOMIC_RELAXED);__atomic_store_n(&game_copy_site,site,__ATOMIC_RELAXED);__atomic_store_n(&game_copy_phase,0,__ATOMIC_RELAXED);copy_trace_owner_open(target,site,thread,__atomic_load_n(&game_copy_started,__ATOMIC_RELAXED));if(waited)lock_diagnostic("copy_order_wait_acquired",target,0,thread,GetTickCount()-start,0,0,0);break;}
        u32 owner=__atomic_load_n(&game_copy_owner,__ATOMIC_ACQUIRE);
        copy_wait_sample(&timing,&game_copy_owner);
        if(owner==thread){
            if(!original_call)lease.observe=0;
            lock_diagnostic(original_call?"copy_order_reentrant":"resident_bind_reentrant",target,0,owner,0,0,0,0);break;
        }
        u32 budget=__atomic_load_n(&game_cpu_transfer_wait,__ATOMIC_ACQUIRE)?10000u+GAME_COPY_WAIT_MS:GAME_COPY_WAIT_MS;
        if(GetTickCount()-start>=budget){
            lease.observe=0;
            copy_wait_timeout(&timing,&game_copy_owner,command_channel_session,target,site);
            /* Binding has no original fallback or unobserved pixel mutation.
             * Refuse admission without inventing a missing drawing operation. */
            if(original_call==GAME_COPY_MUTATION){
                if(history_current())__atomic_store_n(&history_invalid,1,__ATOMIC_RELEASE);
                __atomic_add_fetch(&game_lock_epoch,1,__ATOMIC_RELAXED);game_pixel_missed(target);
            }
            u32 values[19]={(u32)target,thread,owner,GetTickCount()-start,GetTickCount()-__atomic_load_n(&game_copy_started,__ATOMIC_RELAXED),__atomic_load_n(&game_copy_site,__ATOMIC_RELAXED),site,__atomic_load_n(&game_copy_phase,__ATOMIC_RELAXED),game_copy_native_stage?game_copy_native_stage():0};
            lock_diagnostic_values(original_call==GAME_COPY_OBSERVATION?"copy_observation_timeout":original_call?"copy_order_timeout":"resident_bind_admission_timeout",values);break;
        }
        /* Wait only while this observed owner remains unchanged. Release wakes
         * waiters without relying on Sleep timer granularity; no lost wakeup. */
        u32 elapsed=GetTickCount()-start;
        if(elapsed<budget){waited=1;u32 before=copy_trace_enabled?GetTickCount():0;copy_wait_before(&timing);
            u32 returned=WaitOnAddress(&game_copy_owner,&owner,4,budget-elapsed);copy_wait_after(&timing,before,returned);}
    }
 done:if(lease.held && game_copy_native_defer){game_copy_native_defer(1);lease.deferred=1;}
    SetLastError(error);return lease;
}
static struct GameCopyLease game_copy_enter_at(void* target,u32 site){return game_copy_enter_policy(target,site,1);}
static struct GameCopyLease game_copy_observation_enter_at(void* target,u32 site){return game_copy_enter_policy(target,site,GAME_COPY_OBSERVATION);}
static struct GameCopyLease game_copy_bind_enter(void){return game_copy_enter_policy(0,0x80000000u,0);}
static void game_copy_stage(u32 phase){
    /* Tracker release also runs outside an ordered callback. Such a caller
     * must not overwrite the actual copy owner's diagnostic phase. */
    if(__atomic_load_n(&game_copy_owner,__ATOMIC_ACQUIRE)==GetCurrentThreadId()){
        __atomic_store_n(&game_copy_phase,phase,__ATOMIC_RELAXED);copy_trace_owner_phase(phase);
    }
}
#define game_copy_enter(target) game_copy_enter_at((target),__LINE__)
#define game_copy_observation_enter(target) game_copy_observation_enter_at((target),__LINE__)
static void game_copy_leave(struct GameCopyLease* lease){
    if(lease->held){u32 error=GetLastError(),flush=lease->deferred;
        copy_trace_owner_close();
        lease->held=lease->deferred=0;
        __atomic_store_n(&game_copy_owner,0,__ATOMIC_RELEASE);WakeByAddressAll(&game_copy_owner);
        if(flush && game_copy_native_defer)game_copy_native_defer(0);
        SetLastError(error);
    }
}
static void game_copy_unobserved(void* target){
    u32 error=GetLastError();__atomic_add_fetch(&game_lock_epoch,1,__ATOMIC_RELAXED);
    game_pixel_missed(target);SetLastError(error);
}

/* A missed lifetime boundary can reuse any alias token, not only its receiver.
 * Reset the derived graph at the next tracker entry without dereferencing COM. */
static void game_lifetime_invalidate(void){
    __atomic_store_n(&game_alias_reset_pending,1,__ATOMIC_RELEASE);
    game_metadata_invalidate();
}

/* Shutdown refusal must leave an active stream intact for a later retry.
 * Unlike a missed drawing callback, this does not forward an unobserved write. */
static struct GameCopyLease game_copy_shutdown_enter(void){
    u32 error=GetLastError(),thread=GetCurrentThreadId(),start=GetTickCount();struct GameCopyLease lease={0,1,0};
    if(!game_copy_order_enabled)goto done;
    for(;;){
        if(__sync_bool_compare_and_swap(&game_copy_owner,0,thread)){lease.held=1;break;}
        if(__atomic_load_n(&game_copy_owner,__ATOMIC_ACQUIRE)==thread){lease.observe=0;lock_diagnostic("command_shutdown_reentrant",0,0,thread,0,0,0,0);break;}
        u32 budget=__atomic_load_n(&game_cpu_transfer_wait,__ATOMIC_ACQUIRE)?10000u+GAME_COPY_WAIT_MS:GAME_COPY_WAIT_MS;
        if(GetTickCount()-start>=budget){lease.observe=0;lock_diagnostic("command_shutdown_admission_timeout",0,0,thread,0,0,0,0);break;}
        Sleep(0);
    }
 done:SetLastError(error);return lease;
}

/* A gate timeout bypasses any callback family, including lifetime/property
 * changes. Preserve no inferred identity or pixels across that original. */
static void command_callback_missed(void){
    u32 error=GetLastError();game_lifetime_invalidate();game_copy_unobserved(0);SetLastError(error);
}
