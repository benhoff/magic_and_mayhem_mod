/* Private provider pins are independent of COM reference counts. A queued
 * clone stores interface identity, never a caller's mutable wrapper address.
 * Interface headers remain bounded tombstones until process teardown so an old
 * alias cannot accidentally designate a new allocation. Retired backing is freed.
 * Valid Lock/DC leases retain backing after the last public Release. */
struct NsLifetimePin {u32 identity;struct NsState* state;};
static struct NsLifetimePin ns_lifetime_pins[512];
static u32 ns_lifetime_next_pin,ns_lifetime_retired_count;
static void* ns_lifetime_retired[32768];
static struct NsState* ns_lifetime_find(const void* object){
 for(struct NsState* s=ns_owned;s;s=s->next_owned)for(u32 i=0;i<5;++i)if(object==&s->aliases[i])return s;
 return 0;
}
static int ns_lifetime_valid(const void* object){return object && ns_lifetime_find(object)!=0;}
static void ns_lifetime_collect(void){
 for(;;){struct NsState* victim=0;
  for(struct NsState* s=ns_owned;s;s=s->next_owned)if(!s->interfaces && !s->lifetime_pins && !ns_borrowed(s)){victim=s;break;}
  if(!victim)break;
  if(ns_lifetime_retired_count>=32768)ExitProcess(123);
  ns_lifetime_retired[ns_lifetime_retired_count++]=&victim->aliases[1];ns_destroy(victim);
 }
}
__declspec(dllexport) i32 WIN NativeDrawPinV1(void* object,u32* identity){
 struct NsLifeGuard guard __attribute__((cleanup(ns_life_guard_leave)))=ns_life_guard_enter();
 if(!ns_lifetime_mode || !identity)return ND_INVALID;
 struct NsState* s=ns_lifetime_find(object);if(!s || (!s->interfaces && !s->lifetime_pins && !ns_borrowed(s)))return ND_INVALID;
 for(u32 i=0;i<512;++i)if(!ns_lifetime_pins[i].identity){
  if(ns_lifetime_next_pin==0xffffffffu)return (i32)0x8007000eu;
  u32 token=++ns_lifetime_next_pin;ns_lifetime_pins[i]=(struct NsLifetimePin){token,s};++s->lifetime_pins;*identity=token;return 0;
 }
 return (i32)0x8007000eu;
}
__declspec(dllexport) i32 WIN NativeDrawUnpinV1(u32 identity){
 struct NsLifeGuard guard __attribute__((cleanup(ns_life_guard_leave)))=ns_life_guard_enter();
 if(!ns_lifetime_mode || !identity)return ND_INVALID;
 for(u32 i=0;i<512;++i)if(ns_lifetime_pins[i].identity==identity){--ns_lifetime_pins[i].state->lifetime_pins;ns_lifetime_pins[i]=(struct NsLifetimePin){0,0};return 0;}
 return ND_INVALID;
}
__declspec(dllexport) i32 WIN NativeDrawBackingLiveV1(void* object){
 struct NsLifeGuard guard __attribute__((cleanup(ns_life_guard_leave)))=ns_life_guard_enter();
 if(!ns_lifetime_mode)return -1;
 if(ns_lifetime_find(object))return 1;
 for(struct NsState* s=ns_lifetime_dead;s;s=s->next_owned)for(u32 i=0;i<5;++i)if(object==&s->aliases[i])return 0;
 return -1;
}
__declspec(dllexport) u32 WIN NativeDrawRetirementsV1(void** objects,u32 count){
 struct NsLifeGuard guard __attribute__((cleanup(ns_life_guard_leave)))=ns_life_guard_enter();
 if(!ns_lifetime_mode || !objects || !count || count>32768)return 0;
 u32 n=ns_lifetime_retired_count<count?ns_lifetime_retired_count:count;
 for(u32 i=0;i<n;++i)objects[i]=ns_lifetime_retired[i];
 for(u32 i=n;i<ns_lifetime_retired_count;++i)ns_lifetime_retired[i-n]=ns_lifetime_retired[i];
 ns_lifetime_retired_count-=n;return n;
}
static i32 WIN live_ns_query(struct NsObject* o,const u8* iid,void** out){
 struct NsLifeGuard guard __attribute__((cleanup(ns_life_guard_leave)))=ns_life_guard_enter();
 if(!ns_lifetime_valid(o))return ND_INVALID;
 return ns_query(o,iid,out);
}
static u32 WIN live_ns_retain(struct NsObject* o){
 struct NsLifeGuard guard __attribute__((cleanup(ns_life_guard_leave)))=ns_life_guard_enter();
 if(!ns_lifetime_valid(o))return 0;
 return ns_retain(o);
}
static u32 WIN live_ns_release(struct NsObject* o){
 struct NsLifeGuard guard __attribute__((cleanup(ns_life_guard_leave)))=ns_life_guard_enter();
 if(!ns_lifetime_valid(o))return 0;
 return ns_release(o);
}
static i32 WIN live_ns_flip(struct NsObject* o,struct NsObject* target,u32 flags){
 struct NsLifeGuard guard __attribute__((cleanup(ns_life_guard_leave)))=ns_life_guard_enter();
 if(!ns_lifetime_valid(o) || (target && !ns_lifetime_valid(target)))return ND_INVALID;
 return ns_flip(o,target,flags);
}
static i32 WIN live_ns_attached(struct NsObject* o,const u32* caps,void** out){
 struct NsLifeGuard guard __attribute__((cleanup(ns_life_guard_leave)))=ns_life_guard_enter();
 if(!ns_lifetime_valid(o))return ND_INVALID;
 return ns_attached(o,caps,out);
}
static i32 WIN live_ns_blt(struct NsObject* o,const i32* dr,struct NsObject* src,const i32* sr,u32 flags,const u32* fx){
 struct NsLifeGuard guard __attribute__((cleanup(ns_life_guard_leave)))=ns_life_guard_enter();
 if(!ns_lifetime_valid(o) || (src && !ns_lifetime_valid(src)))return ND_INVALID;
 return ns_blt(o,dr,src,sr,flags,fx);
}
static i32 WIN live_ns_fast(struct NsObject* o,u32 x,u32 y,struct NsObject* src,const i32* r,u32 flags){
 struct NsLifeGuard guard __attribute__((cleanup(ns_life_guard_leave)))=ns_life_guard_enter();
 if(!ns_lifetime_valid(o) || (src && !ns_lifetime_valid(src)))return ND_INVALID;
 return ns_fast(o,x,y,src,r,flags);
}
static i32 WIN live_ns_status(struct NsObject* o,u32 flags){
 struct NsLifeGuard guard __attribute__((cleanup(ns_life_guard_leave)))=ns_life_guard_enter();
 if(!ns_lifetime_valid(o))return ND_INVALID;
 return ns_status(o,flags);
}
static i32 WIN live_ns_caps(struct NsObject* o,u32* caps){
 struct NsLifeGuard guard __attribute__((cleanup(ns_life_guard_leave)))=ns_life_guard_enter();
 if(!ns_lifetime_valid(o))return ND_INVALID;
 return ns_caps(o,caps);
}
static i32 WIN live_ns_get_key(struct NsObject* o,u32 flags,u32* key){
 struct NsLifeGuard guard __attribute__((cleanup(ns_life_guard_leave)))=ns_life_guard_enter();
 if(!ns_lifetime_valid(o))return ND_INVALID;
 return ns_get_key(o,flags,key);
}
static i32 WIN live_ns_get_clipper(struct NsObject* o,void** out){
 struct NsLifeGuard guard __attribute__((cleanup(ns_life_guard_leave)))=ns_life_guard_enter();
 if(!ns_lifetime_valid(o))return ND_INVALID;
 return ns_get_clipper(o,out);
}
static i32 WIN live_ns_set_clipper(struct NsObject* o,struct NcClipper* c){
 struct NsLifeGuard guard __attribute__((cleanup(ns_life_guard_leave)))=ns_life_guard_enter();
 if(!ns_lifetime_valid(o))return ND_INVALID;
 return ns_set_clipper(o,c);
}
static i32 WIN live_ns_get_palette(struct NsObject* o,void** out){
 struct NsLifeGuard guard __attribute__((cleanup(ns_life_guard_leave)))=ns_life_guard_enter();
 if(!ns_lifetime_valid(o))return ND_INVALID;
 return ns_get_palette(o,out);
}
static i32 WIN live_ns_set_palette(struct NsObject* o,struct NpPalette* p){
 struct NsLifeGuard guard __attribute__((cleanup(ns_life_guard_leave)))=ns_life_guard_enter();
 if(!ns_lifetime_valid(o))return ND_INVALID;
 return ns_set_palette(o,p);
}
static i32 WIN live_ns_pixel_format(struct NsObject* o,u32* p){
 struct NsLifeGuard guard __attribute__((cleanup(ns_life_guard_leave)))=ns_life_guard_enter();
 if(!ns_lifetime_valid(o))return ND_INVALID;
 return ns_pixel_format(o,p);
}
static i32 WIN live_ns_desc(struct NsObject* o,u32* d){
 struct NsLifeGuard guard __attribute__((cleanup(ns_life_guard_leave)))=ns_life_guard_enter();
 if(!ns_lifetime_valid(o))return ND_INVALID;
 return ns_desc(o,d);
}
static i32 WIN live_ns_lost(struct NsObject* o){
 struct NsLifeGuard guard __attribute__((cleanup(ns_life_guard_leave)))=ns_life_guard_enter();
 if(!ns_lifetime_valid(o))return ND_INVALID;
 return ns_lost(o);
}
static i32 WIN live_ns_get_dc(struct NsObject* o,void** out){
 struct NsLifeGuard guard __attribute__((cleanup(ns_life_guard_leave)))=ns_life_guard_enter();
 if(!ns_lifetime_valid(o))return ND_INVALID;
 return ns_get_dc(o,out);
}
static i32 WIN live_ns_release_dc(struct NsObject* o,void* dc){
 struct NsLifeGuard guard __attribute__((cleanup(ns_life_guard_leave)))=ns_life_guard_enter();
 if(!ns_lifetime_valid(o))return ND_INVALID;
 return ns_release_dc(o,dc);
}
static i32 WIN live_ns_lock(struct NsObject* o,const i32* rect,u32* d,u32 flags,HANDLE event){
 struct NsLifeGuard guard __attribute__((cleanup(ns_life_guard_leave)))=ns_life_guard_enter();
 if(!ns_lifetime_valid(o))return ND_INVALID;
 return ns_lock(o,rect,d,flags,event);
}
static i32 WIN live_ns_restore(struct NsObject* o){
 struct NsLifeGuard guard __attribute__((cleanup(ns_life_guard_leave)))=ns_life_guard_enter();
 if(!ns_lifetime_valid(o))return ND_INVALID;
 return ns_restore(o);
}
static i32 WIN live_ns_key(struct NsObject* o,u32 flags,const u32* key){
 struct NsLifeGuard guard __attribute__((cleanup(ns_life_guard_leave)))=ns_life_guard_enter();
 if(!ns_lifetime_valid(o))return ND_INVALID;
 return ns_key(o,flags,key);
}
static i32 WIN live_ns_unlock(struct NsObject* o,void* arg){
 struct NsLifeGuard guard __attribute__((cleanup(ns_life_guard_leave)))=ns_life_guard_enter();
 if(!ns_lifetime_valid(o))return ND_INVALID;
 return ns_unlock(o,arg);
}
static i32 WIN live_ns_page_hint(struct NsObject* o,u32 flags){
 struct NsLifeGuard guard __attribute__((cleanup(ns_life_guard_leave)))=ns_life_guard_enter();
 if(!ns_lifetime_valid(o))return ND_INVALID;
 return ns_page_hint(o,flags);
}
static void ns_lifetime_install(void){
 if(!ns_lifetime_mode)return;
 for(u32 k=0;k<5;++k){
 ns_tables[k][0]=(void*)&live_ns_query;
 ns_tables[k][1]=(void*)&live_ns_retain;
 ns_tables[k][2]=(void*)&live_ns_release;
 ns_tables[k][11]=(void*)&live_ns_flip;
 ns_tables[k][12]=(void*)&live_ns_attached;
 ns_tables[k][5]=(void*)&live_ns_blt;
 ns_tables[k][7]=(void*)&live_ns_fast;
 ns_tables[k][13]=(void*)&live_ns_status;
 ns_tables[k][18]=(void*)&live_ns_status;
 ns_tables[k][14]=(void*)&live_ns_caps;
 ns_tables[k][16]=(void*)&live_ns_get_key;
 ns_tables[k][15]=(void*)&live_ns_get_clipper;
 ns_tables[k][28]=(void*)&live_ns_set_clipper;
 ns_tables[k][20]=(void*)&live_ns_get_palette;
 ns_tables[k][31]=(void*)&live_ns_set_palette;
 ns_tables[k][21]=(void*)&live_ns_pixel_format;
 ns_tables[k][22]=(void*)&live_ns_desc;
 ns_tables[k][24]=(void*)&live_ns_lost;
 ns_tables[k][17]=(void*)&live_ns_get_dc;
 ns_tables[k][26]=(void*)&live_ns_release_dc;
 ns_tables[k][25]=(void*)&live_ns_lock;
 ns_tables[k][27]=(void*)&live_ns_restore;
 ns_tables[k][29]=(void*)&live_ns_key;
 ns_tables[k][32]=(void*)&live_ns_unlock;
 if(k>=2)ns_tables[k][37]=(void*)&live_ns_page_hint;
 if(k>=2)ns_tables[k][38]=(void*)&live_ns_page_hint;
 }
}
