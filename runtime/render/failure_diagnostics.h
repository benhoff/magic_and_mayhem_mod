/* Separate bounded failure evidence: unaffected by the successful-event cap. */
static char failure_path[512];
static HANDLE failure_file;
static volatile i32 failure_busy;
static u32 failure_count;
struct FailureSeen {const char* phase;u32 values[5];};
static struct FailureSeen failure_seen[64];
static void init_failure_diagnostics(void){
    u32 length=GetEnvironmentVariableA("MNM_RENDER_FAILURE_LOG",failure_path,sizeof(failure_path));
    if(!length || length>=sizeof(failure_path))failure_path[0]=0;
}
static u32 failure_hex(char* out,u32 value){
    const char* digits="0123456789abcdef";
    for(u32 i=0;i<8;++i)out[i]=digits[(value>>(28-i*4))&15];return 8;
}
static void render_failure(const char* phase,i32 result,void* object,u32 kind,u32 flags){
    if(result>=0 || !failure_path[0] || !__sync_bool_compare_and_swap(&failure_busy,0,1))return;
    u32 saved=GetLastError();
    if(failure_count>=64)goto done;
    u32 values[5]={(u32)result,GetCurrentThreadId(),(u32)object,kind,flags};
    u32 phase_count=0;
    for(u32 i=0;i<failure_count;++i){
        if(failure_seen[i].phase!=phase)continue;
        ++phase_count;
        if(same(failure_seen[i].values,values,sizeof(values)))goto done;
    }
    /* Reserve capacity for later application errors even if readback keeps failing. */
    if(phase_count>=8)goto done;
    const char* name=phase;
    if(!failure_file){
        failure_file=CreateFileA(failure_path,0x40000000,1,0,1,0x80,0);
        if(failure_file==(HANDLE)-1){failure_file=0;failure_count=64;goto done;}
    }
    char line[160];u32 length=0;
    while(*phase && length<80)line[length++]=*phase++;
    line[length++]=' ';
    for(u32 i=0;i<5;++i){length+=failure_hex(line+length,values[i]);line[length++]=' ';}
    line[length++]='\r';line[length++]='\n';
    if(!write_all(failure_file,line,length))failure_count=64;
    else {failure_seen[failure_count].phase=name;copy(failure_seen[failure_count].values,values,sizeof(values));++failure_count;}
    if(failure_count>=64){CloseHandle(failure_file);failure_file=0;}
 done:SetLastError(saved);__sync_lock_release(&failure_busy);
}
