/* Pinned Wine11.16 diagnostic private system-memory ownership policy.
 * Own allocation lifetimes, never retain an ephemeral map pointer. Shared,
 * imported, BO and unknown client backing refuse. GPU dispatch is default deny
 * while any source lease exists, except reviewed state/read/disjoint writes. */
#include "wine_source_handoff_state.h"
API void WIN Sleep(u32);
API void* WIN AddVectoredExceptionHandler(u32,void*);
static u32 wine_phys_enabled,wine_phys_installed,wine_phys_records,wine_phys_next,wine_phys_active,wine_phys_internal,wine_phys_vm_active,wine_phys_heap_active;
static HANDLE wine_phys_file,wine_phys_fault_file,wine_phys_clients_file;
static void* wine_phys_prepare_original;
static void* wine_phys_free_original;
static void* wine_phys_cs_original[51];
static void* wine_phys_nt_original[10];
static void* wine_phys_heap_original[4];
static u32 wine_phys_heap_next;
static struct WineHeapAllocation {void* heap;u32 pointer,size,id,retiring,caller,stack[32];} wine_phys_heap_allocations[32768];
static struct WineHeapAllocation* wine_phys_heap_containing(u32 pointer,u32 size){
 for(u32 i=0;i<32768;++i){struct WineHeapAllocation* h=wine_phys_heap_allocations+i;if(h->id&&pointer>=h->pointer&&size<=h->size&&pointer-h->pointer<=h->size-size)return h;}return 0;
}
/* Exact allocation lookups use a bounded open-addressed registry. Backshift
 * deletion preserves probe chains without ever accumulating tombstones. */
static u32 wine_phys_heap_hash(void* heap,void* pointer){return ((((u32)heap>>4)^((u32)pointer>>4))*0x9e3779b1u)>>17;}
static struct WineHeapAllocation* wine_phys_heap_find(void* heap,void* pointer){
 if(!pointer)return 0;u32 at=wine_phys_heap_hash(heap,pointer);
 for(u32 n=0;n<32768;++n,at=(at+1)&32767){struct WineHeapAllocation* h=wine_phys_heap_allocations+at;if(!h->id)return 0;if(h->heap==heap&&h->pointer==(u32)pointer)return h;}return 0;
}
static struct WineHeapAllocation* wine_phys_heap_record(void* heap,void* pointer,u32 size){
 if(!pointer)return 0;if(wine_phys_heap_next==~0u)ExitProcess(99);
 struct WineHeapAllocation* previous=wine_phys_heap_find(heap,pointer);
 if(previous){if(!previous->retiring)ExitProcess(99);memset(previous,0,sizeof(*previous));previous->heap=heap;previous->pointer=(u32)pointer;previous->size=size;previous->id=++wine_phys_heap_next;return previous;}
 u32 at=wine_phys_heap_hash(heap,pointer);
 for(u32 n=0;n<32768;++n,at=(at+1)&32767){struct WineHeapAllocation* h=wine_phys_heap_allocations+at;if(!h->id){h->heap=heap;h->pointer=(u32)pointer;h->size=size;h->id=++wine_phys_heap_next;return h;}}
 ExitProcess(99);return 0;
}
static void wine_phys_heap_forget(void* heap,void* pointer){
 struct WineHeapAllocation* h=wine_phys_heap_find(heap,pointer);if(!h)return;
 u32 hole=(u32)(h-wine_phys_heap_allocations),at=(hole+1)&32767;
 for(u32 n=0;n<32767;++n,at=(at+1)&32767){struct WineHeapAllocation* next=wine_phys_heap_allocations+at;if(!next->id)break;u32 home=wine_phys_heap_hash(next->heap,(void*)next->pointer);
  if(((hole-home)&32767)<((at-home)&32767)){copy(wine_phys_heap_allocations+hole,next,sizeof(*next));hole=at;}
 }
 memset(wine_phys_heap_allocations+hole,0,sizeof(*h));
}
static void wine_phys_heap_complete(void* heap,void* pointer,u32 generation,u32 success){
 struct WineHeapAllocation* h=wine_phys_heap_find(heap,pointer);if(!generation||!h||h->id!=generation)return;
 if(success)wine_phys_heap_forget(heap,pointer);else{if(!h->retiring)ExitProcess(99);--h->retiring;}
}
static struct WinePhysicalOwner {void* resource;void* heap;u32 base,bytes,size,id,refs,prior,data,allocation,kind;} wine_phys_owners[2048];
static struct {u32 submission,owner;void* resource;} wine_phys_leases[2048];
static void wine_phys_emit(u32 phase,u32 owner,u32 submission,u32 kind,u32 value,u32 reason){
 if(!wine_phys_enabled)return;
 u32 r[32]={0};r[0]=++wine_phys_records;r[1]=phase;r[2]=GetCurrentThreadId();r[3]=owner;r[4]=submission;r[5]=kind;r[6]=value;r[7]=reason;
 if(wine_phys_records>262144||!write(wine_phys_file,r,128))ExitProcess(99);
}
static void wine_phys_refuse(u32 owner,u32 submission,u32 kind,u32 reason){wine_phys_emit(5,owner,submission,kind,0,reason);ExitProcess(99);}
static struct WinePhysicalOwner* wine_phys_owner(u32 address){
 for(u32 i=0;i<2048;++i){struct WinePhysicalOwner* o=wine_phys_owners+i;if(o->id&&address>=o->base&&address-o->base<o->bytes)return o;}return 0;
}
static u32 wine_phys_any(void){if(wine_handoff_source_active())return 1;for(u32 i=0;i<2048;++i)if(wine_phys_owners[i].refs)return 1;return 0;}
static int wine_phys_overlap(u32 a,u32 n,u32 b,u32 m){return n&&m&&a<=~0u-n&&b<=~0u-m&&a<b+m&&b<a+n;}
static u32 wine_phys_word(void* p,u32 offset){if(!p||!readable((u8*)p+offset,4))wine_phys_refuse(0,0,0,1);return *(u32*)((u8*)p+offset);}
/* All known client/heap aliases are checked by backing range, not resource ID.
 * A private GPU texture's views retain its resource; shared resources refuse. */
static void wine_phys_writer(void* resource,u32 kind){
 if(!wine_phys_enabled)return;wine_handoff_resource_guard(resource,kind);if(!wine_phys_any())return;
 if(!resource||wine_phys_word(resource,0x14)!=3||(wine_phys_word(resource,0x30)&7))wine_phys_refuse(0,0,kind,2);
 u32 heap=wine_phys_word(resource,0x58),layers=wine_phys_word(resource,0xb0),levels=wine_phys_word(resource,0xb4),subs=wine_phys_word(resource,0x120);
 if(!layers||!levels||layers>64||levels>64||levels*layers>256||!readable((void*)subs,layers*levels*0x34))wine_phys_refuse(0,0,kind,2);
 for(u32 i=0;i<2048;++i){struct WinePhysicalOwner* o=wine_phys_owners+i;if(!o->refs&&!wine_handoff_stage_writer(o->id,resource,kind))continue;
  if(o->resource==resource||wine_phys_overlap(heap,wine_phys_word(resource,0x4c),o->base,o->bytes))wine_phys_refuse(o->id,0,kind,3);
  for(u32 j=0;j<layers*levels;++j){u32* sub=(u32*)(subs+j*0x34);if(wine_phys_overlap(sub[12],sub[3],o->base,o->bytes))wine_phys_refuse(o->id,0,kind,3);}
 }
}
static void wine_phys_binding(void* memory){
 if(!wine_phys_enabled||!memory)return;struct WinePhysicalOwner* o=wine_phys_owner((u32)memory);if(o&&(o->refs||wine_handoff_stage_owned(o->id)))wine_phys_refuse(o->id,0,4,4);
}
static u32 __attribute__((cdecl)) wine_phys_prepare(void* resource){
 u32 error=GetLastError();u8 fp[512] __attribute__((aligned(16)));snapshot_fp(fp);
 EnterCriticalSection(journal_lock);
 u32 memory=wine_phys_word(resource,0x58),size=wine_phys_word(resource,0x4c),result=1;
 if(!memory&&wine_handoff_prepare_restore(resource)){LeaveCriticalSection(journal_lock);restore_fp(fp);SetLastError(error);return 1;}
 if(memory){struct WinePhysicalOwner* o=wine_phys_owner(memory);if(!o||o->resource!=resource)wine_phys_refuse(0,0,0,5);}
 else{
  if(!size||size>0x40000000u)wine_phys_refuse(0,0,0,5);
  u32 slot=2048;for(u32 i=0;i<2048;++i)if(!wine_phys_owners[i].id){slot=i;break;}
  if(slot==2048||wine_phys_next==~0u)wine_phys_refuse(0,0,0,5);
  u32 bytes=(size+16+4095)&~4095u;void* base=VirtualAlloc(0,bytes,0x3000,4);
  if(!base)result=0;
  else{struct WinePhysicalOwner* o=wine_phys_owners+slot;o->resource=resource;o->base=(u32)base;o->bytes=bytes;o->size=size;o->id=++wine_phys_next;o->data=(u32)base+16;o->allocation=(u32)base;o->kind=1;
   *(u32*)((u8*)resource+0x54)=(u32)base;*(u32*)((u8*)resource+0x58)=(u32)base+16;wine_phys_emit(1,o->id,0,1,size,0);}
 }
 LeaveCriticalSection(journal_lock);restore_fp(fp);SetLastError(error);return result;
}
/* Covers the out-of-line free and compiler-inlined resource destruction. */
static void __attribute__((cdecl)) wine_phys_free(void* pointer){
 u32 error=GetLastError();u8 fp[512] __attribute__((aligned(16)));snapshot_fp(fp);u32 owned=0;
 EnterCriticalSection(journal_lock);struct WinePhysicalOwner* o=wine_phys_owner((u32)pointer);
 if(o&&wine_handoff_defer_free(o->id)){LeaveCriticalSection(journal_lock);restore_fp(fp);SetLastError(error);return;}
 if(o&&o->kind==1){if(o->base!=(u32)pointer||o->refs)wine_phys_refuse(o->id,0,0,6);owned=1;
  wine_phys_emit(4,o->id,0,0,o->size,0);wine_phys_internal=GetCurrentThreadId();if(!VirtualFree(pointer,0,0x8000))wine_phys_refuse(o->id,0,0,6);wine_phys_internal=0;memset(o,0,sizeof(*o));}
 LeaveCriticalSection(journal_lock);restore_fp(fp);SetLastError(error);
 if(!owned)((void(__attribute__((cdecl))*)(void*))wine_phys_free_original)(pointer);
}
/* Called while the added READ map is still valid. Only the separately owned
 * allocation survives unmap; the map address is checked and then discarded. */
static void wine_phys_acquire(u32 submission,void* resource,u32 sub,u32 address,u32 row,u32 height,u32 width){
 if(!wine_phys_enabled)return;u32 start=GetTickCount();EnterCriticalSection(journal_lock);
 while(wine_phys_active||wine_phys_vm_active||wine_phys_heap_active){
  if(GetTickCount()-start>=5000)wine_phys_refuse(0,submission,0,8);
  LeaveCriticalSection(journal_lock);Sleep(1);EnterCriticalSection(journal_lock);
 }
 u32 heap=wine_phys_word(resource,0x58),subs=wine_phys_word(resource,0x120),client=readable((void*)subs,0x34)?*(u32*)(subs+0x30):0;
 struct WinePhysicalOwner* o=wine_phys_owner(client?client:heap);u32 size=wine_phys_word(resource,0x4c);
 if(!o&&client){
  struct WineHeapAllocation* allocation=wine_phys_heap_containing(client,size);
  u32 base=client&~4095u,bytes=(client+size+4095u)&~4095u,m[7],slot=2048;
  if(allocation&&size&&client<=~0u-size-4095u&&bytes>base&&VirtualQuery((void*)client,m,28)==28&&m[4]==0x1000&&m[6]==0x20000){
   bytes-=base;
   for(u32 i=0;i<2048;++i){if(!wine_phys_owners[i].id)slot=i;else if(wine_phys_overlap(base,bytes,wine_phys_owners[i].base,wine_phys_owners[i].bytes))wine_phys_refuse(wine_phys_owners[i].id,submission,0,7);}
   if(slot==2048||wine_phys_next==~0u)wine_phys_refuse(0,submission,0,5);
   o=wine_phys_owners+slot;o->resource=resource;o->base=base;o->bytes=bytes;o->size=size;o->data=client;o->allocation=m[1];o->heap=allocation->heap;o->kind=2;o->id=++wine_phys_next;wine_phys_emit(1,o->id,0,2,size,0);wine_phys_emit(9,o->id,0,allocation->id,client-allocation->pointer,0);u32 trace[64]={0};copy(trace,"MNMPHCL1",8);trace[2]=o->id;trace[3]=submission;trace[4]=allocation->id;trace[5]=allocation->size;trace[6]=client-allocation->pointer;trace[7]=allocation->caller;copy(trace+8,allocation->stack,128);if(!write(wine_phys_clients_file,trace,256))ExitProcess(99);
  }
 }
 if(!o||(!client&&(o->resource!=resource||wine_phys_word(resource,0x54)!=o->base||heap!=o->base+16||size!=o->size))||wine_phys_word(resource,0x14)!=3||(wine_phys_word(resource,0x30)&7)||wine_phys_word(resource,0xb0)!=1||wine_phys_word(resource,0xb4)!=1||sub||!readable((void*)subs,0x34)||*(u32*)(subs+8)||*(u32*)(subs+12)!=size||address!=(client?client:heap)||address<o->data||size>o->size||address-o->data>o->size-size||!row||!height||!width||row>32768||height>600||width>800||(height-1)*row+width*2>size)wine_phys_refuse(o?o->id:0,submission,0,7);
 u32 slot=2048;for(u32 i=0;i<2048;++i){if(wine_phys_leases[i].submission==submission)wine_phys_refuse(o->id,submission,0,9);if(!wine_phys_leases[i].submission)slot=i;}
 if(!submission||slot==2048)wine_phys_refuse(o->id,submission,0,9);
 if(!o->refs){u32 m[7],old;o->prior=0;
  for(u32 at=o->base;at<o->base+o->bytes;){
   if(VirtualQuery((void*)at,m,28)!=28||m[1]!=o->allocation||m[4]!=0x1000||(m[5]!=4&&m[5]!=2)||m[6]!=0x20000||m[0]+m[3]<=at||(o->prior&&o->prior!=m[5]))wine_phys_refuse(o->id,submission,0,10);
   o->prior=m[5];at=m[0]+m[3];
  }
  wine_phys_internal=GetCurrentThreadId();if(!VirtualProtect((void*)o->base,o->bytes,2,&old)||(old!=4&&old!=2))wine_phys_refuse(o->id,submission,0,10);wine_phys_internal=0;o->prior=old;
 }
 ++o->refs;wine_phys_leases[slot].submission=submission;wine_phys_leases[slot].owner=o->id;wine_phys_leases[slot].resource=resource;wine_phys_emit(2,o->id,submission,0,o->refs,0);LeaveCriticalSection(journal_lock);
}
static void wine_phys_release(u32 submission){
 if(!wine_phys_enabled)return;
 u32 slot=2048;for(u32 i=0;i<2048;++i)if(wine_phys_leases[i].submission==submission)slot=i;
 if(slot==2048)wine_phys_refuse(0,submission,0,11);struct WinePhysicalOwner* o=0;
 for(u32 i=0;i<2048;++i)if(wine_phys_owners[i].id==wine_phys_leases[slot].owner)o=wine_phys_owners+i;
 if(!o||!o->refs)wine_phys_refuse(0,submission,0,11);
 if(!--o->refs){u32 old;wine_phys_internal=GetCurrentThreadId();if(!VirtualProtect((void*)o->base,o->bytes,o->prior,&old)||old!=2)wine_phys_refuse(o->id,submission,0,11);wine_phys_internal=0;}
 wine_phys_emit(3,o->id,submission,0,o->refs,0);memset(wine_phys_leases+slot,0,sizeof(wine_phys_leases[0]));
 if(!o->refs&&o->kind==2){wine_phys_emit(4,o->id,0,2,o->size,0);memset(o,0,sizeof(*o));}
}
struct WinePhysicalException {u32 code,flags;void* nested;void* address;u32 count,info[15];};
struct WinePhysicalPointers {struct WinePhysicalException* record;void* context;};
static i32 WIN wine_phys_fault(struct WinePhysicalPointers* pointers){
 struct WinePhysicalException* e=pointers?pointers->record:0;if(!e||e->code!=0xc0000005u||e->count<2||e->info[0]!=1)return 0;
 if(wine_handoff_fault_wait(e->info[1]))return -1;
 struct WinePhysicalOwner* o=wine_phys_owner(e->info[1]);if(!o||!o->refs)return 0;
 /* Separate fatal artifact: no recorder lock or partial success terminal. */
 u32 r[16]={0};copy(r,"MNMPFLT1",8);r[2]=o->id;r[3]=e->info[1]-o->base;r[4]=GetCurrentThreadId();r[5]=o->refs;
 if(wine_phys_fault_file)write(wine_phys_fault_file,r,64);ExitProcess(99);return 0;
}
static void wine_phys_vm_check(void* process,void** base,u32* size,u32 kind){
 if(!wine_phys_enabled||wine_phys_internal==GetCurrentThreadId())return;wine_handoff_vm_check(process,base,size,kind);if(!wine_phys_any())return;
 if(process!=(void*)-1&&process!=GetCurrentProcess())wine_phys_refuse(0,0,kind,12);
 if(!readable(base,4)||!readable(size,4))wine_phys_refuse(0,0,kind,12);
 u32 a=(u32)*base,n=*size;
 for(u32 i=0;i<2048;++i){struct WinePhysicalOwner* o=wine_phys_owners+i;if(o->refs&&((!n&&a==o->allocation)||wine_phys_overlap(a,n,o->base,o->bytes)))wine_phys_refuse(o->id,0,kind,12);}
}
/* Keep an in-flight mutation lease through the syscall return: an acquisition
 * cannot slip between validation and an already admitted protection/free. */
static u32 WIN wine_phys_nt_protect(void* process,void** base,u32* size,u32 protection,u32* old){
 u32 error=GetLastError(),external;u8 fp[512] __attribute__((aligned(16)));snapshot_fp(fp);if(wine_phys_internal!=GetCurrentThreadId())wine_handoff_wait_snapshot();EnterCriticalSection(journal_lock);external=wine_phys_internal!=GetCurrentThreadId();wine_phys_vm_check(process,base,size,1);if(external)++wine_phys_vm_active;LeaveCriticalSection(journal_lock);restore_fp(fp);SetLastError(error);
 u32 result=((u32(WIN*)(void*,void**,u32*,u32,u32*))wine_phys_nt_original[0])(process,base,size,protection,old);
 error=GetLastError();snapshot_fp(fp);EnterCriticalSection(journal_lock);if(external)--wine_phys_vm_active;LeaveCriticalSection(journal_lock);restore_fp(fp);SetLastError(error);return result;
}
static u32 WIN wine_phys_nt_free(void* process,void** base,u32* size,u32 type){
 u32 error=GetLastError(),external;u8 fp[512] __attribute__((aligned(16)));snapshot_fp(fp);if(wine_phys_internal!=GetCurrentThreadId())wine_handoff_wait_snapshot();EnterCriticalSection(journal_lock);external=wine_phys_internal!=GetCurrentThreadId();wine_phys_vm_check(process,base,size,2);if(external)++wine_phys_vm_active;LeaveCriticalSection(journal_lock);restore_fp(fp);SetLastError(error);
 u32 result=((u32(WIN*)(void*,void**,u32*,u32))wine_phys_nt_original[1])(process,base,size,type);
 error=GetLastError();snapshot_fp(fp);EnterCriticalSection(journal_lock);if(external)--wine_phys_vm_active;LeaveCriticalSection(journal_lock);restore_fp(fp);SetLastError(error);return result;
}
static void wine_phys_unmap_check(void* process,void* address,u32 kind){
 if(!wine_phys_enabled||!wine_phys_any())return;u32 m[7];
 if(process!=(void*)-1&&process!=GetCurrentProcess())wine_phys_refuse(0,0,kind,12);
 if(VirtualQuery(address,m,28)!=28)wine_phys_refuse(0,0,kind,12);
 for(u32 i=0;i<2048;++i)if(wine_phys_owners[i].refs&&wine_phys_owners[i].allocation==m[1])wine_phys_refuse(wine_phys_owners[i].id,0,kind,12);
}
static u32 WIN wine_phys_nt_allocate(void* process,void** base,u32 zero,u32* size,u32 type,u32 protection){
 u32 error=GetLastError(),external;u8 fp[512] __attribute__((aligned(16)));snapshot_fp(fp);if(wine_phys_internal!=GetCurrentThreadId())wine_handoff_wait_snapshot();EnterCriticalSection(journal_lock);external=wine_phys_internal!=GetCurrentThreadId();if(external){if(type&0x400000u)wine_phys_refuse(0,0,2,19);wine_phys_vm_check(process,base,size,2);++wine_phys_vm_active;}LeaveCriticalSection(journal_lock);restore_fp(fp);SetLastError(error);
 u32 result=((u32(WIN*)(void*,void**,u32,u32*,u32,u32))wine_phys_nt_original[2])(process,base,zero,size,type,protection);
 error=GetLastError();snapshot_fp(fp);EnterCriticalSection(journal_lock);if(external)--wine_phys_vm_active;LeaveCriticalSection(journal_lock);restore_fp(fp);SetLastError(error);return result;
}
static u32 WIN wine_phys_nt_allocate_ex(void* process,void** base,u32* size,u32 type,u32 protection,void* parameters,u32 count){
 u32 error=GetLastError(),external;u8 fp[512] __attribute__((aligned(16)));snapshot_fp(fp);if(wine_phys_internal!=GetCurrentThreadId())wine_handoff_wait_snapshot();EnterCriticalSection(journal_lock);external=wine_phys_internal!=GetCurrentThreadId();if(external){if(type&0x400000u)wine_phys_refuse(0,0,3,19);wine_phys_vm_check(process,base,size,3);++wine_phys_vm_active;}LeaveCriticalSection(journal_lock);restore_fp(fp);SetLastError(error);
 u32 result=((u32(WIN*)(void*,void**,u32*,u32,u32,void*,u32))wine_phys_nt_original[3])(process,base,size,type,protection,parameters,count);
 error=GetLastError();snapshot_fp(fp);EnterCriticalSection(journal_lock);if(external)--wine_phys_vm_active;LeaveCriticalSection(journal_lock);restore_fp(fp);SetLastError(error);return result;
}
static u32 WIN wine_phys_nt_write(void* process,void* address,const void* data,u32 size,u32* count){
 u32 error=GetLastError(),external;u8 fp[512] __attribute__((aligned(16)));snapshot_fp(fp);if(wine_phys_internal!=GetCurrentThreadId())wine_handoff_wait_snapshot();EnterCriticalSection(journal_lock);external=wine_phys_internal!=GetCurrentThreadId();if(external){wine_phys_vm_check(process,&address,&size,4);++wine_phys_vm_active;}LeaveCriticalSection(journal_lock);restore_fp(fp);SetLastError(error);
 u32 result=((u32(WIN*)(void*,void*,const void*,u32,u32*))wine_phys_nt_original[4])(process,address,data,size,count);
 error=GetLastError();snapshot_fp(fp);EnterCriticalSection(journal_lock);if(external)--wine_phys_vm_active;LeaveCriticalSection(journal_lock);restore_fp(fp);SetLastError(error);return result;
}
static u32 WIN wine_phys_nt_set_info(void* process,u32 info,u32 count,void* ranges,void* data,u32 size){
 u32 error=GetLastError(),external;u8 fp[512] __attribute__((aligned(16)));snapshot_fp(fp);if(wine_phys_internal!=GetCurrentThreadId())wine_handoff_wait_snapshot();EnterCriticalSection(journal_lock);external=wine_phys_internal!=GetCurrentThreadId();if(external){if(wine_phys_any())wine_phys_refuse(0,0,5,12);++wine_phys_vm_active;}LeaveCriticalSection(journal_lock);restore_fp(fp);SetLastError(error);
 u32 result=((u32(WIN*)(void*,u32,u32,void*,void*,u32))wine_phys_nt_original[5])(process,info,count,ranges,data,size);
 error=GetLastError();snapshot_fp(fp);EnterCriticalSection(journal_lock);if(external)--wine_phys_vm_active;LeaveCriticalSection(journal_lock);restore_fp(fp);SetLastError(error);return result;
}
static u32 WIN wine_phys_nt_unmap(void* process,void* address){
 u32 error=GetLastError(),external;u8 fp[512] __attribute__((aligned(16)));snapshot_fp(fp);if(wine_phys_internal!=GetCurrentThreadId())wine_handoff_wait_snapshot();EnterCriticalSection(journal_lock);external=wine_phys_internal!=GetCurrentThreadId();if(external){wine_phys_unmap_check(process,address,6);++wine_phys_vm_active;}LeaveCriticalSection(journal_lock);restore_fp(fp);SetLastError(error);
 u32 result=((u32(WIN*)(void*,void*))wine_phys_nt_original[6])(process,address);
 error=GetLastError();snapshot_fp(fp);EnterCriticalSection(journal_lock);if(external)--wine_phys_vm_active;LeaveCriticalSection(journal_lock);restore_fp(fp);SetLastError(error);return result;
}
static u32 WIN wine_phys_nt_unmap_ex(void* process,void* address,u32 flags){
 u32 error=GetLastError(),external;u8 fp[512] __attribute__((aligned(16)));snapshot_fp(fp);if(wine_phys_internal!=GetCurrentThreadId())wine_handoff_wait_snapshot();EnterCriticalSection(journal_lock);external=wine_phys_internal!=GetCurrentThreadId();if(external){wine_phys_unmap_check(process,address,7);++wine_phys_vm_active;}LeaveCriticalSection(journal_lock);restore_fp(fp);SetLastError(error);
 u32 result=((u32(WIN*)(void*,void*,u32))wine_phys_nt_original[7])(process,address,flags);
 error=GetLastError();snapshot_fp(fp);EnterCriticalSection(journal_lock);if(external)--wine_phys_vm_active;LeaveCriticalSection(journal_lock);restore_fp(fp);SetLastError(error);return result;
}
static u32 WIN wine_phys_nt_awe(void){wine_phys_refuse(0,0,8,19);return 0;}
/* Heap entries record exact requested extents and monotonic live generations.
 * No heap query or heap body is called while holding journal_lock. Allocation
 * operation leases also exclude acquisition across an in-flight free/realloc. */
static u32 wine_phys_heap_begin(void* heap,void* pointer){
 wine_handoff_wait_snapshot();EnterCriticalSection(journal_lock);++wine_phys_heap_active;
 struct WineHeapAllocation* h=wine_phys_heap_find(heap,pointer);
 wine_handoff_heap_check(heap,pointer,h?h->id:0,0);
 for(u32 i=0;i<2048;++i){struct WinePhysicalOwner* o=wine_phys_owners+i;if(!o->refs||o->kind!=2)continue;
  if((u32)pointer>=o->base&&(u32)pointer-o->base<o->bytes)wine_phys_refuse(o->id,0,3,18);
  if(h&&wine_phys_overlap(h->pointer,h->size,o->base,o->bytes))wine_phys_refuse(o->id,0,3,18);
  if(!h&&pointer){u32 m[7];if(VirtualQuery(pointer,m,28)!=28||m[1]==o->allocation)wine_phys_refuse(o->id,0,3,18);}
 }
 u32 generation=h?h->id:0;if(h)++h->retiring;LeaveCriticalSection(journal_lock);return generation;
}
static void* WIN wine_phys_heap_alloc(void* heap,u32 flags,u32 size){
 u32 error=GetLastError(),caller=(u32)__builtin_return_address(0),stack[32]={0};u8 fp[512] __attribute__((aligned(16)));snapshot_fp(fp);wine_handoff_wait_snapshot();u32* frame=(u32*)__builtin_frame_address(0);if(size>=2048&&readable(frame+1,128))copy(stack,frame+1,128);EnterCriticalSection(journal_lock);++wine_phys_heap_active;LeaveCriticalSection(journal_lock);restore_fp(fp);SetLastError(error);
 void* result=((void*(WIN*)(void*,u32,u32))wine_phys_heap_original[3])(heap,flags,size);
 error=GetLastError();snapshot_fp(fp);EnterCriticalSection(journal_lock);struct WineHeapAllocation* h=wine_phys_heap_record(heap,result,size);if(h){h->caller=caller;copy(h->stack,stack,128);}--wine_phys_heap_active;LeaveCriticalSection(journal_lock);restore_fp(fp);SetLastError(error);return result;
}
static u8 WIN wine_phys_heap_free(void* heap,u32 flags,void* pointer){
 u32 error=GetLastError();u8 fp[512] __attribute__((aligned(16)));snapshot_fp(fp);u32 generation=wine_phys_heap_begin(heap,pointer);restore_fp(fp);SetLastError(error);
 u8 result=((u8(WIN*)(void*,u32,void*))wine_phys_heap_original[0])(heap,flags,pointer);
 error=GetLastError();snapshot_fp(fp);EnterCriticalSection(journal_lock);wine_phys_heap_complete(heap,pointer,generation,result!=0);--wine_phys_heap_active;LeaveCriticalSection(journal_lock);restore_fp(fp);SetLastError(error);return result;
}
static void* WIN wine_phys_heap_realloc(void* heap,u32 flags,void* pointer,u32 size){
 u32 error=GetLastError();u8 fp[512] __attribute__((aligned(16)));snapshot_fp(fp);u32 generation=wine_phys_heap_begin(heap,pointer);restore_fp(fp);SetLastError(error);
 void* result=((void*(WIN*)(void*,u32,void*,u32))wine_phys_heap_original[1])(heap,flags,pointer,size);
 error=GetLastError();snapshot_fp(fp);EnterCriticalSection(journal_lock);if(result){wine_phys_heap_complete(heap,pointer,generation,1);struct WineHeapAllocation* h=wine_phys_heap_find(heap,result);if(!h)wine_phys_heap_record(heap,result,size);else if(h->heap!=heap||h->pointer!=(u32)result||h->size!=size)wine_phys_refuse(0,0,3,18);}else wine_phys_heap_complete(heap,pointer,generation,0);--wine_phys_heap_active;LeaveCriticalSection(journal_lock);restore_fp(fp);SetLastError(error);return result;
}
static void* WIN wine_phys_heap_destroy(void* heap){
 u32 error=GetLastError();u8 fp[512] __attribute__((aligned(16)));snapshot_fp(fp);wine_handoff_wait_snapshot();EnterCriticalSection(journal_lock);wine_handoff_heap_check(heap,0,0,1);LeaveCriticalSection(journal_lock);EnterCriticalSection(journal_lock);for(u32 i=0;i<2048;++i){struct WinePhysicalOwner* o=wine_phys_owners+i;if(o->refs){struct WineHeapAllocation* h=wine_phys_heap_containing((u32)o->resource,1);if(o->heap==heap||(h&&h->heap==heap))wine_phys_refuse(o->id,0,3,18);}
  if(wine_phys_leases[i].submission){struct WineHeapAllocation* h=wine_phys_heap_containing((u32)wine_phys_leases[i].resource,1);if(h&&h->heap==heap)wine_phys_refuse(wine_phys_leases[i].owner,0,3,18);}
 }u32 epoch=wine_phys_heap_next;for(u32 i=0;i<32768;++i)if(wine_phys_heap_allocations[i].id&&wine_phys_heap_allocations[i].heap==heap)++wine_phys_heap_allocations[i].retiring;++wine_phys_heap_active;LeaveCriticalSection(journal_lock);restore_fp(fp);SetLastError(error);
 void* result=((void*(WIN*)(void*))wine_phys_heap_original[2])(heap);
 error=GetLastError();snapshot_fp(fp);EnterCriticalSection(journal_lock);if(!result)for(u32 i=0;i<32768;){struct WineHeapAllocation* h=wine_phys_heap_allocations+i;if(h->id&&h->heap==heap&&h->id<=epoch)wine_phys_heap_forget(heap,(void*)h->pointer);else ++i;}else for(u32 i=0;i<32768;++i){struct WineHeapAllocation* h=wine_phys_heap_allocations+i;if(h->id&&h->heap==heap&&h->id<=epoch){if(!h->retiring)ExitProcess(99);--h->retiring;}}--wine_phys_heap_active;LeaveCriticalSection(journal_lock);restore_fp(fp);SetLastError(error);return result;
}
static void __attribute__((cdecl)) wine_phys_cs(void* cs,const u32* packet){
 u32 error=GetLastError();u8 fp[512] __attribute__((aligned(16)));snapshot_fp(fp);u32 opcode=wine_phys_word((void*)packet,0),recording;
 if(opcode>=51)wine_phys_refuse(0,0,opcode,13);wine_handoff_wait_snapshot();
 EnterCriticalSection(journal_lock);recording=wine_phys_enabled;
 if(recording){
  if(!wine_handoff_cs_allowed(opcode,packet))wine_phys_refuse(0,0,opcode,32);
  if(wine_phys_any()){
   if(opcode==42||opcode==43||opcode==44||opcode==45||opcode==38)wine_phys_writer((void*)wine_phys_word((void*)packet,4),opcode);
   else if(opcode==39){if(wine_phys_word((void*)packet,20)!=0x80000000u)wine_phys_writer((void*)wine_phys_word((void*)packet,4),opcode);}
   else if(opcode==40){void* resource=(void*)wine_phys_word((void*)packet,4);u32 index=wine_phys_word((void*)packet,8),subs=wine_phys_word(resource,0x120),count=wine_phys_word(resource,0xb0)*wine_phys_word(resource,0xb4);if(index>=count||index>255||wine_phys_word((void*)subs,index*0x34+20)!=0x80000000u)wine_phys_writer(resource,opcode);}
   else if(!(opcode==0||opcode==5||(opcode>=6&&opcode<=34)||opcode==36||opcode==37))wine_phys_refuse(0,0,opcode,14);
  }
  ++wine_phys_active;wine_phys_emit(7,0,0,opcode,wine_phys_active,0);
 }
 LeaveCriticalSection(journal_lock);restore_fp(fp);SetLastError(error);
 ((void(__attribute__((cdecl))*)(void*,const void*))wine_phys_cs_original[opcode])(cs,packet);
 error=GetLastError();snapshot_fp(fp);EnterCriticalSection(journal_lock);
 if(recording){if(!wine_phys_active||!wine_phys_enabled)wine_phys_refuse(0,0,opcode,15);--wine_phys_active;wine_phys_emit(8,0,0,opcode,wine_phys_active,0);}
 LeaveCriticalSection(journal_lock);restore_fp(fp);SetLastError(error);
}
static int wine_phys_open(u32 prerequisites){
 char flag[2];u32 n=GetEnvironmentVariableA("MNM_DRAW_WINE_PHYSICAL_OWNERS",flag,2);if(!n)return 1;if(n!=1||flag[0]!='1'||!prerequisites)return 0;
 char name[600];path(name,"wine-physical-owners.bin");wine_phys_file=CreateFileA(name,0x40000000,3,0,1,0x80,0);if(wine_phys_file==(HANDLE)-1)return 0;
 u32 h[16]={0};copy(h,"MNMPOW02",8);h[2]=2;h[3]=128;h[4]=0x40209ca7;h[5]=262144;h[6]=2048;h[7]=51;if(!write(wine_phys_file,h,64))return 0;path(name,"wine-physical-owner-fault.bin");wine_phys_fault_file=CreateFileA(name,0x40000000,3,0,1,0x80,0);if(wine_phys_fault_file==(HANDLE)-1)return 0;path(name,"wine-physical-clients.bin");wine_phys_clients_file=CreateFileA(name,0x40000000,3,0,1,0x80,0);if(wine_phys_clients_file==(HANDLE)-1)return 0;wine_phys_enabled=1;return 1;
}
static int wine_phys_detour(u8* site,const u8* prefix,u32 n,void* hook,void** original){
 if(!readable(site,n)||!equal(site,prefix,n))return 0;u8* out=VirtualAlloc(0,n+5,0x3000,0x40);if(!out)return 0;copy(out,site,n);out[n]=0xe9;u32 delta=(u32)site-(u32)out-5;copy(out+n+1,&delta,4);
 if(!FlushInstructionCache(GetCurrentProcess(),out,n+5))return 0;u32 old,unused;if(!VirtualProtect(site,n,0x40,&old))return 0;
 *original=out;u8 patch[7]={0xe9,0,0,0,0,0x90,0x90};delta=(u32)hook-(u32)site-5;copy(patch+1,&delta,4);copy(site,patch,n);int okay=FlushInstructionCache(GetCurrentProcess(),site,n);return VirtualProtect(site,n,old,&unused)&&okay;
}
static void __attribute__((cdecl)) wine_blit_worker(void*,const u32*);
static int wine_phys_install(void* module){
 if(!wine_phys_enabled)return 1;if(wine_phys_installed)return 0;
 const u32 rvas[51]={0x1cf2c0,0x1cf2f0,0x1ce7b0,0x1cea70,0x1ceb00,0x1cf070,0x1d0510,0x1d0da0,0x1d0820,0x1d0590,0x1cfe60,0x1d0d70,0x1d0a60,0x1d0a00,0x1d0060,0x1cfcf0,0x1d0af0,0x1d0960,0x1d0ca0,0x1d07a0,0x1d08a0,0x1cfa60,0x1cfe00,0x1d0530,0x1cfdc0,0x1d0560,0x1d0c50,0x1cfb00,0x1d00c0,0x1d0480,0x1d0010,0x1cffc0,0x1d0040,0x1cf800,0x1cf9a0,0x1ce790,0x1cf8e0,0x1cf2d0,0x1d0e20,0x1cf1d0,0x1d0e40,0x1cf220,0x1ce280,0x1d0e70,0x1ce130,0x1ce820,0x1ce870,0x1ce920,0x1cf130,0x1ce9c0,0x1ced00};
 void** table=(void**)((u8*)module+0x24d420);if(!readable(table,51*4))return 0;
 for(u32 i=0;i<51;++i)if(table[i]!=(i==42?(void*)&wine_blit_worker:(u8*)module+rvas[i]))return 0;
 const u8 prepare[6]={0x55,0x89,0xe5,0x83,0xec,0x10};
 void* nt=GetModuleHandleA("ntdll.dll");const u8 protect[5]={0xb8,0x50,0,0,0},freent[5]={0xb8,0x1e,0,0,0};
 u8* ps=nt?(u8*)GetProcAddress(nt,"NtProtectVirtualMemory"):0;u8* fs=nt?(u8*)GetProcAddress(nt,"NtFreeVirtualMemory"):0;
 void** free_slot=(void**)((u8*)module+0x3396b0);void* crt=GetModuleHandleA("ucrtbase.dll");void* fre=crt?GetProcAddress(crt,"free"):0;
 if(!readable((u8*)module+0x204a60,6)||!equal((u8*)module+0x204a60,prepare,6)||!nt||ps!=(u8*)nt+0xd724||fs!=(u8*)nt+0xd404||!readable(ps,5)||!equal(ps,protect,5)||!readable(fs,5)||!equal(fs,freent,5)||!fre||!readable(free_slot,4)||*free_slot!=fre)return 0;
 const char* hn[4]={"RtlFreeHeap","RtlReAllocateHeap","RtlDestroyHeap","RtlAllocateHeap"};const u32 hr[4]={0x25d70,0x32f60,0x21250,0x19890},hs[4]={7,6,6,6};
 const u8 hp[4][7]={{0x8b,0xff,0x55,0x8b,0xec,0x57,0x56},{0x55,0x89,0xe5,0x83,0xec,0x7c,0},{0x55,0x89,0xe5,0x83,0xec,0x38,0},{0x8b,0xff,0x55,0x8b,0xec,0x57,0}};
 void* hh[4]={(void*)&wine_phys_heap_free,(void*)&wine_phys_heap_realloc,(void*)&wine_phys_heap_destroy,(void*)&wine_phys_heap_alloc};
 for(u32 i=0;i<4;++i){u8* site=(u8*)GetProcAddress(nt,hn[i]);if(site!=(u8*)nt+hr[i]||!readable(site,hs[i])||!equal(site,hp[i],hs[i]))return 0;}
 const char* vn[7]={"NtAllocateVirtualMemory","NtAllocateVirtualMemoryEx","NtWriteVirtualMemory","NtSetInformationVirtualMemory","NtUnmapViewOfSection","NtUnmapViewOfSectionEx","NtMapUserPhysicalPagesScatter"};
 const u32 vr[7]={0xd3a4,0xd8d4,0xd5c4,0xe0f4,0xd4c4,0xe264,0xd254},vi[7]={0x18,0x6b,0x3a,0xed,0x2a,0x104,3};u8 vp[7][5];
 void* vh[7]={(void*)&wine_phys_nt_allocate,(void*)&wine_phys_nt_allocate_ex,(void*)&wine_phys_nt_write,(void*)&wine_phys_nt_set_info,(void*)&wine_phys_nt_unmap,(void*)&wine_phys_nt_unmap_ex,(void*)&wine_phys_nt_awe};
 for(u32 i=0;i<7;++i){vp[i][0]=0xb8;copy(vp[i]+1,vi+i,4);u8* site=(u8*)GetProcAddress(nt,vn[i]);if(site!=(u8*)nt+vr[i]||!readable(site,5)||!equal(site,vp[i],5))return 0;}
 if(!AddVectoredExceptionHandler(1,(void*)&wine_phys_fault))return 0;
 if(!wine_phys_detour((u8*)module+0x204a60,prepare,6,(void*)&wine_phys_prepare,&wine_phys_prepare_original)||!wine_phys_detour(ps,protect,5,(void*)&wine_phys_nt_protect,wine_phys_nt_original)||!wine_phys_detour(fs,freent,5,(void*)&wine_phys_nt_free,wine_phys_nt_original+1))return 0;
 for(u32 i=0;i<7;++i)if(!wine_phys_detour((u8*)nt+vr[i],vp[i],5,vh[i],wine_phys_nt_original+i+2))return 0;
 for(u32 i=0;i<4;++i)if(!wine_phys_detour((u8*)nt+hr[i],hp[i],hs[i],hh[i],wine_phys_heap_original+i))return 0;
 u32 old,unused;if(!VirtualProtect(free_slot,4,4,&old))return 0;wine_phys_free_original=*free_slot;*free_slot=(void*)&wine_phys_free;if(!VirtualProtect(free_slot,4,old,&unused))return 0;
 if(!VirtualProtect(table,51*4,4,&old))return 0;for(u32 i=0;i<51;++i){wine_phys_cs_original[i]=table[i];table[i]=(void*)&wine_phys_cs;}if(!VirtualProtect(table,51*4,old,&unused))return 0;wine_phys_installed=1;return 1;
}
static void wine_phys_close(u32 reason){
 if(!wine_phys_enabled)return;if(wine_phys_any()||wine_phys_active)wine_phys_refuse(0,0,0,16);wine_phys_emit(6,0,0,0,reason,0);wine_phys_enabled=0;if(!CloseHandle(wine_phys_file))ExitProcess(99);wine_phys_file=0;if(!CloseHandle(wine_phys_fault_file))ExitProcess(99);wine_phys_fault_file=0;if(!CloseHandle(wine_phys_clients_file))ExitProcess(99);wine_phys_clients_file=0;
 /* Allocator/free detours remain paired until process termination. */
}
