/* Read-only-forwarding provider allocation observation. Only checked IAT slots
 * in the two hash-pinned test-process modules are swapped. No allocation,
 * logging API or original game call is made inside an observer callback. */
struct DriverAllocationEvent {u32 op,phase,address,a,b,caller;};
static struct DriverAllocationEvent allocation_events[131072];
static volatile u32 allocation_count,allocation_phase,allocation_overflow;
struct DriverAllocationModule {u8* base;void** calloc_slot;void** free_slot;void* calloc_real;void* free_real;};
static struct DriverAllocationModule allocation_modules[2];
static u32 allocation_event(u32 op,u32 address,u32 a,u32 b,u32 caller){
    u32 index=__sync_fetch_and_add(&allocation_count,1);
    if(index>=131072){allocation_overflow=1;return index;}
    struct DriverAllocationEvent* event=&allocation_events[index];
    event->op=op;event->phase=allocation_phase;event->address=address;event->a=a;event->b=b;event->caller=caller;
    return index;
}
static void* observe_calloc(u32 module,u32 count,u32 size,u32 caller){
    typedef void*(*Allocate)(u32,u32);void* result=((Allocate)allocation_modules[module].calloc_real)(count,size);
    allocation_event(100+module,(u32)result,count,size,caller);return result;
}
static void observe_free(u32 module,void* memory,u32 caller){
    typedef void(*Free)(void*);u32 index=allocation_event(110+module,(u32)memory,0,0,caller);
    ((Free)allocation_modules[module].free_real)(memory);
    allocation_event(120+module,(u32)memory,index,0,caller);
}
static void* observer_calloc0(u32 count,u32 size){return observe_calloc(0,count,size,(u32)__builtin_return_address(0));}
static void* observer_calloc1(u32 count,u32 size){return observe_calloc(1,count,size,(u32)__builtin_return_address(0));}
static void observer_free0(void* memory){observe_free(0,memory,(u32)__builtin_return_address(0));}
static void observer_free1(void* memory){observe_free(1,memory,(u32)__builtin_return_address(0));}
static int allocation_name(const char* a,const char* b){u32 n=0;while(n<128&&a[n]&&a[n]==b[n])++n;return n<128&&a[n]==b[n];}
static void allocation_swap(void** slot,void* expected,void* replacement){
    u32 old,ignored;original_require(*slot==expected,230);
    original_require(VirtualProtect(slot,4,4,&old),231);*slot=replacement;
    original_require(VirtualProtect(slot,4,old,&ignored),232);
}
static void allocation_install(u32 module,const char* name){
    struct DriverAllocationModule* entry=&allocation_modules[module];entry->base=GetModuleHandleA(name);original_require(entry->base!=0,233);
    u8* base=entry->base;u32 pe=*(u32*)(base+60),length=*(u32*)(base+pe+24+56);
    original_require(*(u16*)base==0x5a4d&&pe<length-256&&*(u32*)(base+pe)==0x4550&&*(u16*)(base+pe+4)==0x14c&&*(u16*)(base+pe+24)==0x10b,234);
    u32 rva=*(u32*)(base+pe+24+104);original_require(rva<length-20,235);
    u32* descriptor=(u32*)(base+rva);
    for(u32 libraries=0;libraries<256&&descriptor[0];++libraries,descriptor+=5){
        original_require(descriptor[0]<length-4&&descriptor[4]<length-4&&descriptor[3]<length-1,236);
        HANDLE library=GetModuleHandleA((char*)base+descriptor[3]);original_require(library!=0,237);
        u32* lookup=(u32*)(base+descriptor[0]);void** address=(void**)(base+descriptor[4]);
        for(u32 i=0;i<8192&&lookup[i];++i){
            if(lookup[i]&0x80000000)continue;original_require(lookup[i]<length-2,238);
            const char* imported=(char*)base+lookup[i]+2;
            if(allocation_name(imported,"calloc")){original_require(!entry->calloc_slot,239);entry->calloc_slot=address+i;entry->calloc_real=GetProcAddress(library,"calloc");original_require(entry->calloc_real!=0&&*entry->calloc_slot==entry->calloc_real,240);}
            if(allocation_name(imported,"free")){original_require(!entry->free_slot,241);entry->free_slot=address+i;entry->free_real=GetProcAddress(library,"free");original_require(entry->free_real!=0&&*entry->free_slot==entry->free_real,242);}
        }
    }
    original_require(entry->calloc_slot&&entry->free_slot,243);
    allocation_swap(entry->calloc_slot,entry->calloc_real,module?(void*)observer_calloc1:(void*)observer_calloc0);
    allocation_swap(entry->free_slot,entry->free_real,module?(void*)observer_free1:(void*)observer_free0);
    emit(81,99,module,(u32)base,length,(u32)entry->calloc_real);
}
static void allocation_finish(void){
    for(u32 module=0;module<2;++module){struct DriverAllocationModule* entry=&allocation_modules[module];
        allocation_swap(entry->calloc_slot,module?(void*)observer_calloc1:(void*)observer_calloc0,entry->calloc_real);
        allocation_swap(entry->free_slot,module?(void*)observer_free1:(void*)observer_free0,entry->free_real);
    }
    original_require(!allocation_overflow&&allocation_count<131072,244);
    HANDLE events=CreateFileA("provider-allocations.bin",0x40000000,0,0,1,0,0);original_require(events!=(HANDLE)-1,245);
    u32 written,size=allocation_count*sizeof(struct DriverAllocationEvent);
    original_require(WriteFile(events,allocation_events,size,&written,0)&&written==size,246);CloseHandle(events);emit(82,99,0,0,allocation_count,0);
}
