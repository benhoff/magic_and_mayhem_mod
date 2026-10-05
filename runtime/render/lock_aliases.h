/* Relationships come only from successful application QueryInterface calls. */
static void game_surface_alias(void* object);
struct GameAlias {void* from;void* to;};
#define GAME_ALIAS_COUNT 256u
static struct GameAlias game_aliases[GAME_ALIAS_COUNT];
static u32 game_alias_reset_pending;
/* Derived cache only: rebuild from observed edges after any graph mutation.
 * Hash lookup avoids traversing the alias graph for every surface candidate. */
struct GameAliasIndex {void* object;u32 parent;};
static struct GameAliasIndex game_alias_index[1024];
static u32 game_alias_index_valid;
static u32 game_alias_index_find(void* object,int create){
    if(!object)return 1024;
    u32 at=(((u32)(__SIZE_TYPE__)object>>2)*2654435761u)&1023;
    for(u32 i=0;i<1024;++i){struct GameAliasIndex* slot=game_alias_index+at;
        if(slot->object==object)return at;
        if(!slot->object){if(!create)return 1024;slot->object=object;slot->parent=at;return at;}
        at=(at+1)&1023;
    }
    return 1024;
}
static u32 game_alias_index_root(u32 at){
    u32 root=at;while(game_alias_index[root].parent!=root)root=game_alias_index[root].parent;
    while(at!=root){u32 next=game_alias_index[at].parent;game_alias_index[at].parent=root;at=next;}return root;
}
static void game_alias_index_build(void){
    zero(game_alias_index,sizeof(game_alias_index));
    for(u32 i=0;i<GAME_ALIAS_COUNT;++i)if(game_aliases[i].from){
        u32 a=game_alias_index_find(game_aliases[i].from,1),b=game_alias_index_find(game_aliases[i].to,1);
        if(a<1024 && b<1024)game_alias_index[game_alias_index_root(a)].parent=game_alias_index_root(b);
    }
    game_alias_index_valid=1;
}
/* A final Release missed under contention must not leave reusable alias tokens. */
static void game_alias_sync(void){
    if(__atomic_exchange_n(&game_alias_reset_pending,0,__ATOMIC_ACQ_REL)){zero(game_aliases,sizeof(game_aliases));game_alias_index_valid=0;}
}
static u32 game_alias_nodes(void* object,void** nodes){
    u32 count=1;nodes[0]=object;
    for(u32 at=0;at<count;++at)for(u32 i=0;i<GAME_ALIAS_COUNT;++i){
        void* next=game_aliases[i].from==nodes[at]?game_aliases[i].to:
                   game_aliases[i].to==nodes[at]?game_aliases[i].from:0;
        if(!next)continue;
        u32 found=0;for(u32 j=0;j<count;++j)if(nodes[j]==next){found=1;break;}
        if(!found && count<GAME_ALIAS_COUNT*2)nodes[count++]=next;
    }
    return count;
}
static int game_alias_same(void* first,void* second){
    if(first==second)return 1;
    if(!game_alias_index_valid)game_alias_index_build();
    u32 a=game_alias_index_find(first,0),b=game_alias_index_find(second,0);
    return a<1024 && b<1024 && game_alias_index_root(a)==game_alias_index_root(b);
}
static void game_alias_retire(void* object){
    void* nodes[GAME_ALIAS_COUNT*2];u32 count=game_alias_nodes(object,nodes);
    game_alias_index_valid=0;
    for(u32 i=0;i<GAME_ALIAS_COUNT;++i)for(u32 j=0;j<count;++j)
        if(game_aliases[i].from==nodes[j] || game_aliases[i].to==nodes[j]){zero(game_aliases+i,sizeof(game_aliases[i]));break;}
}
static void game_alias_observed(void* object,void* alias,u32 kind){
    if(!lock_capture_path_length || object==alias)return;
    if(!game_tracker_acquire()){game_metadata_invalidate();return;}
    game_alias_sync();
    if(!game_alias_same(object,alias)){
        struct GameAlias* free_slot=0;
        for(u32 i=0;i<GAME_ALIAS_COUNT;++i)if(!game_aliases[i].from){free_slot=game_aliases+i;break;}
        if(!free_slot){
            /* Saturation invalidates provenance, never invents a relationship. */
            game_metadata_invalidate();zero(game_aliases,sizeof(game_aliases));free_slot=game_aliases;
        }
        free_slot->from=object;free_slot->to=alias;
        game_alias_index_valid=0;
        u32 active=0;
        for(u32 i=0;i<32;++i)if(game_locks[i].active && game_locks[i].epoch==__atomic_load_n(&game_lock_epoch,__ATOMIC_RELAXED) && game_alias_same(object,game_locks[i].object))++active;
        if(active>1)__atomic_add_fetch(&game_lock_epoch,1,__ATOMIC_RELAXED);
        game_surface_alias(object);
        lock_diagnostic("alias_observed",object,kind,(u32)alias,0,0,0,0);
    }
    game_tracker_release();
}
