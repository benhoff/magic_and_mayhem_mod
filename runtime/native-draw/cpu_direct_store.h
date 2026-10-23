/* PE32-local value-only write service. Private alias pointers never escape.
 * Return0 handled,1 retain ordinary CPU storage, or an explicit refusal. */
#include <stdint.h>
/* Only the namespace lookup is cached. Allocation publication, final release
 * and physical Flip invalidate before mutation. All admission stays per call;
 * lifecycle mutation remains serialized by the caller's existing lease. */
static struct NsState* ns_store_cached;
static void ns_store_invalidate(void){__atomic_store_n(&ns_store_cached,0,__ATOMIC_RELEASE);}
__declspec(dllexport) i32 WIN NativeDrawCpuStoreV1(void* destination,u32 value,u32 width){
 const u32 error=GetLastError();const uintptr_t at=(uintptr_t)destination;
 i32 result=1;
 if(!at || (width!=1 && width!=2 && width!=4) || at>UINTPTR_MAX-width){result=ND_INVALID;goto done;}
 struct NsState* selected=__atomic_load_n(&ns_store_cached,__ATOMIC_ACQUIRE);int cached=0;
 if(selected){const uintptr_t base=(uintptr_t)selected->pixels;const u32 bytes=ns_guard_bytes(selected);
  if(base<=UINTPTR_MAX-bytes && at>=base && at<base+bytes){
   if(width>bytes || at-base>bytes-width){result=ND_INVALID;goto done;}cached=1;
  }else selected=0;
 }
 if(!cached)for(struct NsState* s=ns_owned;s;s=s->next_owned){
  const uintptr_t alias=(uintptr_t)s->pixels_alias;const u32 bytes=ns_guard_bytes(s);
  if(alias && alias>UINTPTR_MAX-bytes){result=ND_INVALID;goto done;}
  if(alias && at<alias+bytes && alias<at+width){result=ND_INVALID;goto done;}
  if(!selected){
   const uintptr_t base=(uintptr_t)s->pixels;
   if(base>UINTPTR_MAX-bytes){result=ND_INVALID;goto done;}
   if(at<base+bytes && at+width>base){
    if(at<base || width>bytes || at-base>bytes-width){result=ND_INVALID;goto done;}
    selected=s;
   }
  }
 }
 if(selected){
  struct NsState* s=selected;const uintptr_t base=(uintptr_t)s->pixels;
  if(s->cpu_guard_world){result=NS_BUSY;goto done;}
  if(!s->history_stale || ns_guard_enabled!=3)goto done;
  if(!s->pixels_alias || !s->cpu_dirty || !s->cpu_guard || s->locked<=0 ||
     s->lock_owner!=GetCurrentThreadId() || s->dc || s->storage_dc){result=NS_BUSY;goto done;}
  if(!cached){
   /* A malformed overlapping registry keeps the complete per-value path.
    * Cache only a full public allocation disjoint from every private view
    * and every other public allocation, preserving first-match selection. */
   int cacheable=1;const uintptr_t end=base+ns_guard_bytes(s);
   for(struct NsState* p=ns_owned;p;p=p->next_owned){
    const u32 bytes=ns_guard_bytes(p);const uintptr_t alias=(uintptr_t)p->pixels_alias;
    const uintptr_t public_base=(uintptr_t)p->pixels;
    if((alias && (alias>UINTPTR_MAX-bytes || (base<alias+bytes && alias<end))) ||
       (p!=s && (public_base>UINTPTR_MAX-bytes || (base<public_base+bytes && public_base<end)))){cacheable=0;break;}}
   if(cacheable)__atomic_store_n(&ns_store_cached,s,__ATOMIC_RELEASE);
  }
  const u32 offset=(u32)(at-base);u8* target=(u8*)s->pixels_alias+offset;
  for(u32 i=0;i<width;++i)target[i]=(u8)(value>>(i*8));
  ns_guard_dirty(s,offset,width);result=0;goto done;
 }
 done:SetLastError(error);return result;
}
