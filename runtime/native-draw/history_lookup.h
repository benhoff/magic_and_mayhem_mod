/* Cache namespace resolution only, never CPU authority or permission. All
 * Cache use requires the provider lifetime lease; compatibility mode resolves
 * each call directly, avoiding shared cache writes by concurrent readers. Publication, final backing release
 * and physical Flip invalidate before changing the owned namespace. Exact
 * address keys preserve first-match/interior-pointer behavior, including misses.
 * No public/private pixel pointer or cached state escapes this process. */
#include <stdint.h>
struct NsHistoryLookup {uintptr_t address;struct NsState* state;};
static struct NsHistoryLookup ns_history_lookup[8];
static inline void ns_history_lookup_clear(void){
 for(u32 i=0;i<8;++i)ns_history_lookup[i].address=0;
}
static inline struct NsState* ns_history_find(const void* pixels,u32 serialized){
 const uintptr_t at=(uintptr_t)pixels;struct NsHistoryLookup* entry=ns_history_lookup+((at>>4)&7);
 if(serialized && at && entry->address==at)return entry->state;
 struct NsState* found=0;
 for(struct NsState* s=ns_owned;s;s=s->next_owned){
  const uintptr_t base=(uintptr_t)s->pixels;
  if(at>=base && at-base<s->pitch*s->height){found=s;break;}
 }
 if(serialized){entry->state=found;entry->address=at;}return found;
}
