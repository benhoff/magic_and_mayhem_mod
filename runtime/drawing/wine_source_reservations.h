/* Observed-path policy, not physical backing ownership. All table operations
 * hold journal_lock. Never retain a mapped CPU pointer after unmap. */
static HANDLE wine_reserve_file;
static u32 wine_reserve_enabled,wine_reserve_records,wine_reserve_acquired,wine_reserve_released;
static struct WineReadReservation {
 void* resource;void* texture;
 u32 submission,api,texture_id,generation,sub,epoch,payload,command,worker,state;
} wine_reservations[2048];
static struct {void* texture;void* resource;} wine_reserve_retiring[2048];
static struct WineReadReservation* wine_reserve_find(u32 submission){
 for(u32 i=0;i<2048;++i)if(submission&&wine_reservations[i].submission==submission)return wine_reservations+i;
 return 0;
}
static void wine_reserve_emit(const struct WineReadReservation* s,u32 phase,u32 reason,u32 kind){
 u32 r[32]={0};r[0]=++wine_reserve_records;r[1]=phase;r[2]=GetCurrentThreadId();r[3]=GetTickCount();
 if(s){r[4]=s->submission;r[5]=s->api;r[6]=s->texture_id;r[7]=s->generation;r[8]=wine_resource_id(s->resource,1);r[9]=s->sub;r[10]=s->epoch;r[11]=s->payload;r[12]=s->command;r[13]=s->worker;r[14]=s->state;}
 r[15]=reason;r[16]=kind;r[17]=wine_reserve_acquired;r[18]=wine_reserve_released;
 if(wine_reserve_records>131072||!write(wine_reserve_file,r,128))ExitProcess(99);
}
static void wine_reserve_refuse(const struct WineReadReservation* s,u32 reason,u32 kind){wine_reserve_emit(s,8,reason,kind);ExitProcess(99);}
/* Called before any of the four observed mutation entry points forwards. */
static void wine_reserve_write(void* resource,u32 kind){
 if(!wine_reserve_enabled)return;
 wine_phys_writer(resource,kind);
 for(u32 i=0;i<2048;++i)if(wine_reservations[i].submission&&wine_reservations[i].resource==resource)wine_reserve_refuse(wine_reservations+i,1,kind);
}
static void wine_reserve_begin(const u32* sample,void* texture,void* resource){
 if(!wine_reserve_enabled)return;EnterCriticalSection(journal_lock);
 struct WineReadReservation s={0};s.resource=resource;s.texture=texture;s.submission=sample[2];s.api=sample[3];s.texture_id=sample[6];s.generation=sample[7];s.sub=sample[8];
 for(u32 i=0;i<2048;++i)if(wine_reserve_retiring[i].texture&&(!wine_reserve_retiring[i].resource||wine_reserve_retiring[i].resource==resource||wine_reserve_retiring[i].texture==texture))wine_reserve_refuse(&s,12,0);
 u32 n=wine_source_version_index(resource);s.epoch=wine_source_versions[n].epoch;s.state=1;
 if(!s.submission||!s.texture_id||!s.generation||wine_lifetime_find(texture,1)!=s.generation||wine_source_versions[n].writers)wine_reserve_refuse(&s,2,0);
 if(wine_reserve_find(s.submission))wine_reserve_refuse(&s,3,0);
 u32 free=2048;for(u32 i=0;i<2048;++i)if(!wine_reservations[i].submission){free=i;break;}
 if(free==2048)wine_reserve_refuse(&s,4,0);
 wine_reservations[free]=s;++wine_reserve_acquired;wine_reserve_emit(&s,1,0,0);LeaveCriticalSection(journal_lock);
}
static void wine_reserve_step(u32 submission,u32 phase,u32 value){
 if(!wine_reserve_enabled)return;struct WineReadReservation* s=wine_reserve_find(submission);
 if(!s)wine_reserve_refuse(0,5,phase);
 if(s->state!=phase-1||!value||wine_lifetime_find(s->texture,1)!=s->generation||wine_source_versions[wine_source_version_index(s->resource)].epoch!=s->epoch)wine_reserve_refuse(s,6,phase);
 if(phase==2)s->payload=value;
 else if(phase==3)s->command=value;
 else if(phase==4)s->worker=value;
 else if(phase==5){if(value!=2)wine_reserve_refuse(s,7,value);}
 else if(phase==6){if(value!=s->worker)wine_reserve_refuse(s,6,phase);++wine_reserve_released;}
 else wine_reserve_refuse(s,6,phase);
 s->state=phase;wine_reserve_emit(s,phase,0,0);if(phase==6){wine_phys_release(submission);memset(s,0,sizeof(*s));}
}
/* Public getter is called outside journal_lock; preserve the wrapper's FP/error
 * around this additional diagnostic call. Any decref is refused conservatively,
 * since its original result cannot safely determine retirement in advance. */
static u32 wine_reserve_retire(void* texture){
 if(!wine_reserve_enabled)return 0;
 // Publish an unresolved retirement before the extra getter, then retain its
 // resolved resource until the ORIGINAL decref and generation journal return.
 // Acquisitions refuse unresolved/aliasing retirements; no original body runs
 // under journal_lock. A null early-install getter stays a wildcard lease.
 EnterCriticalSection(journal_lock);u32 slot=2048;
 for(u32 i=0;i<2048;++i){
  if(wine_reservations[i].submission&&wine_reservations[i].texture==texture)wine_reserve_refuse(wine_reservations+i,9,0);
  if(slot==2048&&!wine_reserve_retiring[i].texture)slot=i;
 }
 if(!texture||slot==2048)wine_reserve_refuse(0,4,5);
 wine_reserve_retiring[slot].texture=texture;LeaveCriticalSection(journal_lock);
 void* resource=wine_resource_original[0]?((void*(__attribute__((cdecl))*)(void*))wine_resource_original[0])(texture):0;
 EnterCriticalSection(journal_lock);
 for(u32 i=0;i<2048;++i)if(wine_reservations[i].submission&&(!resource||wine_reservations[i].texture==texture||wine_reservations[i].resource==resource))wine_reserve_refuse(wine_reservations+i,9,0);
 wine_reserve_retiring[slot].resource=resource;LeaveCriticalSection(journal_lock);return slot+1;
}
static void wine_reserve_retired(u32 token){
 if(!token)return;EnterCriticalSection(journal_lock);
 if(token>2048||!wine_reserve_retiring[token-1].texture)wine_reserve_refuse(0,6,5);
 memset(wine_reserve_retiring+token-1,0,sizeof(wine_reserve_retiring[0]));LeaveCriticalSection(journal_lock);
}
static void wine_reserve_unsupported(u32 status){
 if(!wine_reserve_enabled||!status)return;EnterCriticalSection(journal_lock);wine_reserve_refuse(0,10,status);
}
static int wine_reserve_open(u32 prerequisites){
 char flag[2];u32 n=GetEnvironmentVariableA("MNM_DRAW_WINE_SOURCE_RESERVATIONS",flag,2);if(!n)return 1;
 if(n!=1||flag[0]!='1'||!prerequisites)return 0;
 char name[600];path(name,"wine-source-reservations.bin");wine_reserve_file=CreateFileA(name,0x40000000,3,0,1,0x80,0);if(wine_reserve_file==(HANDLE)-1)return 0;
 u32 h[16]={0};copy(h,"MNMSRG01",8);h[2]=1;h[3]=128;h[4]=0x40209ca7;h[5]=131072;h[6]=2048;h[7]=4;
 if(!write(wine_reserve_file,h,64))return 0;wine_reserve_enabled=1;return 1;
}
static void wine_reserve_close(u32 reason){
 if(!wine_reserve_enabled)return;
 for(u32 i=0;i<2048;++i)if(wine_reservations[i].submission)wine_reserve_refuse(wine_reservations+i,11,0);
 for(u32 i=0;i<2048;++i)if(wine_reserve_retiring[i].texture)wine_reserve_refuse(0,11,5);
 wine_reserve_emit(0,7,reason,0);if(!CloseHandle(wine_reserve_file))ExitProcess(99);wine_reserve_file=0;wine_reserve_enabled=0;
}
