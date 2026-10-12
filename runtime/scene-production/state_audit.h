/* Owned bounded snapshots. Exact bytes establish equality; hashes are diagnostic.
 * Whole records do not follow nested heap pointers or establish map invariance. */
#define STATE_MAX (8u*1024u*1024u)
#define REGION_MAX 16u
struct StateRegion {u32 kind,base,bytes,offset,capacity,bound,manager;};
struct SceneState {u32 count,bytes;struct StateRegion regions[REGION_MAX];u8 data[STATE_MAX];};
static int state_region(struct SceneState* s,u32 kind,u32 base,u32 bytes,u32 capacity,u32 bound,u32 manager){
 if(s->count==REGION_MAX||bytes>STATE_MAX-s->bytes||(bytes&&!readable(base,bytes)))return 0;
 struct StateRegion* r=s->regions+s->count++;r->kind=kind;r->base=base;r->bytes=bytes;
 r->offset=s->bytes;r->capacity=capacity;r->bound=bound;r->manager=manager;
 copy(s->data+s->bytes,(void*)base,bytes);s->bytes+=bytes;return 1;
}
static int state_pool(struct SceneState* s,u32 kind,u32 manager,u32 bound_at,u32 stride,u32 maximum){
 if(!readable(manager,8)||!readable(bound_at,4))return 0;
 u32 base=get(manager),capacity=get(manager+4),bound=get(bound_at);
 if(!capacity)return !base&&!bound&&state_region(s,kind,0,0,0,0,manager);
 if(capacity>maximum||bound>capacity||!base||capacity>STATE_MAX/stride)return 0;
 return state_region(s,kind,base,capacity*stride,capacity,bound,manager);
}
static int state_capture(struct SceneState* s,u32 object){
 s->count=s->bytes=0;
 return state_region(s,1,0x5d8000,0x123b39,0,0,0)&&
        state_region(s,2,object,0x240,0,0,0)&&
        state_region(s,10,0x6c4830,4,0,0,0)&&
        state_region(s,11,0x6cbb78+0xfd5a,12,0,0,0)&&
        state_region(s,12,0x6dbe95,8,0,0,0)&&
        state_region(s,13,0x6e8100,4,0,0,0)&&
        state_region(s,14,0x6deffc,8*48,0,0,0)&&
        state_region(s,15,0x6df188,0x3c,0,0,0)&&
        state_region(s,16,0x6e1ac8,252*4,0,0,0)&&
        state_region(s,17,0x6cbb78,0x1031b,0,0,0)&&
        state_region(s,18,0x6c5490,0x66d2,0,0,0)&&
        state_region(s,19,object+0x229,16,0,0,0)&&
        state_pool(s,20,0x6def58,0x6df180,0xe4b,256)&&
        state_pool(s,21,0x689498,0x6898dc,0x22e,2048)&&
        state_pool(s,22,0x6b1248,0x6b1260,0x7e,4096);
}
static int state_shape(const struct SceneState* a,const struct SceneState* b){
 if(a->count!=b->count||a->bytes!=b->bytes)return 0;
 for(u32 i=0;i<a->count;++i)if(!same(a->regions+i,b->regions+i,sizeof(*a->regions)))return 0;
 return 1;
}
