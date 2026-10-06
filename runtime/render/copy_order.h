/* Intentional opt-in native scheduling, not recovered driver ordering.
 * Never hold the tracker while waiting or calling the original. Same-thread
 * callbacks keep generation guards; timeout forwards and invalidates both sides
 * of the unobserved call. Other mutation families retain generation checks. */
#define GAME_COPY_WAIT_MS 50u
static u32 game_copy_owner;
struct GameCopyLease {u32 held,observe;};
static struct GameCopyLease game_copy_enter(void* target){
    u32 error=GetLastError(),thread=GetCurrentThreadId(),start=GetTickCount(),waited=0;
    struct GameCopyLease lease={0,1};
    if(!game_copy_order_enabled)goto done;
    for(;;){
        if(__sync_bool_compare_and_swap(&game_copy_owner,0,thread)){lease.held=1;if(waited)lock_diagnostic("copy_order_wait_acquired",target,0,thread,GetTickCount()-start,0,0,0);break;}
        u32 owner=__atomic_load_n(&game_copy_owner,__ATOMIC_ACQUIRE);
        if(owner==thread){lock_diagnostic("copy_order_reentrant",target,0,owner,0,0,0,0);break;}
        if(GetTickCount()-start>=GAME_COPY_WAIT_MS){
            lease.observe=0;if(history_current())__atomic_store_n(&history_invalid,1,__ATOMIC_RELEASE);
            __atomic_add_fetch(&game_lock_epoch,1,__ATOMIC_RELAXED);game_pixel_missed(target);
            lock_diagnostic("copy_order_timeout",target,0,owner,GetTickCount()-start,0,0,0);break;
        }
        waited=1;Sleep(0);
    }
 done:SetLastError(error);return lease;
}
static void game_copy_leave(struct GameCopyLease* lease){
    if(lease->held)__atomic_store_n(&game_copy_owner,0,__ATOMIC_RELEASE);
}
static void game_copy_unobserved(void* target){
    u32 error=GetLastError();__atomic_add_fetch(&game_lock_epoch,1,__ATOMIC_RELAXED);
    game_pixel_missed(target);SetLastError(error);
}
