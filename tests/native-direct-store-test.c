#define _GNU_SOURCE
#include <sys/mman.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef uint8_t u8;typedef uint16_t u16;typedef uint32_t u32;typedef int32_t i32;
#define WIN
#define __declspec(x)
#define ND_INVALID ((i32)0x80070057u)
#define NS_BUSY ((i32)0x887601aeu)
struct NsState {u16 *pixels,*pixels_alias;u8* cpu_dirty;u32 bytes,history_stale,cpu_guard,cpu_guard_world,lock_owner;int locked;void *dc,*storage_dc;struct NsState* next_owned;};
static struct NsState* ns_owned;static u32 ns_guard_enabled=3,last_error=0x77,thread=17,dirty_calls,dirty_offset,dirty_width;
static u32 GetLastError(void){return last_error;}
static void SetLastError(u32 error){last_error=error;}
static u32 GetCurrentThreadId(void){last_error=0xbad;return thread;}
static u32 ns_guard_bytes(const struct NsState* s){return s->bytes;}
static void ns_guard_dirty(struct NsState* s,u32 offset,u32 width){(void)s;++dirty_calls;dirty_offset=offset;dirty_width=width;}
#include "../runtime/native-draw/cpu_direct_store.h"
#define check(x) do{if(!(x)){fprintf(stderr,"failed line%d\n",__LINE__);exit(1);}}while(0)
int main(void){
 u16 public_bytes[16],private_bytes[16];u8 mask[4]={0};memset(public_bytes,0xa5,sizeof(public_bytes));memset(private_bytes,0,sizeof(private_bytes));
 struct NsState s={0};s.pixels=public_bytes;s.pixels_alias=private_bytes;s.cpu_dirty=mask;s.bytes=sizeof(public_bytes);s.history_stale=s.cpu_guard=1;s.locked=1;s.lock_owner=thread;ns_owned=&s;
 check(!NativeDrawCpuStoreV1((u8*)public_bytes+1,0x12345678,4));check(last_error==0x77);
 check(((u8*)private_bytes)[1]==0x78&&((u8*)private_bytes)[4]==0x12&&public_bytes[0]==0xa5a5);
 check(dirty_calls==1&&dirty_offset==1&&dirty_width==4);
 check(!NativeDrawCpuStoreV1((u8*)public_bytes+1,0x12345678,4));check(dirty_calls==2);
 check(NativeDrawCpuStoreV1(private_bytes,1,2)==ND_INVALID);
 check(NativeDrawCpuStoreV1((u8*)public_bytes+s.bytes-1,1,2)==ND_INVALID);
 check(NativeDrawCpuStoreV1(public_bytes,1,3)==ND_INVALID);
 check(NativeDrawCpuStoreV1((void*)(UINTPTR_MAX-1),1,4)==ND_INVALID);
 check(NativeDrawCpuStoreV1(0,1,2)==ND_INVALID);
 thread=18;check(NativeDrawCpuStoreV1(public_bytes,1,2)==NS_BUSY);thread=17;
 s.locked=0;check(NativeDrawCpuStoreV1(public_bytes,1,2)==NS_BUSY);s.locked=1;
 s.cpu_guard_world=1;check(NativeDrawCpuStoreV1(public_bytes,1,2)==NS_BUSY);s.cpu_guard_world=0;
 s.dc=&s;check(NativeDrawCpuStoreV1(public_bytes,1,2)==NS_BUSY);s.dc=0;
 s.storage_dc=&s;check(NativeDrawCpuStoreV1(public_bytes,1,2)==NS_BUSY);s.storage_dc=0;
 s.cpu_dirty=0;check(NativeDrawCpuStoreV1(public_bytes,1,2)==NS_BUSY);s.cpu_dirty=mask;
 check(dirty_calls==2&&last_error==0x77&&private_bytes[0]==0x7800);
 s.history_stale=0;check(NativeDrawCpuStoreV1(public_bytes,1,2)==1);check(public_bytes[0]==0xa5a5);
 u16 outside=42;check(NativeDrawCpuStoreV1(&outside,7,2)==1&&outside==42);
 struct NsState later={0};later.pixels_alias=public_bytes;later.bytes=sizeof(public_bytes);s.next_owned=&later;s.history_stale=1;ns_store_invalidate();
 check(NativeDrawCpuStoreV1(public_bytes,9,2)==ND_INVALID);check(dirty_calls==2&&public_bytes[0]==0xa5a5&&last_error==0x77);
 s.next_owned=0;ns_store_invalidate();
 /* A safe value in a malformed overlapping namespace must not cache the
  * remainder of that public allocation as proof for later values. */
 later.pixels_alias=public_bytes+8;s.next_owned=&later;ns_store_invalidate();
 check(!NativeDrawCpuStoreV1(public_bytes,0x55,2));check(!ns_store_cached);
 check(NativeDrawCpuStoreV1(public_bytes+8,9,2)==ND_INVALID);
 s.next_owned=0;ns_store_invalidate();
 /* Overlapping public views retain list-order selection after an earlier
  * value belonging only to the later entry. No namespace cache is eligible. */
 u16 shadow_private[8]={0};struct NsState shadow=s;
 shadow.pixels=public_bytes+8;shadow.pixels_alias=shadow_private;shadow.bytes=sizeof(shadow_private);shadow.next_owned=&s;
 ns_owned=&shadow;ns_store_invalidate();
 check(!NativeDrawCpuStoreV1(public_bytes,0x6699,2));check(!ns_store_cached);
 u16 prior=private_bytes[8];check(!NativeDrawCpuStoreV1(public_bytes+8,0x7788,2));
 check(shadow_private[0]==0x7788&&private_bytes[8]==prior&&!ns_store_cached);
 ns_owned=&s;ns_store_invalidate();
 /* Invalidate before physical replacement and before destroying metadata.
  * A protected metadata page makes any stale cache dereference observable. */
 long page=sysconf(_SC_PAGESIZE);check(page>0);
 struct NsState* heap=mmap(0,(size_t)page,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);check(heap!=MAP_FAILED);
 *heap=s;ns_owned=heap;ns_store_invalidate();
 check(!NativeDrawCpuStoreV1(public_bytes+6,0x4567,2));check(ns_store_cached==heap);
 ns_store_invalidate();ns_owned=0;check(!mprotect(heap,(size_t)page,PROT_NONE));
 check(NativeDrawCpuStoreV1(&outside,9,2)==1&&outside==42);
 check(!munmap(heap,(size_t)page));
 s.pixels=private_bytes;s.pixels_alias=public_bytes;ns_owned=&s;ns_store_invalidate();
 u16 unchanged=private_bytes[5];check(!NativeDrawCpuStoreV1(private_bytes+5,0x1234,2));check(public_bytes[5]==0x1234&&private_bytes[5]==unchanged);
 ns_store_invalidate();ns_owned=0;
 puts("{\"success\":true,\"checked_private_stores\":7,\"public_writes\":0}");return 0;
}
