/* Shared independently generated workloads, not shared pool implementation. */
#define POOL_CAPACITY 1792
#define POOL_NODES 8192
#define POOL_EVENTS 16384
struct PoolNode {u32 address,alive; i32 score;u32 age;};
static struct PoolNode nodes[POOL_NODES];
static u32 events[POOL_EVENTS*3],event_count,next_node,case_id,total_frees;
static u32 initial_count,pattern,spawn_remaining,life_mode,spawns_requested;
static i32 offered_priority,initial_hint;
static void fixture_require(int ok,u32 code);
static void fixture_write(const void* data,u32 bytes);
static u32 pool_insert(u32 token);
static void pool_update(void);
static void pool_clear(void);
static u32 pool_entry(u32 index);
static i32 pool_count(void);
static i32 pool_hint(void);
static void pool_initialize(u32 count,i32 hint);
static void pool_allocate(u32 token);
static void pool_validate_object(u32 token);
static void pool_increment_age(u32 token);
static void pool_free_unowned(u32 token);

static void fixture_event(u32 kind,u32 token,u32 value){
    fixture_require(event_count<POOL_EVENTS,201);
    events[event_count*3]=kind;events[event_count*3+1]=token;events[event_count*3+2]=value;++event_count;
}
static u32 fixture_new(i32 priority){
    fixture_require(next_node+1<POOL_NODES,202);u32 token=++next_node;
    nodes[token].alive=1;nodes[token].score=priority;nodes[token].age=0;
    pool_allocate(token);return token;
}
static i32 fixture_priority(u32 token){
    fixture_require(token&&token<=next_node&&nodes[token].alive,203);pool_validate_object(token);
    fixture_event(1,token,(u32)nodes[token].score);return nodes[token].score;
}
static void fixture_destroyed(u32 token){
    fixture_require(token&&token<=next_node&&nodes[token].alive,204);
    fixture_event(3,token,1);nodes[token].alive=0;++total_frees;
}
static u32 fixture_update(u32 token){
    fixture_require(token&&token<=next_node&&nodes[token].alive,205);pool_validate_object(token);
    ++nodes[token].age;pool_increment_age(token);
    u32 alive=life_mode==1?1:life_mode==2?token%2:life_mode==3?token%3!=0:0;
    fixture_event(2,token,alive);
    while(spawn_remaining){
        --spawn_remaining;u32 child=fixture_new(offered_priority);
        u32 adopted=pool_insert(child);fixture_event(4,child,adopted);
    }
    return alive;
}
static void fixture_snapshot(u32 stage){
    u32 live=0;for(u32 i=1;i<=next_node;++i)if(nodes[i].alive){++live;pool_validate_object(i);}
    u32 header[14]={case_id,stage,initial_count,pattern,(u32)offered_priority,(u32)initial_hint,
        spawns_requested,life_mode,(u32)pool_count(),(u32)pool_hint(),live,next_node,event_count,total_frees};
    fixture_write(header,sizeof(header));
    static u32 identities[POOL_CAPACITY];
    for(u32 i=0;i<POOL_CAPACITY;++i)identities[i]=pool_entry(i);
    fixture_write(identities,sizeof(identities));fixture_write(events,event_count*12);event_count=0;
}
static void fixture_run(void){
    const u32 counts[5]={0,1,3,1791,1792};const i32 priorities[5]={0,1024,1025,2000,-1};const i32 hints[3]={0,1024,4096};
    for(u32 n=0;n<5;++n)for(pattern=0;pattern<4;++pattern)for(u32 p=0;p<5;++p)
    for(u32 h=0;h<3;++h)for(spawns_requested=0;spawns_requested<3;++spawns_requested)for(life_mode=0;life_mode<4;++life_mode){
        for(u32 i=0;i<POOL_NODES;++i){nodes[i].alive=0;nodes[i].address=0;}
        initial_count=counts[n];offered_priority=priorities[p];initial_hint=hints[h];
        event_count=next_node=total_frees=0;spawn_remaining=spawns_requested;
        for(u32 i=0;i<initial_count;++i){
            i32 score=pattern==0?0:pattern==1?1024:pattern==2?4096:(i32)(i%7)*500-1000;
            fixture_new(score);
        }
        pool_initialize(initial_count,initial_hint);
        u32 incoming=fixture_new(offered_priority);u32 adopted=pool_insert(incoming);fixture_event(5,incoming,adopted);
        fixture_snapshot(0);pool_update();fixture_snapshot(1);pool_clear();fixture_snapshot(2);
        fixture_require(pool_count()==0&&pool_hint()==0,206);
        // Deliberately observe failed-insertion owners before separate caller
        // cleanup. Clearing the manager must not hide stranded allocations.
        for(u32 i=1;i<=next_node;++i)if(nodes[i].alive)pool_free_unowned(i);
        fixture_require(total_frees==next_node,207);++case_id;
    }
    fixture_require(case_id==3600,208);
}
