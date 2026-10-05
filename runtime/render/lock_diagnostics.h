/* Bounded rejection evidence. No COM calls; successful repeats cannot fill the log. */
struct LockDiagnostic {const char* reason;u32 values[19];};
static struct LockDiagnostic lock_diagnostic_seen[128];
static u32 lock_diagnostic_count;
static volatile i32 lock_diagnostic_busy;
static HANDLE lock_diagnostic_file;
static char lock_diagnostic_path[544];
static void lock_diagnostic_values(const char* reason,const u32* values){
    if(!lock_capture_path_length || !__sync_bool_compare_and_swap(&lock_diagnostic_busy,0,1))return;
    u32 saved=GetLastError();
    u32 repeated=0;
    if(lock_diagnostic_count>=128)goto done;
    for(u32 i=0;i<lock_diagnostic_count;++i){
        if(lock_diagnostic_seen[i].reason!=reason)continue;
        ++repeated;if(same(values,lock_diagnostic_seen[i].values,sizeof(lock_diagnostic_seen[i].values)))goto done;
    }
    if(repeated>=4)goto done;
    if(!lock_diagnostic_file){
        copy(lock_diagnostic_path,lock_capture_path,lock_capture_path_length);
        copy(lock_diagnostic_path+lock_capture_path_length,"\\lifecycle.log",15);
        lock_diagnostic_file=CreateFileA(lock_diagnostic_path,0x40000000,1,0,1,0x80,0);
        if(lock_diagnostic_file==(HANDLE)-1){lock_diagnostic_file=0;lock_diagnostic_count=128;goto done;}
    }
    char line[256];u32 length=0;const char* at=reason;
    while(*at && length<40)line[length++]=*at++;
    line[length++]=' ';
    for(u32 i=0;i<19;++i){length+=failure_hex(line+length,values[i]);line[length++]=' ';}
    line[length++]='\r';line[length++]='\n';
    if(!write_all(lock_diagnostic_file,line,length))lock_diagnostic_count=128;
    else {lock_diagnostic_seen[lock_diagnostic_count].reason=reason;copy(lock_diagnostic_seen[lock_diagnostic_count].values,values,sizeof(lock_diagnostic_seen[lock_diagnostic_count].values));++lock_diagnostic_count;}
 done:SetLastError(saved);__sync_lock_release(&lock_diagnostic_busy);
}
static void lock_diagnostic(const char* reason,void* object,u32 kind,u32 argument,u32 flags,i32 result,u32 owner,const u32* d){
    u32 values[19]={(u32)object,GetCurrentThreadId(),kind,argument,flags,(u32)result,owner,0};
    if(d){values[7]=d[0];values[8]=d[1];values[9]=d[3];values[10]=d[2];values[11]=d[4];values[12]=d[9];
        values[13]=d[19];values[14]=d[21];values[15]=d[22];values[16]=d[23];values[17]=d[24];values[18]=d[26];}
    lock_diagnostic_values(reason,values);
}
