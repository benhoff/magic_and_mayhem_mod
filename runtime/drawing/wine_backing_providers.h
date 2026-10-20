/* Serialized by journal_lock. Providers own enforcement across foreign aliases;
 * this registry retains the exact read lease through private bind and restore. */
#include "backing_lease_v1.h"
static u32 wine_backing_callbacks;
static struct WineBackingObject {u32 provider,low,high,generation,active;} wine_backing_objects[2048];
static int wine_backing_code(const void* p){u32 m[7];return p&&VirtualQuery(p,m,28)==28&&m[4]==0x1000&&m[6]==0x1000000u&&(m[5]&0xf0)&&!(m[5]&0x100);}
static struct WineBackingProviderSlot {struct MnmBackingProviderV1 api;u32 active,last_token,registered;void* modules[5];} wine_backing_providers[64];
API i32 WIN GetModuleHandleExA(u32,const char*,void**);
API i32 WIN FreeLibrary(void*);
static void wine_backing_unpin(void** modules){for(u32 i=0;i<5;++i)if(modules[i]&&!FreeLibrary(modules[i]))ExitProcess(99);}
static u32 wine_backing_register(struct MnmBackingProviderV1 api){
 if(!wine_handoff_enabled||(api.version!=1&&api.version!=2)||api.bytes!=sizeof(api)||!api.id||api.kind<1||api.kind>4||
    !api.acquire||!api.verify_restored||!api.release||(api.version==1&&(!api.bind_private||!api.restore)))return 0;
 void* modules[5]={0};const void* callbacks[5]={(void*)api.acquire,(void*)api.bind_private,(void*)api.restore,(void*)api.verify_restored,(void*)api.release};
 for(u32 i=0;i<5;++i)if(callbacks[i]&&(!wine_backing_code(callbacks[i])||!GetModuleHandleExA(4,(const char*)callbacks[i],modules+i))){wine_backing_unpin(modules);return 0;}
 EnterCriticalSection(journal_lock);u32 free=64;
 if(!wine_backing_callbacks)for(u32 i=0;i<64;++i){if(wine_backing_providers[i].api.id==api.id){free=64;break;}if(free==64&&!wine_backing_providers[i].api.id)free=i;}
 if(free<64){copy(&wine_backing_providers[free].api,&api,sizeof(api));copy(wine_backing_providers[free].modules,modules,sizeof(modules));wine_backing_providers[free].registered=1;}
 LeaveCriticalSection(journal_lock);if(free==64)wine_backing_unpin(modules);return free<64;
}
u32 WIN DrawingRegisterBackingV1(const struct MnmBackingProviderV1* supplied){
 if(!readable(supplied,sizeof(*supplied)))return 0;struct MnmBackingProviderV1 api;copy(&api,supplied,sizeof(api));if(api.version!=1)return 0;return wine_backing_register(api);
}
u32 WIN DrawingRegisterBackingV2(const struct MnmBackingProviderV2* supplied){
 if(wine_source_ownership_required!=2||!wine_source_ownership_ready||!readable(supplied,sizeof(*supplied)))return 0;
 struct MnmBackingProviderV2 v;copy(&v,supplied,sizeof(v));if(v.version!=2||v.bytes!=sizeof(v))return 0;
 /* Normalize local callback storage only; published V1/V2 layouts are distinct. */
 struct MnmBackingProviderV1 api={2,sizeof(api),v.id,v.kind,v.context,v.acquire,0,0,v.verify_unchanged,v.release};return wine_backing_register(api);
}
static int wine_backing_direct_registered(void){
 if(wine_source_ownership_required!=2)return 0;
 EnterCriticalSection(journal_lock);u32 found=0;
 for(u32 i=0;i<64;++i)if(wine_backing_providers[i].registered&&wine_backing_providers[i].api.version==2)found=1;
 LeaveCriticalSection(journal_lock);return found;
}
u32 WIN DrawingUnregisterBackingV1(u32 id){
 void* modules[5]={0};u32 found=0;
 EnterCriticalSection(journal_lock);if(wine_backing_callbacks){LeaveCriticalSection(journal_lock);return 0;}
 for(u32 i=0;i<64;++i){struct WineBackingProviderSlot* p=wine_backing_providers+i;if(id&&p->api.id==id&&p->registered){
  if(p->active){LeaveCriticalSection(journal_lock);return 0;}p->registered=0;copy(modules,p->modules,sizeof(modules));memset(p->modules,0,sizeof(p->modules));found=1;break;
 }}LeaveCriticalSection(journal_lock);wine_backing_unpin(modules);return found;
}
static void wine_backing_request(struct WineSourceHandoff* s,u32 sub,const u32* mapped,struct MnmBackingRequestV1* q){
 memset(q,0,sizeof(*q));q->version=1;q->bytes=sizeof(*q);q->submission=s->submission;q->texture_id=s->texture_id;q->texture_generation=s->generation;q->subresource=sub;
 q->texture=s->texture;q->resource=s->resource;q->width=s->width;q->height=s->height;q->pitch=mapped?mapped[0]:0;q->mapped=mapped?(const void*)mapped[2]:0;
}
static int wine_backing_acquire(struct WineSourceHandoff* s,u32 sub,const u32* mapped){
 struct MnmBackingRequestV1 q;wine_backing_request(s,sub,mapped,&q);
 for(u32 i=0;i<64;++i){struct WineBackingProviderSlot* p=wine_backing_providers+i;if(!p->registered||p->api.version!=(mapped?1u:2u))continue;
  q.version=p->api.version;
  struct MnmBackingReadV1 r;memset(&r,0,sizeof(r));if(p->active>=2048)wine_handoff_refuse(s,60);++p->active;++wine_backing_callbacks;int result=p->api.acquire(p->api.context,&q,&r);--wine_backing_callbacks;
  if(result==0){--p->active;continue;}
  if(result!=1){--p->active;wine_handoff_refuse(s,60);}
  if(!(mapped?mnm_backing_read_valid(&q,&p->api,&r):mnm_backing_read2_valid(&q,p->api.kind,&r))||(mapped&&r.pitch!=s->pitch)||r.token<=p->last_token||!readable(r.data,r.pitch*s->height)){
   ++wine_backing_callbacks;p->api.release(p->api.context,&r);--wine_backing_callbacks;--p->active;wine_handoff_refuse(s,61);
  }
  u32 object=2048,free_object=2048;
  for(u32 j=0;j<2048;++j){struct WineBackingObject* o=wine_backing_objects+j;
   if(o->provider==p->api.id&&o->low==r.object_low&&o->high==r.object_high){object=j;break;}
   if(free_object==2048&&!o->provider)free_object=j;
  }
  if(object==2048)object=free_object;
  if(object==2048){++wine_backing_callbacks;p->api.release(p->api.context,&r);--wine_backing_callbacks;--p->active;wine_handoff_refuse(s,61);}
  struct WineBackingObject* o=wine_backing_objects+object;
  if(o->provider&&(r.generation<o->generation||(o->active&&r.generation!=o->generation))){++wine_backing_callbacks;p->api.release(p->api.context,&r);--wine_backing_callbacks;--p->active;wine_handoff_refuse(s,61);}
  o->provider=p->api.id;o->low=r.object_low;o->high=r.object_high;o->generation=r.generation;++o->active;s->provider_object=object+1;
  p->last_token=r.token;s->provider=i+1;s->source_mode=mapped?1:2;copy(&s->provider_request,&q,sizeof(q));copy(&s->provider_read,&r,sizeof(r));return 1;
 }
 return 0;
}
static struct WineBackingProviderSlot* wine_backing_slot(struct WineSourceHandoff* s){
 if(!s->provider||s->provider>64)wine_handoff_refuse(s,62);
 struct WineBackingProviderSlot* p=wine_backing_providers+s->provider-1;
 if(!p->registered||!p->active||p->api.kind!=s->provider_read.kind)wine_handoff_refuse(s,62);return p;
}
static int wine_handoff_transfer_writer(void* resource,u32 kind){
 if(kind!=38)return 0;
 for(u32 i=0;i<2048;++i){struct WineSourceHandoff* s=wine_handoffs+i;
  if(s->submission&&s->provider&&s->source_mode==1&&s->resource==resource&&s->thread==GetCurrentThreadId()&&(s->state==3||s->state==6))return 1;
 }return 0;
}
static void wine_backing_bind(struct WineSourceHandoff* s){
 struct WineBackingProviderSlot* p=wine_backing_slot(s);
 ++wine_backing_callbacks;int okay=p->api.bind_private(p->api.context,&s->provider_request,&s->provider_read,(void*)(s->stage_base+16),s->pitch);--wine_backing_callbacks;if(!okay)wine_handoff_refuse(s,63);
}
static void wine_backing_restore(struct WineSourceHandoff* s){
 struct WineBackingProviderSlot* p=wine_backing_slot(s);
 ++wine_backing_callbacks;int okay=p->api.restore(p->api.context,&s->provider_request,&s->provider_read)&&p->api.verify_restored(p->api.context,&s->provider_request,&s->provider_read);--wine_backing_callbacks;if(!okay)wine_handoff_refuse(s,64);
}
static void wine_backing_verify_direct(struct WineSourceHandoff* s){
 struct WineBackingProviderSlot* p=wine_backing_slot(s);
 if(s->source_mode!=2||p->api.version!=2)wine_handoff_refuse(s,62);
 ++wine_backing_callbacks;int okay=p->api.verify_restored(p->api.context,&s->provider_request,&s->provider_read);--wine_backing_callbacks;if(!okay)wine_handoff_refuse(s,64);
}
static void wine_backing_release(struct WineSourceHandoff* s){
 struct WineBackingProviderSlot* p=wine_backing_slot(s);if(!s->provider_object||s->provider_object>2048)wine_handoff_refuse(s,62);
 struct WineBackingObject* o=wine_backing_objects+s->provider_object-1;
 if(o->provider!=p->api.id||o->low!=s->provider_read.object_low||o->high!=s->provider_read.object_high||o->generation!=s->provider_read.generation||!o->active)wine_handoff_refuse(s,62);
 ++wine_backing_callbacks;p->api.release(p->api.context,&s->provider_read);--wine_backing_callbacks;--p->active;--o->active;s->provider=0;s->provider_object=0;
}
