/* Immediate cursor calls retain entry-generation views too. All five original
 * copy wrappers resolve their operands here, so a synchronous callback may
 * retire/rebind a public wrapper without changing an in-flight copy's source.
 * No borrowed wrapper is read after the original copy starts. */
struct NdCursorView {u32 origin,words[12],pin;};
struct NdCursorScope {struct NdCursorView views[3];u32 count,failed;struct NdCursorScope* previous;};
struct NdCursorCopy {struct NdCursorView source,dest;struct NdCursorCopy* previous;};
static struct NdCursorScope* nd_cursor_scope;
static struct NdCursorCopy* nd_cursor_copy_active;
static void nd_cursor_view_drop(struct NdCursorView* v){if(v->pin){if(NativeDrawUnpinV1(v->pin))ExitProcess(123);v->pin=0;}}
static int nd_cursor_scope_enter(struct NdCursorScope* s,u32 slot){
 s->count=s->failed=0;s->previous=nd_cursor_scope;
 u32 target=slot==7||slot==5?0x6a4fe0:0x6a49e0;
 const u32 addresses[3]={0x641fc8,slot==4||slot==5?target:0x642000,target};
 u32 count=slot==4||slot==5?2:slot==6||slot==7?3:slot==8?1:0;
 for(u32 i=0;i<count;++i){struct NdCursorView* v=s->views+i;v->origin=slot==8?0x642000:addresses[i];v->pin=0;
  if(!nd_cursor_wrapper(v->words,(void*)v->origin,&v->pin)){
   for(u32 k=0;k<i;++k)nd_cursor_view_drop(s->views+k);return 0;
  }
  ++s->count;
 }
 nd_cursor_scope=s;return 1;
}
static void nd_cursor_scope_leave(struct NdCursorScope* s){
 if(nd_cursor_scope!=s)ExitProcess(123);nd_cursor_scope=s->previous;
 for(u32 i=0;i<s->count;++i)nd_cursor_view_drop(s->views+i);
}
static void* nd_cursor_owned_operand(void* object,struct NdCursorView* temporary){
 if(nd_cursor_scope)for(u32 i=0;i<nd_cursor_scope->count;++i)if(nd_cursor_scope->views[i].origin==(u32)object)return nd_cursor_scope->views[i].words;
 for(struct NdCursorScope* s=nd_cursor_scope;s;s=s->previous)for(u32 i=0;i<s->count;++i)if(object==s->views[i].words)return object;
 for(struct NdCursorCopy* c=nd_cursor_copy_active;c;c=c->previous)if(object==c->source.words||object==c->dest.words)return object;
 for(u32 i=0;i<128;++i)if(nd_cursor_jobs[i].active && (object==nd_cursor_jobs[i].source||object==nd_cursor_jobs[i].dest))return object;
 temporary->origin=(u32)object;temporary->pin=0;
 if(!nd_cursor_wrapper(temporary->words,object,&temporary->pin))return 0;
 return temporary->words;
}
static void nd_cursor_copy_service(u32 index,void* source,void* dest,const u32* rectangle,u32 x,u32 y){
 struct NdCursorFx entry,exit;nd_cursor_save(&entry);
 struct NsLifeGuard guard __attribute__((cleanup(ns_life_guard_leave)))=ns_life_guard_enter();
 struct NdCursorCopy copy;copy.source.pin=copy.dest.pin=0;copy.previous=nd_cursor_copy_active;
 void* s=nd_cursor_owned_operand(source,&copy.source),*d=nd_cursor_owned_operand(dest,&copy.dest);
 u32 primary_token=nd_cursor_primary?nd_cursor_primary->original:*(u32*)0x6f98c0;
 u32 primary=(u32)dest==primary_token || (nd_cursor_primary && (u32)dest==nd_cursor_primary->clone);
 struct NdCursorJob geometry;geometry.source[2]=s?((u32*)s)[2]:0;geometry.dest[2]=d?((u32*)d)[2]:0;
 if(index!=13 && (!rectangle || !nd_cursor_readable(rectangle,16))){s=0;}
 if(!s || !d || ns_borrowed(ns_lifetime_find((void*)geometry.source[2])) || ns_borrowed(ns_lifetime_find((void*)geometry.dest[2]))){
  ++nd_cursor_counts[13];if(nd_cursor_scope)nd_cursor_scope->failed=1;
  nd_cursor_view_drop(&copy.dest);nd_cursor_view_drop(&copy.source);nd_cursor_restore(&entry);return;
 }
 struct NdCursorPrimary shadow={*(u32*)0x6f98c0,(u32)d,nd_cursor_primary};
 if(primary){nd_cursor_primary=&shadow;*(u32*)0x6f98c0=(u32)d;}
 nd_cursor_copy_active=&copy;nd_cursor_restore(&entry);
 if(index==13)((void(__attribute__((thiscall))*)(void*,void*,u32,u32))nd_cursor_original[index])(s,d,x,y);
 else ((void(__attribute__((thiscall))*)(void*,void*,const u32*,u32,u32))nd_cursor_original[index])(s,d,rectangle,x,y);
 nd_cursor_save(&exit);nd_cursor_copy_active=copy.previous;
 if(primary){if(*(u32*)0x6f98c0==shadow.clone)*(u32*)0x6f98c0=shadow.original;nd_cursor_primary=shadow.previous;}
 nd_cursor_view_drop(&copy.dest);nd_cursor_view_drop(&copy.source);nd_cursor_restore(&exit);
}
static void __attribute__((thiscall)) nd_cursor_copy_full(void* s,void* d,u32 x,u32 y){nd_cursor_copy_service(13,s,d,0,x,y);}
#define ND_CURSOR_COPY(name,index) static void __attribute__((thiscall)) name(void* s,void* d,const u32* r,u32 x,u32 y){nd_cursor_copy_service(index,s,d,r,x,y);}
ND_CURSOR_COPY(nd_cursor_copy_guarded,14)
ND_CURSOR_COPY(nd_cursor_copy_opaque,15)
ND_CURSOR_COPY(nd_cursor_copy_keyed,16)
ND_CURSOR_COPY(nd_cursor_copy_front_keyed,17)
