/* Opt-in diagnostics only. Immutable bounded slots; drawing callbacks never
 * perform file I/O or wait for the diagnostic worker. No pointer dereferences. */
API int WIN QueryPerformanceCounter(unsigned long long*);
API int WIN QueryPerformanceFrequency(unsigned long long*);
#define COPY_TRACE_LIMIT 512u
#define COPY_TRACE_WORDS 25u
enum { COPY_TRACE_WAIT=1,COPY_TRACE_CLOCK=2,COPY_TRACE_OWNER=3,
       COPY_TRACE_DROP=4,COPY_TRACE_CHECKPOINT=5,COPY_TRACE_RECOVERY=6 };
struct CopyTraceSlot {u32 ready,words[COPY_TRACE_WORDS];};
static struct CopyTraceSlot copy_trace_slots[COPY_TRACE_LIMIT];
static u32 copy_trace_enabled,copy_trace_written,copy_trace_read,copy_trace_dropped,copy_trace_wait_id,copy_trace_armed;
static HANDLE copy_trace_file;
static char copy_trace_path[544];
static unsigned long long copy_trace_frequency;
static u32 copy_trace_owner_sequence,copy_trace_owner_ready,copy_trace_acquisitions;
/* thread,acquisition,startTick,site,target,phase,startQpcLow,startQpcHigh */
static u32 copy_trace_owner[8];
static unsigned long long copy_trace_clock(void){
    u32 error=GetLastError();unsigned long long value=0;
    if(copy_trace_frequency)QueryPerformanceCounter(&value);
    SetLastError(error);return value;
}
static void copy_trace_init(const char* directory,u32 length){
    u32 error=GetLastError();char flag[2];
    if(directory && length && length+18<sizeof(copy_trace_path) &&
       GetEnvironmentVariableA("MNM_RENDER_COPY_TIMING",flag,sizeof(flag))==1 && flag[0]=='1'){
        copy(copy_trace_path,directory,length);
        copy(copy_trace_path+length,"\\copy-timing.bin",17);
        if(!QueryPerformanceFrequency(&copy_trace_frequency))copy_trace_frequency=0;
        copy_trace_enabled=1;
    }
    SetLastError(error);
}
static void copy_trace_record(u32 event,u32 session,const u32* values){
    if(!copy_trace_enabled)return;
    if(event==COPY_TRACE_WAIT || event==COPY_TRACE_CHECKPOINT || event==COPY_TRACE_RECOVERY)
        __atomic_store_n(&copy_trace_armed,1,__ATOMIC_RELEASE);
    /* Ordinary replacements are not invalidations. Start provenance after a
     * failure/recovery boundary and reserve half the slots for fault timing. */
    if(event==COPY_TRACE_DROP && (!values[0] || !__atomic_load_n(&copy_trace_armed,__ATOMIC_ACQUIRE)))return;
    u32 error=GetLastError(),at=__atomic_load_n(&copy_trace_written,__ATOMIC_RELAXED);
    for(;;){
        if(at>=COPY_TRACE_LIMIT || (event==COPY_TRACE_DROP && at>=COPY_TRACE_LIMIT/2)){
            __atomic_add_fetch(&copy_trace_dropped,1,__ATOMIC_RELAXED);SetLastError(error);return;}
        if(__atomic_compare_exchange_n(&copy_trace_written,&at,at+1,0,__ATOMIC_RELAXED,__ATOMIC_RELAXED))break;
    }
    struct CopyTraceSlot* s=copy_trace_slots+at;unsigned long long clock=copy_trace_clock();
    s->words[0]=event;s->words[1]=session;s->words[2]=GetCurrentThreadId();s->words[3]=GetTickCount();
    s->words[4]=(u32)clock;s->words[5]=(u32)(clock>>32);copy(s->words+6,values,19*4);
    __atomic_store_n(&s->ready,1,__ATOMIC_RELEASE);SetLastError(error);
}
/* Publication worker only; recovery joins the old worker before starting another.
 * An unpublished slot pauses this finite drain, never the producer. */
static void copy_trace_drain(void){
    if(!copy_trace_enabled || copy_trace_read>=COPY_TRACE_LIMIT)return;
    u32 error=GetLastError();
    while(copy_trace_read<__atomic_load_n(&copy_trace_written,__ATOMIC_ACQUIRE)){
        struct CopyTraceSlot* s=copy_trace_slots+copy_trace_read;
        if(!__atomic_load_n(&s->ready,__ATOMIC_ACQUIRE))break;
        if(!copy_trace_file){
            copy_trace_file=CreateFileA(copy_trace_path,0x40000000,1,0,1,0x80,0);
            if(copy_trace_file==(HANDLE)-1){copy_trace_file=0;break;}
            u32 header[8]={0x434d4e4d,0x3159504f,1,COPY_TRACE_WORDS*4,
                (u32)copy_trace_frequency,(u32)(copy_trace_frequency>>32),COPY_TRACE_LIMIT,0};
            if(!write_all(copy_trace_file,header,sizeof(header))){CloseHandle(copy_trace_file);copy_trace_file=0;break;}
        }
        if(!write_all(copy_trace_file,s->words,sizeof(s->words)))break;
        ++copy_trace_read;
    }
    SetLastError(error);
}
static void copy_trace_owner_open(void* target,u32 site,u32 thread,u32 tick){
    if(!copy_trace_enabled)return;
    u32 error=GetLastError();unsigned long long clock=copy_trace_clock();
    __atomic_add_fetch(&copy_trace_owner_sequence,1,__ATOMIC_ACQ_REL);
    __atomic_store_n(&copy_trace_owner_ready,0,__ATOMIC_RELAXED);
    u32 values[8]={thread,__atomic_add_fetch(&copy_trace_acquisitions,1,__ATOMIC_RELAXED),tick,site,(u32)target,0,(u32)clock,(u32)(clock>>32)};
    for(u32 i=0;i<8;++i)__atomic_store_n(copy_trace_owner+i,values[i],__ATOMIC_RELAXED);
    __atomic_store_n(&copy_trace_owner_ready,1,__ATOMIC_RELAXED);
    __atomic_add_fetch(&copy_trace_owner_sequence,1,__ATOMIC_RELEASE);SetLastError(error);
}
static void copy_trace_owner_phase(u32 phase){
    if(!copy_trace_enabled)return;
    __atomic_add_fetch(&copy_trace_owner_sequence,1,__ATOMIC_ACQ_REL);
    __atomic_store_n(copy_trace_owner+5,phase,__ATOMIC_RELAXED);
    __atomic_add_fetch(&copy_trace_owner_sequence,1,__ATOMIC_RELEASE);
}
static void copy_trace_owner_close(void){
    if(copy_trace_enabled)__atomic_store_n(&copy_trace_owner_ready,0,__ATOMIC_RELEASE);
}
static int copy_trace_sample(const u32* owner,u32* fields){
    for(u32 retry=0;retry<8;++retry){
        u32 sequence=__atomic_load_n(&copy_trace_owner_sequence,__ATOMIC_ACQUIRE);
        if(sequence&1)continue;
        u32 thread=__atomic_load_n(owner,__ATOMIC_ACQUIRE);
        for(u32 i=0;i<8;++i)fields[i]=__atomic_load_n(copy_trace_owner+i,__ATOMIC_RELAXED);
        __atomic_thread_fence(__ATOMIC_ACQUIRE);
        if(sequence==__atomic_load_n(&copy_trace_owner_sequence,__ATOMIC_ACQUIRE) &&
           __atomic_load_n(&copy_trace_owner_ready,__ATOMIC_ACQUIRE) && thread && fields[0]==thread &&
           thread==__atomic_load_n(owner,__ATOMIC_ACQUIRE))return 1;
    }
    zero(fields,8*4);return 0;
}
struct CopyWaitTiming {u32 attempts,waits,wakes,startAcquisitions,startTick,lastOwner,changes,coherent,missed,maxWait;
    unsigned long long startClock,lastWaitClock,lastWakeClock;};
static void copy_wait_begin(struct CopyWaitTiming* t,u32 tick){
    if(!copy_trace_enabled)return;
    zero(t,sizeof(*t));t->startTick=tick;t->startAcquisitions=__atomic_load_n(&copy_trace_acquisitions,__ATOMIC_RELAXED);t->startClock=copy_trace_clock();
}
static void copy_wait_sample(struct CopyWaitTiming* t,const u32* owner){
    if(!copy_trace_enabled)return;
    ++t->attempts;u32 fields[8];
    if(copy_trace_sample(owner,fields)){++t->coherent;if(t->lastOwner && t->lastOwner!=fields[0])++t->changes;t->lastOwner=fields[0];}
    else ++t->missed;
}
static void copy_wait_before(struct CopyWaitTiming* t){
    if(copy_trace_enabled){++t->waits;t->lastWaitClock=copy_trace_clock();}
}
static void copy_wait_after(struct CopyWaitTiming* t,u32 tick,u32 returned){
    if(!copy_trace_enabled)return;
    t->lastWakeClock=copy_trace_clock();if(returned)++t->wakes;
    u32 span=GetTickCount()-tick;if(span>t->maxWait)t->maxWait=span;
}
static void copy_wait_timeout(struct CopyWaitTiming* t,const u32* owner,u32 session,void* target,u32 site){
    if(!copy_trace_enabled)return;
    u32 error=GetLastError(),fields[8];int coherent=copy_trace_sample(owner,fields);
    u32 id=__atomic_add_fetch(&copy_trace_wait_id,1,__ATOMIC_RELAXED);
    u32 values[19]={site,(u32)target,GetTickCount()-t->startTick,t->attempts,t->waits,t->wakes,
        t->startAcquisitions,__atomic_load_n(&copy_trace_acquisitions,__ATOMIC_RELAXED),t->changes,t->coherent,t->missed,t->maxWait,(u32)coherent};values[18]=id;
    copy_trace_record(COPY_TRACE_WAIT,session,values);
    zero(values,sizeof(values));values[0]=site;values[1]=(u32)target;
    values[2]=(u32)t->startClock;values[3]=(u32)(t->startClock>>32);
    values[4]=(u32)t->lastWaitClock;values[5]=(u32)(t->lastWaitClock>>32);
    values[6]=(u32)t->lastWakeClock;values[7]=(u32)(t->lastWakeClock>>32);
    values[18]=id;
    copy_trace_record(COPY_TRACE_CLOCK,session,values);
    zero(values,sizeof(values));values[0]=site;values[1]=(u32)target;values[2]=(u32)coherent;copy(values+3,fields,sizeof(fields));values[18]=id;
    copy_trace_record(COPY_TRACE_OWNER,session,values);SetLastError(error);
}
