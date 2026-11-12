#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
typedef uint32_t u32;typedef int32_t i32;typedef uint8_t u8;
struct Snapshot {u8* data;u32 length;};
static unsigned allocations,frees,fail;
static void* GetProcessHeap(void){return 0;}
static void* HeapAlloc(void* heap,u32 flags,u32 bytes){(void)heap;if(fail)return 0;__atomic_add_fetch(&allocations,1,__ATOMIC_RELAXED);return flags&8?calloc(1,bytes):malloc(bytes);}
static void HeapFree(void* heap,u32 flags,void* p){(void)heap;(void)flags;assert(p);__atomic_add_fetch(&frees,1,__ATOMIC_RELAXED);free(p);}
static void zero(void* p,u32 n){memset(p,0,n);}
#include "../runtime/render/snapshot_storage.h"
static void* release_thread(void* p){capture_storage_release(p,65536);return 0;}
int main(void){
 const u32 limit=64u*1024*1024,n=640*480*2;
 u8* a=capture_storage_allocate(0,n,limit);capture_storage_release(a,n);assert(allocations==frees && !capture_storage_bytes);
 capture_storage_enabled=1;
 a=capture_storage_allocate(0,n,limit);memset(a,0xa5,n);capture_storage_release(a,n);assert(capture_storage_bytes==n);
 unsigned old=allocations;u8* b=capture_storage_allocate(0,n,limit);assert(a==b && allocations==old && !capture_storage_bytes);
 for(u32 i=0;i<n;++i)assert(b[i]==0xa5);
 capture_storage_release(b,n);b=capture_storage_allocate(8,n,limit);assert(a==b);for(u32 i=0;i<n;++i)assert(!b[i]);
 struct Snapshot snapshot={b,n};game_snapshot_free(&snapshot);assert(!snapshot.data && capture_storage_bytes==n);
 // A different shape cannot receive a buffer with the wrong extent.
 b=capture_storage_allocate(0,n+2,limit);assert(b!=a);capture_storage_release(b,n+2);assert(capture_storage_bytes==2*n+2);
 // Near budget: discard idle buffers instead of rejecting admissible work.
 b=capture_storage_allocate(0,1024,1024);assert(b && !capture_storage_bytes);capture_storage_release(b,1024);
 assert(!capture_storage_allocate(0,1025,1024));
 // Cache is bounded and live buffers never alias despite concurrent returns.
 u8* held[12];for(unsigned i=0;i<12;++i){held[i]=capture_storage_allocate(0,65536,limit);for(unsigned j=0;j<i;++j)assert(held[i]!=held[j]);}
 pthread_t threads[12];for(unsigned i=0;i<12;++i)assert(!pthread_create(threads+i,0,release_thread,held[i]));
 for(unsigned i=0;i<12;++i)assert(!pthread_join(threads[i],0));
 assert(capture_storage_bytes==4*65536);
 // A checked-out exact match remains usable even if the underlying heap fails.
 fail=1;b=capture_storage_allocate(8,65536,limit);assert(b);for(u32 i=0;i<65536;++i)assert(!b[i]);
 assert(!capture_storage_allocate(0,n,limit));fail=0;capture_storage_release(b,65536);
 // Reuse under a tight budget evicts other idle entries too.
 b=capture_storage_allocate(0,65536,65536);assert(b && !capture_storage_bytes);capture_storage_release(b,65536);
 capture_storage_clear(0);assert(!capture_storage_bytes && capture_storage_enabled && allocations==frees);
 // Stop rejects later recycling, including buffers checked out before stop.
 b=capture_storage_allocate(0,n,limit);capture_storage_clear(1);capture_storage_release(b,n);assert(!capture_storage_bytes && !capture_storage_enabled && allocations==frees);
 capture_storage_clear(1);printf("snapshot storage passed: %u allocations reclaimed; exact extents, zeroing, budget eviction, failure, concurrent return and late teardown\n",allocations);
}
