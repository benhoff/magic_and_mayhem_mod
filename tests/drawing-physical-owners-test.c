/* Actual PE32 allocation/VM hooks, dispatch and full source/packet/fence chain. */
#define MNM_DRAWING_SELFTEST
#define DllMain PhysicalFixtureDllMain
#include "../runtime/drawing/session.c"
API void* WIN LoadLibraryA(const char*);
API HANDLE WIN CreateThread(void*,u32,u32(WIN*)(void*),void*,u32,u32*);
static u32 trial,forwards,packet[25],retire_case,ownership_case;
static u8 *source,*destination,*alias,*module;
static void* cached_protect;
static void* cached_free;
static u8* old_pixels;
static u32 initial_client;
static void* private_heap;
static u8* client_pixels;
static u8 expected_fp[512] __attribute__((aligned(16))),result_fp[512] __attribute__((aligned(16)));
static void check(int value){if(!value)ExitProcess(210);}
static void check_fp(const u8* expected){u8 actual[512] __attribute__((aligned(16)))={0};snapshot_fp(actual);check(equal(actual,expected,512));}
static u32 dc_forwards;
static u32 __attribute__((cdecl)) dc_body(void* texture,u32 sub,void* value){check(texture==source&&!sub&&!value);check_fp(expected_fp);check(GetLastError()==0x1234);++dc_forwards;__asm__ volatile("fninit; fld1":::"memory");snapshot_fp(result_fp);SetLastError(0x4321);return 0x8876086cu;}
static void dc_call(u32 kind){SetLastError(0x1234);__asm__ volatile("fninit; fldz":::"memory");snapshot_fp(expected_fp);check(((u32(__attribute__((cdecl))*)(void*,u32,void*))(module+(kind?0x220d30:0x218a80)))(source,0,0)==0x8876086cu);check(GetLastError()==0x4321);check_fp(result_fp);}
static void dc_entries(void){
 const u32 rv[6]={0x204490,0x204bc0,0x1df0d0,0x221460,0x218a80,0x220d30};
 const u8 prefix[6][7]={{0x55,0x89,0xe5,0x53,0x83,0xec,0x24},{0x55,0x89,0xe5,0x83,0xec,0x18,0},{0x55,0x89,0xe5,0x57,0x56,0x53,0},{0x8d,0x4c,0x24,0x04,0x83,0xe4,0xf8},{0x55,0x89,0xe5,0x57,0x56,0x53,0},{0x55,0x89,0xe5,0x57,0x56,0x53,0}};
 void** slots[6]={wine_resource_original+1,wine_resource_original+2,&wine_copy_original,&wine_source_version_rebind_original,wine_handoff_dc_original,wine_handoff_dc_original+1};
 for(u32 i=0;i<6;++i){copy(module+rv[i],prefix[i],i==0||i==3?7:6);*slots[i]=module+rv[i];}
 for(u32 i=4;i<6;++i){u8* site=module+rv[i];const u8 undo[4]={0x5b,0x5e,0x5f,0x5d};copy(site+6,undo,4);site[10]=0xe9;u32 delta=(u32)&dc_body-(u32)site-15;copy(site+11,&delta,4);}
 if(trial==41||trial==42){module[rv[trial-37]]^=1;check(!wine_mutator_entry_patch(module));for(u32 i=0;i<6;++i)check(module[rv[i]]!=0xe9);ExitProcess(0);}
 check(wine_mutator_entry_patch(module));
}
static u32 registry_new_id,registry_reuse;
static u8 WIN registry_free_body(void* heap,u32 flags,void* pointer){(void)flags;if(!registry_reuse)return 0;EnterCriticalSection(journal_lock);registry_new_id=wine_phys_heap_record(heap,pointer,64)->id;LeaveCriticalSection(journal_lock);return 1;}
static void* WIN registry_realloc_body(void* heap,u32 flags,void* pointer,u32 size){(void)flags;if(registry_reuse){EnterCriticalSection(journal_lock);registry_new_id=wine_phys_heap_record(heap,pointer,size)->id;LeaveCriticalSection(journal_lock);}return pointer;}
static void* WIN registry_destroy_body(void* heap){EnterCriticalSection(journal_lock);registry_new_id=wine_phys_heap_record(heap,(void*)0x12340,64)->id;LeaveCriticalSection(journal_lock);return 0;}
static void registry_retirement_races(void){
 void* heap=(void*)0x43210;void* pointer=(void*)0x12340;
 wine_phys_heap_original[0]=(void*)&registry_free_body;wine_phys_heap_original[1]=(void*)&registry_realloc_body;wine_phys_heap_original[2]=(void*)&registry_destroy_body;
 u32 old=wine_phys_heap_record(heap,pointer,32)->id;registry_reuse=1;check(wine_phys_heap_free(heap,0,pointer));check(registry_new_id>old&&wine_phys_heap_find(heap,pointer)->id==registry_new_id);
 registry_reuse=0;old=registry_new_id;check(!wine_phys_heap_free(heap,0,pointer));check(wine_phys_heap_find(heap,pointer)->id==old&&!wine_phys_heap_find(heap,pointer)->retiring);
 registry_reuse=1;check(wine_phys_heap_realloc(heap,0,pointer,128)==pointer);check(registry_new_id>old&&wine_phys_heap_find(heap,pointer)->id==registry_new_id);
 registry_reuse=0;old=registry_new_id;check(wine_phys_heap_realloc(heap,0,pointer,256)==pointer);check(wine_phys_heap_find(heap,pointer)->id>old&&wine_phys_heap_find(heap,pointer)->size==256);
 old=wine_phys_heap_find(heap,pointer)->id;check(!wine_phys_heap_destroy(heap));check(registry_new_id>old&&wine_phys_heap_find(heap,pointer)->id==registry_new_id);wine_phys_heap_forget(heap,pointer);check(!wine_phys_heap_find(heap,pointer)&&!wine_phys_heap_active);
}
static void registry_collisions(void){
 /* Force a wrapped cluster, delete its head/middle, and refill/reuse it. */
 void* heap=(void*)0x43210;u32 pointers[16],n=0;
 for(u32 p=16;n<16&&p<0x10000000u;p+=16)if(wine_phys_heap_hash(heap,(void*)p)==32765)pointers[n++]=p;
 check(n==16);
 for(u32 i=0;i<16;++i)check(wine_phys_heap_record(heap,(void*)pointers[i],32)!=0);
 for(u32 i=0;i<16;i+=2)wine_phys_heap_forget(heap,(void*)pointers[i]);
 for(u32 i=0;i<16;++i)check(!!wine_phys_heap_find(heap,(void*)pointers[i])==(i%2!=0));
 u32 epoch=wine_phys_heap_next;
 for(u32 i=0;i<16;i+=2)check(wine_phys_heap_record(heap,(void*)pointers[i],64)->id>epoch);
 for(u32 i=0;i<16;++i){check(wine_phys_heap_find(heap,(void*)pointers[i])!=0);wine_phys_heap_forget(heap,(void*)pointers[i]);}
 for(u32 i=0;i<32768;++i)check(!wine_phys_heap_allocations[i].id);
}
static void init_texture(u8* texture){
 u32* sub=VirtualAlloc(0,0x1000,0x3000,4);check(sub!=0);*(u32*)(texture+0x14)=3;*(u32*)(texture+0x4c)=3200;*(u32*)(texture+0xb0)=*(u32*)(texture+0xb4)=1;*(u32*)(texture+0x120)=(u32)sub;sub[3]=3200;sub[5]=0x80000000u;
 check(((u32(__attribute__((cdecl))*)(void*))(module+0x204a60))(texture)==1);
}
static u32 __attribute__((cdecl)) retire_body(void* texture){
 (void)texture;if(retire_case!=4&&retire_case!=6)ExitProcess(211);return 1;
}
static void* __attribute__((cdecl)) resource_get(void* t){
 if(t==alias&&retire_case==2)return source;
 if(t==alias&&retire_case==3)return 0;
 return t;
}
static u32 __attribute__((cdecl)) descriptor(void* t,u32 sub,u32* out){check(t==source&&!sub);memset(out,0,40);out[0]=116;out[5]=13;out[6]=out[7]=40;out[8]=1;out[9]=3200;
#ifdef MNM_DIRECT_BACKING_FIXTURE
 out[5]=2;
#endif
 return 0;}
static u32 __attribute__((cdecl)) map_body(void* t,u32 sub,u32* out,const u32* box,u32 flags){check(t==source&&!sub&&!box&&flags==0x80000000u);
#ifdef MNM_DIRECT_BACKING_FIXTURE
 ExitProcess(212);
#endif
 out[0]=80;out[1]=3200;out[2]=*(u32*)(*(u32*)(source+0x120)+0x30);if(!out[2])out[2]=*(u32*)(source+0x58);return 0;}
static u32 __attribute__((cdecl)) unmap_body(void* t,u32 sub){check(t==source&&!sub);return 0;}
static u32 __attribute__((cdecl)) handoff_rebind(void* texture,u32 sub,void* memory,u32 pitch){
 check(texture==source&&!sub&&pitch==80);
 if(retire_case==8){struct WineSourceHandoff* h=wine_handoff_find(1);if(h&&h->state==6)((volatile u8*)h->stage_base)[16]=9;}
 if(retire_case==5){struct WineSourceHandoff* h=wine_handoff_find(1);
  if(h&&h->state==6){wine_lifetime_texture_decref(source);ExitProcess(211);}}

 u32 old=*(u32*)(source+0x54);*(u32*)(source+0x58)=0;
 if(old){((void(__attribute__((cdecl))*)(void*))*(void**)(module+0x3396b0))((void*)old);*(u32*)(source+0x54)=0;}
 *(u32*)(*(u32*)(source+0x120)+0x30)=(u32)memory;
 *(u32*)(*(u32*)(source+0x120)+24)=2;*(u32*)(source+0xc4)&=~0x50u;
 if(!memory)check(((u32(__attribute__((cdecl))*)(void*))(module+0x204a60))(source)==1);
 return 0;
}
static void __attribute__((cdecl)) forward_free(void* p){(void)p;ExitProcess(211);}
static u32 WIN current_context(void){return 20;}
static void* WIN fence(u32 c,u32 f){check(c==0x9117&&!f);return (void*)21;}
static u32 WIN wait_sync(void* s,u32 f,unsigned long long n){check(s==(void*)21&&f==1&&n==1000000000ULL);return 0x911c;}
static void WIN delete_sync(void* s){check(s==(void*)21);}
static void* WIN get_proc(const char* n){if(equal(n,"glFenceSync",12))return (void*)&fence;if(equal(n,"glClientWaitSync",17))return (void*)&wait_sync;if(equal(n,"glDeleteSync",13))return (void*)&delete_sync;return 0;}
static void __attribute__((cdecl)) worker(void* cs,const void* p){
 check(cs==(void*)9&&p==packet);check_fp(expected_fp);check(GetLastError()==0x1234);++forwards;
 #ifdef MNM_DIRECT_BACKING_FIXTURE
 provider_fixture_worker();
#else
 u32 user=*(u32*)(*(u32*)(source+0x120)+0x30);check(*(u8*)(user?user:*(u32*)(source+0x58))==3);
#endif
 __asm__ volatile("fninit; fld1":::"memory");snapshot_fp(result_fp);SetLastError(0x4321);
}
static void __attribute__((cdecl)) nop(void* cs,const void* p){(void)cs;(void)p;check_fp(expected_fp);check(GetLastError()==0x1234);__asm__ volatile("fninit; fld1":::"memory");snapshot_fp(result_fp);SetLastError(0x4321);}
static u32 async_returning;
static u8 async_result_fp[512] __attribute__((aligned(16)));
static u32 WIN async_worker(void* unused){(void)unused;Sleep(50);check(__atomic_load_n(&async_returning,__ATOMIC_ACQUIRE));SetLastError(0x1234);restore_fp(expected_fp);((void(__attribute__((cdecl))*)(void*,const void*))*(void**)(module+0x24d420+42*4))((void*)9,packet);return 0;}
static u32 __attribute__((cdecl)) blit(void* cs,void* dst,u32 di,const void* db,void* src,u32 si,const void* sb,u32 flags,const void* fx,u32 filter){
 (void)di;(void)db;(void)si;(void)sb;(void)flags;(void)fx;(void)filter;check(cs==(void*)9&&dst==destination&&src==source);
 u32 user=*(u32*)(*(u32*)(source+0x120)+0x30);u8* pixels=(u8*)(user?user:*(u32*)(source+0x58));
 if(wine_handoff_enabled&&(trial==37||trial==38))dc_call(trial-37);
 if(trial==1||trial==22)pixels[0]^=1;
 if(wine_handoff_enabled&&trial==35)old_pixels[0]^=1;
 if(trial==2||trial==32){EnterCriticalSection(journal_lock);wine_reserve_write(alias,1);LeaveCriticalSection(journal_lock);if(wine_handoff_enabled)old_pixels[0]=7;}
 if(trial==3){u32 old;check(((u32(WIN*)(void*,void**,u32*,u32,u32*))cached_protect)((void*)-1,(void**)&pixels,&(u32){4096},4,&old)==0);}
 if(trial==4){void* base=(void*)*(u32*)(source+0x54);u32 size=0;((u32(WIN*)(void*,void**,u32*,u32))cached_free)((void*)-1,&base,&size,0x8000);}
 if(trial==5)((void(__attribute__((cdecl))*)(void*))*(void**)(module+0x3396b0))((void*)*(u32*)(source+0x54));
 if(trial==6){u32 p[25]={0};p[0]=43;p[1]=(u32)alias;wine_phys_cs(cs,p);}
 if(trial==7){u32 p[25]={0};p[0]=46;wine_phys_cs(cs,p);}
 if(trial==8){EnterCriticalSection(journal_lock);wine_phys_binding(pixels);LeaveCriticalSection(journal_lock);}
 if(trial==9)wine_phys_acquire(901,source,0,(u32)pixels,80,40,40);
 if(trial==23||trial==34)((u32(WIN*)(void*,u32,void*))GetProcAddress(GetModuleHandleA("ntdll.dll"),"RtlFreeHeap"))(private_heap,0,client_pixels);
 if(trial==24)((void*(WIN*)(void*,u32,void*,u32))GetProcAddress(GetModuleHandleA("ntdll.dll"),"RtlReAllocateHeap"))(private_heap,0,client_pixels,6400);
 if(trial==25)((void*(WIN*)(void*))GetProcAddress(GetModuleHandleA("ntdll.dll"),"RtlDestroyHeap"))(private_heap);
 if(trial==26){u32 n;((u32(WIN*)(void*,void*,const void*,u32,u32*))GetProcAddress(GetModuleHandleA("ntdll.dll"),"NtWriteVirtualMemory"))((void*)-1,pixels,pixels,1,&n);}
 if(trial==27){void* base=pixels;u32 size=4096;((u32(WIN*)(void*,void**,u32,u32*,u32,u32))GetProcAddress(GetModuleHandleA("ntdll.dll"),"NtAllocateVirtualMemory"))((void*)-1,&base,0,&size,0x80000,4);}
 if(trial==28)((u32(WIN*)(void*,u32,u32,void*,void*,u32))GetProcAddress(GetModuleHandleA("ntdll.dll"),"NtSetInformationVirtualMemory"))((void*)-1,0,0,0,0,0);
 if(trial==29)((u32(WIN*)(void*,void*))GetProcAddress(GetModuleHandleA("ntdll.dll"),"NtUnmapViewOfSection"))((void*)-1,pixels);
 if(trial==33){void* nt=GetModuleHandleA("ntdll.dll");void* heap=((void*(WIN*)(u32,void*,u32,u32,void*,void*))GetProcAddress(nt,"RtlCreateHeap"))(0,0,0,0,0,0);check(heap!=0);check(((void*(WIN*)(void*))GetProcAddress(nt,"RtlDestroyHeap"))(heap)==0);}
 if(trial==30)((u32(WIN*)(void))GetProcAddress(GetModuleHandleA("ntdll.dll"),"NtMapUserPhysicalPagesScatter"))();
 memset(packet,0,100);packet[0]=42;packet[1]=(u32)dst;packet[9]=(u32)src;packet[5]=packet[6]=packet[13]=packet[14]=40;packet[8]=packet[16]=1;packet[24]=1;
 u32 registers[9]={0};registers[7]=(u32)packet;wine_blit_packet_observe(registers);
 if(wine_handoff_enabled&&trial==40){HANDLE thread=CreateThread(0,0,async_worker,0,0,0);check(thread!=0);CloseHandle(thread);__asm__ volatile("fninit; fld1":::"memory");snapshot_fp(async_result_fp);SetLastError(0x4321);__atomic_store_n(&async_returning,1,__ATOMIC_RELEASE);return 0;}
 if(wine_handoff_enabled&&trial==36){__asm__ volatile("fninit; fld1":::"memory");snapshot_fp(result_fp);SetLastError(0x4321);return 0;}
 SetLastError(0x1234);restore_fp(expected_fp);((void(__attribute__((cdecl))*)(void*,const void*))*(void**)(module+0x24d420+42*4))(cs,packet);
 if(trial==9){EnterCriticalSection(journal_lock);wine_phys_release(901);LeaveCriticalSection(journal_lock);}
 if(retire_case==12||retire_case==13){struct WineSourceHandoff* h=wine_handoff_find(1);check(h&&h->state==4&&!wine_reserve_find(1));
  if(retire_case==12)wine_phys_free((void*)h->stage_base);
  else {u32 n;((u32(WIN*)(void*,void*,const void*,u32,u32*))GetProcAddress(GetModuleHandleA("ntdll.dll"),"NtWriteVirtualMemory"))((void*)-1,(void*)(h->stage_base+16),old_pixels,1,&n);}
 }
 if(retire_case==10||retire_case==11){struct WineSourceHandoff* h=wine_handoff_find(1);check(h&&h->state==4&&!wine_reserve_find(1));
  if(retire_case==10)wine_phys_binding((void*)(h->stage_base+16));
  else {*(u32*)(*(u32*)(alias+0x120)+0x30)=h->stage_base+16;wine_phys_writer(alias,43);}
 }
 if(retire_case==7||retire_case==9){struct WineSourceHandoff* h=wine_handoff_find(1);check(h&&h->state==4&&!wine_reserve_find(1));
  if(retire_case==7)((volatile u8*)h->stage_base)[16]=9;
  else {u32 old;check(VirtualProtect((void*)h->stage_base,4096,4,&old));}
 }
 if(retire_case&&retire_case<=4){
  check(wine_handoff_find(1)&&wine_handoff_find(1)->state==4&&!wine_reserve_find(1));
  check(wine_lifetime_texture_decref(retire_case==1?source:retire_case==4?destination:alias)==1);
 }

 return 0;
}
void start(void){
 char value[4];if(GetEnvironmentVariableA("MNM_TEST_OWNERSHIP",value,4)==1)ownership_case=value[0];
 if(ownership_case=='I'){check(!initialize());ExitProcess(0);}check(initialize());
 if(ownership_case=='5'){wine_source_ownership_require();ExitProcess(211);}
 if(GetEnvironmentVariableA("MNM_TEST_PHYSICAL",value,4)==1)trial=value[0]>='a'?value[0]-'a'+10:value[0]-'0';
 if(trial==39){registry_collisions();registry_retirement_races();finish(1);ExitProcess(0);}
 if(trial==19){check(!wine_phys_enabled&&wine_phys_install(0));finish(1);ExitProcess(0);}
 if(GetEnvironmentVariableA("MNM_TEST_RETIRE",value,4)==1)retire_case=value[0]-'0';
 wine_lifetime_original[1]=(void*)&retire_body;
 module=VirtualAlloc(0,0x340000,0x3000,0x40);check(module!=0);
 const u32 rv[51]={0x1cf2c0,0x1cf2f0,0x1ce7b0,0x1cea70,0x1ceb00,0x1cf070,0x1d0510,0x1d0da0,0x1d0820,0x1d0590,0x1cfe60,0x1d0d70,0x1d0a60,0x1d0a00,0x1d0060,0x1cfcf0,0x1d0af0,0x1d0960,0x1d0ca0,0x1d07a0,0x1d08a0,0x1cfa60,0x1cfe00,0x1d0530,0x1cfdc0,0x1d0560,0x1d0c50,0x1cfb00,0x1d00c0,0x1d0480,0x1d0010,0x1cffc0,0x1d0040,0x1cf800,0x1cf9a0,0x1ce790,0x1cf8e0,0x1cf2d0,0x1d0e20,0x1cf1d0,0x1d0e40,0x1cf220,0x1ce280,0x1d0e70,0x1ce130,0x1ce820,0x1ce870,0x1ce920,0x1cf130,0x1ce9c0,0x1ced00};
 for(u32 i=0;i<51;++i)*(void**)(module+0x24d420+i*4)=i==42?(void*)&wine_blit_worker:module+rv[i];
 const u8 prologue[6]={0x55,0x89,0xe5,0x83,0xec,0x10};copy(module+0x204a60,prologue,6);
 void* crt=LoadLibraryA("ucrtbase.dll");check(crt!=0);*(void**)(module+0x3396b0)=GetProcAddress(crt,"free");
 cached_protect=GetProcAddress(GetModuleHandleA("ntdll.dll"),"NtProtectVirtualMemory");cached_free=GetProcAddress(GetModuleHandleA("ntdll.dll"),"NtFreeVirtualMemory");
 if(trial==17||trial==18){module[0x204a60]^=trial==17?1:0;if(trial==18)*(void**)(module+0x24d420+50*4)=0;check(!wine_phys_install(module));check(*(u8*)cached_protect==0xb8);ExitProcess(0);}
 check(wine_phys_install(module));if(wine_handoff_enabled)dc_entries();wine_phys_free_original=(void*)&forward_free;wine_phys_cs_original[0]=(void*)&nop;
 source=VirtualAlloc(0,4096,0x3000,4);destination=VirtualAlloc(0,4096,0x3000,4);alias=VirtualAlloc(0,4096,0x3000,4);check(source&&destination&&alias);init_texture(source);init_texture(destination);init_texture(alias);
 memset((void*)*(u32*)(source+0x58),3,3200);u32* alias_sub=(u32*)*(u32*)(alias+0x120);alias_sub[12]=*(u32*)(source+0x58);
 wine_resource_original[0]=(void*)&resource_get;wine_resource_original[1]=(void*)&map_body;wine_resource_original[2]=(void*)&unmap_body;wine_source_descriptor_get=&descriptor;wine_submission_original[0]=(void*)&blit;wine_blit_worker_original=(void*)&worker;wine_gpu_current=(void*)&current_context;wine_gpu_proc=&get_proc;wine_source_version_rebind_original=(void*)&handoff_rebind;old_pixels=(u8*)*(u32*)(source+0x58);
 if(wine_source_ownership_required){check(wine_source_ownership_install());
  if(ownership_case=='6')wine_mutator_entry_installed=0;
  if(ownership_case=='9')wine_handoff_enabled=0;
 }
 wine_lifetime_create(source,1);wine_lifetime_create(destination,1);wine_lifetime_create(alias,1);
 if(trial==10)*(u32*)(source+0x30)=1;
 if(trial==11)*(u32*)(*(u32*)(source+0x120)+0x30)=*(u32*)(source+0x58);
 if(trial==12)wine_phys_active=1;
 if(trial==13){u32 oldid=wine_phys_owner(*(u32*)(destination+0x58))->id;wine_phys_free((void*)*(u32*)(destination+0x54));*(u32*)(destination+0x54)=*(u32*)(destination+0x58)=0;check(wine_phys_prepare(destination));check(wine_phys_owner(*(u32*)(destination+0x58))->id>oldid);}
 if(trial==14){u32 p[1]={0};SetLastError(0x1234);__asm__ volatile("fninit; fldz":::"memory");snapshot_fp(expected_fp);wine_phys_cs((void*)9,p);check(GetLastError()==0x4321);check_fp(result_fp);}
 if(trial==15){u32 mapped=(u32)VirtualAlloc(0,4096,0x3000,4);*(u32*)(source+0x58)=mapped;}
 if(trial==16){HANDLE mapping=CreateFileMappingA((HANDLE)-1,0,4,0,4096,0);check(mapping!=0);u8* a=MapViewOfFile(mapping,2,0,0,4096);u8* b=MapViewOfFile(mapping,2,0,0,4096);check(a&&b&&a!=b);a[0]=47;check(b[0]==47);*(u32*)(source+0x58)=(u32)a;}
 if(trial==20){u32 old;check(VirtualProtect((void*)*(u32*)(source+0x54),4096,2,&old));}
 if((trial>=21&&trial<=25)||trial==31||trial==32||trial==34){
  void* nt=GetModuleHandleA("ntdll.dll");private_heap=((void*(WIN*)(u32,void*,u32,u32,void*,void*))GetProcAddress(nt,"RtlCreateHeap"))(0,0,0,0,0,0);check(private_heap!=0);
  client_pixels=((void*(WIN*)(void*,u32,u32))GetProcAddress(nt,"RtlAllocateHeap"))(private_heap,0,20000);check(client_pixels!=0);memset(client_pixels,3,20000);if(trial==31){void* grown=((void*(WIN*)(void*,u32,void*,u32))GetProcAddress(nt,"RtlReAllocateHeap"))(private_heap,0,client_pixels,40000);check(grown!=0);client_pixels=grown;memset(client_pixels,3,3200);}
  u32 view=(u32)client_pixels+(trial==34?8192:0);*(u32*)(*(u32*)(source+0x120)+0x30)=view;alias_sub[12]=view;
 }
 if(wine_handoff_enabled){dc_call(0);dc_call(1);check(dc_forwards==2);}
 initial_client=*(u32*)(*(u32*)(source+0x120)+0x30);old_pixels=(u8*)(initial_client?initial_client:*(u32*)(source+0x58));
#ifdef MNM_BACKING_PROVIDER_FIXTURE
 provider_fixture_setup();
#endif
 SetLastError(0x1234);__asm__ volatile("fninit; fldz":::"memory");snapshot_fp(expected_fp);
 check(wine_submission_blt((void*)9,destination,0,0,source,0,0,0,0,1)==0);check(forwards==1);
#ifdef MNM_BACKING_PROVIDER_FIXTURE
 provider_fixture_check();
#endif
 #ifndef MNM_DIRECT_BACKING_FIXTURE
 if(wine_handoff_enabled){check(!wine_handoff_find(1));if(!initial_client)check(*(u32*)(source+0x58)==(u32)old_pixels);else check(*(u32*)(*(u32*)(source+0x120)+0x30)==initial_client);if(trial==35)check(old_pixels[0]==2);if(trial==2)check(old_pixels[0]==7);}
 #endif
 check(GetLastError()==0x4321);check_fp(trial==40?async_result_fp:result_fp);
 if(retire_case==6)check(wine_lifetime_texture_decref(source)==1);
 if(ownership_case=='7'||ownership_case=='8'){finish(1);
  if(ownership_case=='7')wine_submission_blt((void*)9,destination,0,0,source,0,0,0,0,1);
  else wine_blit_worker((void*)9,packet);ExitProcess(211);
 }


 if(trial==20){u32 old;check(VirtualProtect((void*)*(u32*)(source+0x54),4096,4,&old));}
 #ifndef MNM_DIRECT_BACKING_FIXTURE
 {u32 current=*(u32*)(*(u32*)(source+0x120)+0x30);if(!current)current=*(u32*)(source+0x58);((u8*)current)[0]=4;}
#endif
 if(trial==43){
  char name[600];path(name,"stop.flag");HANDLE stop=CreateFileA(name,0x40000000,3,0,1,0x80,0);check(stop!=(HANDLE)-1&&CloseHandle(stop));
  wine_phys_active=1;u32 args[6]={0};struct Context c=begin(0,1,args,0,0,3);end(&c,0,0);check(journal&&enabled&&wine_phys_enabled);
  wine_phys_active=0;struct Context next=begin(0,1,args,0,0,3);end(&next,0,0);check(!journal&&!enabled);
 }else wine_submission_close(1);
 check(!wine_phys_enabled&&!wine_phys_any());
 if(dc_forwards){dc_call(0);dc_call(1);check(dc_forwards==4);}
 if(*(u32*)(source+0x54))wine_phys_free((void*)*(u32*)(source+0x54));ExitProcess(0);
}
