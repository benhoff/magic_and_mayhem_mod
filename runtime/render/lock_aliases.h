/* Relationships come only from successful application QueryInterface calls. */
struct GameAlias {void* from;void* to;};
static struct GameAlias game_aliases[64];
static u32 game_alias_reset_pending;
/* A final Release missed under contention must not leave reusable alias tokens. */
static void game_alias_sync(void){
    if(__atomic_exchange_n(&game_alias_reset_pending,0,__ATOMIC_ACQ_REL))zero(game_aliases,sizeof(game_aliases));
}
static u32 game_alias_nodes(void* object,void** nodes){
    u32 count=1;nodes[0]=object;
    for(u32 at=0;at<count;++at)for(u32 i=0;i<64;++i){
        void* next=game_aliases[i].from==nodes[at]?game_aliases[i].to:
                   game_aliases[i].to==nodes[at]?game_aliases[i].from:0;
        if(!next)continue;
        u32 found=0;for(u32 j=0;j<count;++j)if(nodes[j]==next){found=1;break;}
        if(!found && count<128)nodes[count++]=next;
    }
    return count;
}
static int game_alias_same(void* first,void* second){
    if(first==second)return 1;
    void* nodes[128];u32 count=game_alias_nodes(first,nodes);
    for(u32 i=0;i<count;++i)if(nodes[i]==second)return 1;return 0;
}
static void game_alias_retire(void* object){
    void* nodes[128];u32 count=game_alias_nodes(object,nodes);
    for(u32 i=0;i<64;++i)for(u32 j=0;j<count;++j)
        if(game_aliases[i].from==nodes[j] || game_aliases[i].to==nodes[j]){zero(game_aliases+i,sizeof(game_aliases[i]));break;}
}
static void game_alias_observed(void* object,void* alias,u32 kind){
    if(!lock_capture_path_length || object==alias)return;
    if(!__sync_bool_compare_and_swap(&game_locks_busy,0,1)){__atomic_add_fetch(&game_lock_epoch,1,__ATOMIC_RELAXED);return;}
    game_alias_sync();
    if(!game_alias_same(object,alias)){
        struct GameAlias* free_slot=0;
        for(u32 i=0;i<64;++i)if(!game_aliases[i].from){free_slot=game_aliases+i;break;}
        if(!free_slot){
            /* Saturation invalidates provenance, never invents a relationship. */
            __atomic_add_fetch(&game_lock_epoch,1,__ATOMIC_RELAXED);zero(game_aliases,sizeof(game_aliases));free_slot=game_aliases;
        }
        free_slot->from=object;free_slot->to=alias;
        u32 active=0;
        for(u32 i=0;i<32;++i)if(game_locks[i].active && game_locks[i].epoch==__atomic_load_n(&game_lock_epoch,__ATOMIC_RELAXED) && game_alias_same(object,game_locks[i].object))++active;
        if(active>1)__atomic_add_fetch(&game_lock_epoch,1,__ATOMIC_RELAXED);
        lock_diagnostic("alias_observed",object,kind,(u32)alias,0,0,0,0);
    }
    __sync_lock_release(&game_locks_busy);
}
