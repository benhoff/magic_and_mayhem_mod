/* Bounded owned rejection evidence. Continuous drawing only copies records;
 * a separate logging worker owns file I/O. Names and values remain immutable. */
#define MNM_LOCK_DIAGNOSTICS_DEFERRED 1
struct LockDiagnostic {const char* reason;char name[41];u32 values[19];};
static struct LockDiagnostic lock_diagnostic_seen[128];
static u32 lock_diagnostic_count,lock_diagnostic_read,lock_diagnostic_failed;
static volatile i32 lock_diagnostic_busy;
static volatile i32 lock_diagnostic_draining;
static HANDLE lock_diagnostic_file;
static char lock_diagnostic_path[544];
static void lock_diagnostic_drain(void){
    if(!lock_capture_path_length || !__sync_bool_compare_and_swap(&lock_diagnostic_draining,0,1))return;
    u32 saved=GetLastError();
    if(__atomic_load_n(&lock_diagnostic_failed,__ATOMIC_ACQUIRE))goto done;
    /* Capture one finite prefix. Producers never wait on the file writer and
     * cannot replace records while this writer uses their owned bytes. */
    u32 end=__atomic_load_n(&lock_diagnostic_count,__ATOMIC_ACQUIRE);
    while(lock_diagnostic_read<end){
        const struct LockDiagnostic* record=lock_diagnostic_seen+lock_diagnostic_read;
        if(!lock_diagnostic_file){
            copy(lock_diagnostic_path,lock_capture_path,lock_capture_path_length);
            copy(lock_diagnostic_path+lock_capture_path_length,"\\lifecycle.log",15);
            lock_diagnostic_file=CreateFileA(lock_diagnostic_path,0x40000000,1,0,1,0x80,0);
            if(lock_diagnostic_file==(HANDLE)-1){lock_diagnostic_file=0;__atomic_store_n(&lock_diagnostic_failed,1,__ATOMIC_RELEASE);break;}
        }
        char line[256];u32 length=0;const char* at=record->name;
        while(*at && length<40)line[length++]=*at++;
        line[length++]=' ';
        for(u32 i=0;i<19;++i){length+=failure_hex(line+length,record->values[i]);line[length++]=' ';}
        line[length++]='\r';line[length++]='\n';
        if(!write_all(lock_diagnostic_file,line,length)){__atomic_store_n(&lock_diagnostic_failed,1,__ATOMIC_RELEASE);break;}
        ++lock_diagnostic_read;
    }
 done:SetLastError(saved);__sync_lock_release(&lock_diagnostic_draining);
}
static void lock_diagnostic_values(const char* reason,const u32* values){
    if(!lock_capture_path_length || !__sync_bool_compare_and_swap(&lock_diagnostic_busy,0,1))return;
    u32 saved=GetLastError(),published=0;
    u32 repeated=0,count=__atomic_load_n(&lock_diagnostic_count,__ATOMIC_RELAXED);
    if(count>=128 || __atomic_load_n(&lock_diagnostic_failed,__ATOMIC_ACQUIRE))goto done;
    for(u32 i=0;i<count;++i){
        if(lock_diagnostic_seen[i].reason!=reason)continue;
        ++repeated;if(same(values,lock_diagnostic_seen[i].values,sizeof(lock_diagnostic_seen[i].values)))goto done;
    }
    if(repeated>=4)goto done;
    struct LockDiagnostic* record=lock_diagnostic_seen+count;
    record->reason=reason;u32 length=0;
    while(length<40 && reason[length]){record->name[length]=reason[length];++length;}
    record->name[length]=0;copy(record->values,values,sizeof(record->values));
    __atomic_store_n(&lock_diagnostic_count,count+1,__ATOMIC_RELEASE);published=1;
 done:
    /* Finite captures keep the original nonwaiting producer guard through
     * delivery: a concurrent producer drops its optional record, rather than
     * accepting bytes that have no worker to deliver them after this drain. */
    if(!game_session_continuous)lock_diagnostic_drain();
    __sync_lock_release(&lock_diagnostic_busy);
    if(game_session_continuous && published)WakeByAddressAll(&lock_diagnostic_count);
    SetLastError(saved);
}
static void lock_diagnostic(const char* reason,void* object,u32 kind,u32 argument,u32 flags,i32 result,u32 owner,const u32* d){
    u32 values[19]={(u32)object,GetCurrentThreadId(),kind,argument,flags,(u32)result,owner,0};
    if(d){values[7]=d[0];values[8]=d[1];values[9]=d[3];values[10]=d[2];values[11]=d[4];values[12]=d[9];
        values[13]=d[19];values[14]=d[21];values[15]=d[22];values[16]=d[23];values[17]=d[24];values[18]=d[26];}
    lock_diagnostic_values(reason,values);
}
