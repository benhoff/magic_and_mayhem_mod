/* Per-submit reference renderer binding. General heap allocation is unchanged.
 * Source pages are protected only across a callout-free value transfer. The
 * original texture binding is restored before the intercepted API returns. */
static struct WineSourceHandoff* wine_handoff_find(u32 submission){for(u32 i=0;i<2048;++i)if(wine_handoffs[i].submission==submission)return wine_handoffs+i;return 0;}
static struct WinePhysicalOwner* wine_handoff_owner(u32 id){for(u32 i=0;i<2048;++i)if(id&&wine_phys_owners[i].id==id)return wine_phys_owners+i;return 0;}
static void wine_handoff_emit(struct WineSourceHandoff* s,u32 phase,u32 reason){
 u32 r[64]={0};r[0]=++wine_handoff_records;r[1]=phase;r[2]=GetCurrentThreadId();r[15]=wine_phys_records;r[16]=reason;
 if(!s&&phase==8)r[14]=wine_handoff_count;
 if(s){r[3]=s->submission;r[4]=s->texture_id;r[5]=s->generation;r[6]=s->resource_id;r[7]=s->old_owner?1:2;r[8]=s->width;r[9]=s->height;r[10]=s->pitch;r[11]=s->width*s->height*2;r[12]=s->crc;r[13]=s->stage_owner;r[14]=s->epoch;r[17]=s->old_heap;r[18]=s->old_heap_offset;r[19]=s->old_owner;r[20]=s->state;}
 if(wine_handoff_records>131072||!write(wine_handoff_file,r,256))ExitProcess(99);
}
static void wine_handoff_refuse(struct WineSourceHandoff* s,u32 reason){if(wine_handoff_enabled)wine_handoff_emit(s,9,reason);ExitProcess(99);}
static void wine_handoff_wait_snapshot(void){
 if(!wine_handoff_enabled)return;u32 start=GetTickCount();
 for(;;){EnterCriticalSection(journal_lock);u32 slot=wine_handoff_snapshot;
  if(!slot){LeaveCriticalSection(journal_lock);return;}
  struct WineSourceHandoff* s=wine_handoffs+slot-1;
  if(s->thread==GetCurrentThreadId()||GetTickCount()-start>=5000)wine_handoff_refuse(s,1);
  LeaveCriticalSection(journal_lock);Sleep(1);
 }
}
static int wine_handoff_fault_wait(u32 address){
 if(!wine_handoff_enabled||!wine_handoff_snapshot)return 0;
 struct WineSourceHandoff* s=wine_handoffs+wine_handoff_snapshot-1;
 if(address<s->original_base||address-s->original_base>=s->original_bytes)return 0;
 /* A write into actual pixels is a failed source boundary, even if another
  * thread could resume later. Heap metadata/padding may wait for the thaw. */
 u32 offset=address-s->old_data;
 if(address>=s->old_data&&offset<s->size&&offset%s->pitch<s->width*2){u32 r[16]={0};copy(r,"MNMPFLT1",8);r[2]=s->stage_owner;r[3]=offset;r[4]=GetCurrentThreadId();r[5]=1;if(wine_phys_fault_file)write(wine_phys_fault_file,r,64);ExitProcess(99);}
 if(GetCurrentThreadId()==s->thread)wine_handoff_refuse(s,2);
 wine_handoff_wait_snapshot();u32 m[7];return VirtualQuery((void*)address,m,28)==28&&m[4]==0x1000&&m[5]==4;
}
/* Cached public DC exports must not expose the private submit buffer. */
static void* wine_handoff_dc_original[2];
static u32 __attribute__((cdecl)) wine_handoff_dc_call(u32 kind,void* texture,u32 sub,void* value){
 u32 error=GetLastError();u8 fp[512] __attribute__((aligned(16)));snapshot_fp(fp);wine_handoff_source_available(texture);restore_fp(fp);SetLastError(error);
 return ((u32(__attribute__((cdecl))*)(void*,u32,void*))wine_handoff_dc_original[kind])(texture,sub,value);
}
static u32 __attribute__((cdecl)) wine_handoff_get_dc(void* texture,u32 sub,void* dc){return wine_handoff_dc_call(0,texture,sub,dc);}
static u32 __attribute__((cdecl)) wine_handoff_release_dc(void* texture,u32 sub,void* dc){return wine_handoff_dc_call(1,texture,sub,dc);}
static int wine_handoff_source_active(void){if(!wine_handoff_enabled)return 0;for(u32 i=0;i<2048;++i)if(wine_handoffs[i].submission)return 1;return 0;}
static int wine_handoff_source_available(void* resource){
 if(!wine_handoff_enabled)return 1;EnterCriticalSection(journal_lock);
 for(u32 i=0;i<2048;++i)if(wine_handoffs[i].submission&&wine_handoffs[i].resource==resource)wine_handoff_refuse(wine_handoffs+i,3);
 LeaveCriticalSection(journal_lock);return 1;
}
static void wine_handoff_resource_guard(void* resource,u32 kind){
 if(!wine_handoff_enabled)return;
 for(u32 i=0;i<2048;++i){struct WineSourceHandoff* s=wine_handoffs+i;if(s->submission&&s->resource==resource){if(kind==38&&(s->state==3||s->state==6))return;wine_handoff_refuse(s,4);}}
}
static int wine_handoff_cs_allowed(u32 opcode,const u32* packet){
 if(!wine_handoff_enabled)return 1;
 if(opcode!=27&&opcode!=37&&opcode!=39&&opcode!=40)return 1;
 void* resource=(void*)wine_phys_word((void*)packet,4);
 for(u32 i=0;i<2048;++i){struct WineSourceHandoff* s=wine_handoffs+i;if(s->submission&&s->resource==resource&&s->state>=4&&s->state<6)return 0;}
 return 1;
}
static void wine_handoff_heap_check(void* heap,void* pointer,u32 generation,u32 destroy){
 (void)pointer; if(!wine_handoff_enabled)return;
 for(u32 i=0;i<2048;++i){struct WineSourceHandoff* s=wine_handoffs+i;if(!s->submission||!s->old_heap)continue;
  if((destroy&&(u32)heap==s->old_heap_handle)||(generation&&generation==s->old_heap))wine_handoff_refuse(s,5);
 }
}
static void wine_handoff_vm_check(void* process,void** base,u32* size,u32 kind){
 if(!wine_handoff_enabled||kind!=2||!wine_handoff_source_active())return;
 if(process!=(void*)-1&&process!=GetCurrentProcess())wine_handoff_refuse(0,6);
 if(!readable(base,4)||!readable(size,4))wine_handoff_refuse(0,6);
 u32 at=(u32)*base,bytes=*size;
 for(u32 i=0;i<2048;++i){struct WineSourceHandoff* s=wine_handoffs+i;if(!s->submission)continue;
  if((!bytes&&at==s->original_allocation)||wine_phys_overlap(at,bytes,s->original_base,s->original_bytes))wine_handoff_refuse(s,6);
 }
}
static int wine_handoff_defer_free(u32 owner){
 if(!wine_handoff_enabled)return 0;
 for(u32 i=0;i<2048;++i){struct WineSourceHandoff* s=wine_handoffs+i;if(s->submission&&s->old_owner==owner&&!s->old_client&&s->state==3&&s->thread==GetCurrentThreadId()){s->detached=1;return 1;}}
 for(u32 i=0;i<2048;++i)if(wine_handoffs[i].submission&&wine_handoffs[i].old_owner==owner)wine_handoff_refuse(wine_handoffs+i,7);
 return 0;
}
static int wine_handoff_prepare_restore(void* resource){
 if(!wine_handoff_enabled)return 0;
 for(u32 i=0;i<2048;++i){struct WineSourceHandoff* s=wine_handoffs+i;if(s->submission&&s->resource==resource&&s->state==6&&s->thread==GetCurrentThreadId()&&!s->old_client){
  struct WinePhysicalOwner* o=wine_handoff_owner(s->old_owner);if(!o||!s->detached||o->refs||o->resource!=resource)wine_handoff_refuse(s,8);
  *(u32*)((u8*)resource+0x54)=o->base;*(u32*)((u8*)resource+0x58)=o->data;s->detached=0;return 1;
 }}return 0;
}
/* All original/API callouts occur before protection or after restoration. */
static int wine_handoff_capture(const u32* sample,void* texture,void* resource,u32 sub,const u32* mapped,u32 width,u32 height,void* packed){
 if(!wine_handoff_enabled)return 0;
 u32 start=GetTickCount();EnterCriticalSection(journal_lock);
 while(wine_phys_active||wine_phys_vm_active||wine_phys_heap_active){if(GetTickCount()-start>=5000)wine_handoff_refuse(0,9);LeaveCriticalSection(journal_lock);Sleep(1);EnterCriticalSection(journal_lock);}
 u32 subs=wine_phys_word(resource,0x120),client=wine_phys_word((void*)subs,0x30),heap=wine_phys_word(resource,0x58),size=wine_phys_word(resource,0x4c),slot=2048;
 if(sub||wine_phys_word(resource,0x14)!=3||(wine_phys_word(resource,0x30)&7)||wine_phys_word(resource,0xb0)!=1||wine_phys_word(resource,0xb4)!=1)wine_handoff_refuse(0,40);
 if(wine_phys_word(resource,0xac))wine_handoff_refuse(0,41);
 u32 dcinfo=wine_phys_word(resource,0x11c);if(dcinfo&&wine_phys_word((void*)dcinfo,4))wine_handoff_refuse(0,42);
 if(!width||width>800||!height||height>600||mapped[0]<width*2||mapped[0]>32768||mapped[2]!=(client?client:heap))wine_handoff_refuse(0,43);
 if(size!=mapped[0]*height)wine_handoff_refuse(0,44);
 if(wine_phys_word((void*)subs,8)||wine_phys_word((void*)subs,12)!=size)wine_handoff_refuse(0,45);
 for(u32 i=0;i<2048;++i){if(wine_handoffs[i].submission&&wine_handoffs[i].resource==resource)wine_handoff_refuse(wine_handoffs+i,3);if(!wine_handoffs[i].submission)slot=i;}
 if(slot==2048)wine_handoff_refuse(0,11);
 struct WineSourceHandoff* s=wine_handoffs+slot;memset(s,0,sizeof(*s));s->submission=sample[2];s->texture=texture;s->resource=resource;s->texture_id=sample[6];s->generation=sample[7];s->resource_id=wine_resource_id(resource,1);s->width=width;s->height=height;s->pitch=mapped[0];s->size=size;s->old_client=client;s->old_data=mapped[2];s->thread=GetCurrentThreadId();s->state=1;
 struct WinePhysicalOwner* original=wine_phys_owner(s->old_data);struct WineHeapAllocation* allocation=wine_phys_heap_containing(s->old_data,size);
 if(original){if(s->old_data<original->data||size>original->size||s->old_data-original->data>original->size-size)wine_handoff_refuse(s,12);s->old_owner=original->id;}
 else if(client&&allocation){s->old_heap=allocation->id;s->old_heap_offset=client-allocation->pointer;s->old_heap_handle=(u32)allocation->heap;}
 else wine_handoff_refuse(s,12);
 if(!client&&(!original||original->resource!=resource||wine_phys_word(resource,0x54)!=original->base))wine_handoff_refuse(s,12);
 s->original_base=s->old_data&~4095u;s->original_bytes=((s->old_data+size+4095u)&~4095u)-s->original_base;
 if(s->old_data>~0u-size-4095u)wine_handoff_refuse(s,12);
 u32 m[7];if(VirtualQuery((void*)s->old_data,m,28)!=28)wine_handoff_refuse(s,12);s->original_allocation=m[1];
 for(u32 at=s->original_base;at<s->original_base+s->original_bytes;){if(VirtualQuery((void*)at,m,28)!=28||m[1]!=s->original_allocation||m[4]!=0x1000||(m[5]!=2&&m[5]!=4)||m[6]!=0x20000||m[0]+m[3]<=at||(s->prior&&s->prior!=m[5]))wine_handoff_refuse(s,12);s->prior=m[5];at=m[0]+m[3];}
 u32 free=2048;for(u32 i=0;i<2048;++i)if(!wine_phys_owners[i].id)free=i;
 if(free==2048||wine_phys_next==~0u)wine_handoff_refuse(s,11);
 u32 bytes=(size+16+4095)&~4095u;void* base=VirtualAlloc(0,bytes,0x3000,4);if(!base)wine_handoff_refuse(s,11);
 struct WinePhysicalOwner* stage=wine_phys_owners+free;stage->resource=resource;stage->base=(u32)base;stage->bytes=bytes;stage->size=size;stage->id=++wine_phys_next;stage->data=(u32)base+16;stage->allocation=(u32)base;stage->kind=1;s->stage_base=(u32)base;s->stage_owner=stage->id;wine_phys_emit(1,stage->id,0,1,size,0);
 s->epoch=++wine_handoff_count;wine_handoff_emit(s,1,0);s->state=2;wine_handoff_emit(s,2,0);wine_handoff_snapshot=slot+1;
 u32 old;wine_phys_internal=GetCurrentThreadId();if(!VirtualProtect((void*)s->original_base,s->original_bytes,2,&old)||old!=s->prior)wine_handoff_refuse(s,13);wine_phys_internal=0;
 /* There are no heap, Wine, journal or other original calls in this interval. */
 for(u32 y=0;y<height;++y){copy((u8*)packed+y*width*2,(const u8*)s->old_data+y*s->pitch,width*2);copy((u8*)base+16+y*s->pitch,(const u8*)s->old_data+y*s->pitch,width*2);}
 wine_phys_internal=GetCurrentThreadId();if(!VirtualProtect((void*)s->original_base,s->original_bytes,s->prior,&old)||old!=2)wine_handoff_refuse(s,13);wine_phys_internal=0;wine_handoff_snapshot=0;
 s->crc=wine_source_map_crc(packed,width*height*2);s->state=3;wine_handoff_emit(s,3,0);LeaveCriticalSection(journal_lock);return 1;
}
static void wine_handoff_bind(u32 submission){
 if(!wine_handoff_enabled)return;struct WineSourceHandoff* s=wine_handoff_find(submission);if(!s||s->state!=3)wine_handoff_refuse(s,14);
 u32 result=((u32(__attribute__((cdecl))*)(void*,u32,void*,u32))wine_source_version_rebind_original)(s->texture,0,(void*)(s->stage_base+16),s->pitch);
 EnterCriticalSection(journal_lock);if((i32)result<0||wine_phys_word(s->resource,0x58)||(wine_phys_word(s->resource,0xc4)&0x50)||wine_phys_word((void*)wine_phys_word(s->resource,0x120),24)!=2||wine_phys_word((void*)wine_phys_word(s->resource,0x120),0x30)!=s->stage_base+16)wine_handoff_refuse(s,14);LeaveCriticalSection(journal_lock);
 u32 crc=0xffffffffu;for(u32 y=0;y<s->height;++y)for(u32 x=0;x<s->width*2;++x){crc^=*((u8*)s->stage_base+16+y*s->pitch+x);for(u32 bit=0;bit<8;++bit)crc=(crc>>1)^(0xedb88320u&(0u-(crc&1)));}if((crc^0xffffffffu)!=s->crc)wine_handoff_refuse(s,14);
 wine_phys_acquire(submission,s->resource,0,s->stage_base+16,s->pitch,s->height,s->width);
 EnterCriticalSection(journal_lock);s->state=4;wine_handoff_emit(s,4,0);LeaveCriticalSection(journal_lock);
}
static void wine_handoff_finish(u32 submission){
 if(!wine_handoff_enabled)return;u32 start=GetTickCount();struct WineSourceHandoff* s=wine_handoff_find(submission);if(!s)return;
 for(;;){u32 pending=0;EnterCriticalSection(journal_lock);for(u32 i=0;i<2048;++i)if(wine_phys_leases[i].submission==submission)pending=1;LeaveCriticalSection(journal_lock);if(!pending)break;if(GetTickCount()-start>=5000)wine_handoff_refuse(s,15);Sleep(1);}
 EnterCriticalSection(journal_lock);if(s->state!=4)wine_handoff_refuse(s,16);s->state=5;wine_handoff_emit(s,5,0);s->state=6;LeaveCriticalSection(journal_lock);
 u32 result=((u32(__attribute__((cdecl))*)(void*,u32,void*,u32))wine_source_version_rebind_original)(s->texture,0,(void*)s->old_client,s->pitch);
 EnterCriticalSection(journal_lock);if((i32)result<0||s->detached||wine_phys_word((void*)wine_phys_word(s->resource,0x120),0x30)!=s->old_client)wine_handoff_refuse(s,16);
 if(!s->old_client){struct WinePhysicalOwner* o=wine_handoff_owner(s->old_owner);if(!o||wine_phys_word(s->resource,0x54)!=o->base||wine_phys_word(s->resource,0x58)!=o->data)wine_handoff_refuse(s,16);}
 wine_handoff_emit(s,6,0);u32 base=s->stage_base;s->state=7;LeaveCriticalSection(journal_lock);
 wine_phys_free((void*)base);EnterCriticalSection(journal_lock);wine_handoff_emit(s,7,0);memset(s,0,sizeof(*s));LeaveCriticalSection(journal_lock);
}
static int wine_handoff_open(u32 prerequisites){
 char flag[2];u32 n=GetEnvironmentVariableA("MNM_DRAW_WINE_SOURCE_HANDOFF",flag,2);if(!n)return 1;if(n!=1||flag[0]!='1'||!prerequisites)return 0;
 char name[600];path(name,"wine-source-handoff.bin");wine_handoff_file=CreateFileA(name,0x40000000,3,0,1,0x80,0);if(wine_handoff_file==(HANDLE)-1)return 0;
 u32 h[16]={0};copy(h,"MNMSHF01",8);h[2]=1;h[3]=256;h[4]=0x40209ca7;h[5]=131072;h[6]=2048;if(!write(wine_handoff_file,h,64))return 0;wine_handoff_enabled=1;return 1;
}
static void wine_handoff_close(u32 reason){
 if(!wine_handoff_enabled)return;for(u32 i=0;i<2048;++i)if(wine_handoffs[i].submission)wine_handoff_refuse(wine_handoffs+i,17);wine_handoff_emit(0,8,reason);wine_handoff_enabled=0;if(!CloseHandle(wine_handoff_file))ExitProcess(99);wine_handoff_file=0;
}
