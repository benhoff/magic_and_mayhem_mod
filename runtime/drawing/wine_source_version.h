/* Observed resource write epochs only. No mapped pointer survives unmap. */
static u32 wine_source_version_enabled,wine_source_version_records,wine_source_version_samples;
static HANDLE wine_source_version_file;
static void* wine_source_version_rebind_original;
static void* wine_resource_original[3];
static u32 wine_lifetime_records;
static u32 wine_lifetime_find(void*,u32);
static u32 wine_resource_id(void*,u32);
static struct {void* resource;u32 epoch,writers;} wine_source_versions[2048];
static struct WineSourceVersion {
 void* resource;void* texture;void* destination;
 u32 submission,api,texture_id,generation,sub,epoch,payload,command,worker,alias;
} wine_source_version_pending[2048];
static u32 wine_source_version_index(void* resource){
 if(!resource)ExitProcess(99);
 for(u32 i=0;i<2048;++i)if(wine_source_versions[i].resource==resource)return i;
 for(u32 i=0;i<2048;++i)if(!wine_source_versions[i].resource){wine_source_versions[i].resource=resource;wine_source_versions[i].epoch=1;return i;}
 ExitProcess(99);return 0;
}
static void wine_source_version_emit(u32* r){
 r[0]=++wine_source_version_records;r[2]=GetCurrentThreadId();r[3]=GetTickCount();
 if(wine_source_version_records>131072||!write(wine_source_version_file,r,128))ExitProcess(99);
}
static void wine_reserve_write(void*,u32);
static void wine_reserve_step(u32,u32,u32);
/* Caller holds journal_lock. Entry and exit both invalidate an older read. */
static void wine_source_version_write(void* resource,u32 sub,u32 kind,u32 phase){
 if(!wine_source_version_enabled)return;u32 i=wine_source_version_index(resource),r[32]={0};
 if(phase!=1&&phase!=2)ExitProcess(99);if(phase==1)wine_reserve_write(resource,kind);if(phase==2&&!wine_source_versions[i].writers)ExitProcess(99);
 if(wine_source_versions[i].epoch==0xffffffffu)ExitProcess(99);
 ++wine_source_versions[i].epoch;
 if(phase==1)++wine_source_versions[i].writers;else --wine_source_versions[i].writers;
 r[1]=phase;r[8]=wine_resource_id(resource,1);r[9]=sub;r[11]=wine_source_versions[i].epoch;r[12]=wine_source_versions[i].writers;r[18]=kind;wine_source_version_emit(r);
}
static void wine_source_version_row(u32* r,const struct WineSourceVersion* s,u32 phase){
 u32 i=wine_source_version_index(s->resource);r[1]=phase;r[4]=s->submission;r[5]=s->api;r[6]=s->texture_id;r[7]=s->generation;r[8]=wine_resource_id(s->resource,1);r[9]=s->sub;
 r[10]=s->epoch;r[11]=wine_source_versions[i].epoch;r[12]=wine_source_versions[i].writers;r[13]=s->payload;r[14]=s->command;r[15]=s->worker;r[17]=wine_lifetime_records;r[22]=wine_lifetime_find(s->texture,1);r[23]=s->alias;
 if(s->destination){u32 d=wine_source_version_index(s->destination);r[19]=wine_resource_id(s->destination,1);r[20]=wine_source_versions[d].epoch;r[21]=wine_source_versions[d].writers;}
}
static void wine_source_version_refuse(struct WineSourceVersion* s,u32 reason){
 u32 r[32]={0};wine_source_version_row(r,s,8);r[16]=reason;wine_source_version_emit(r);ExitProcess(99);
}
/* Called after the synchronous READ map has returned, before copying rows. */
static u32 wine_source_version_read(void* resource){
 if(!wine_source_version_enabled)return 0;EnterCriticalSection(journal_lock);
 u32 i=wine_source_version_index(resource),epoch=wine_source_versions[i].epoch;
 if(wine_source_versions[i].writers){struct WineSourceVersion s={0};s.resource=resource;wine_source_version_refuse(&s,1);}
 LeaveCriticalSection(journal_lock);return epoch;
}
static void wine_source_version_snapshot(const u32* sample,void* texture,void* resource,u32 epoch){
 if(!wine_source_version_enabled)return;EnterCriticalSection(journal_lock);
 struct WineSourceVersion value={0};value.resource=resource;value.texture=texture;value.submission=sample[2];value.api=sample[3];value.texture_id=sample[6];value.generation=sample[7];value.sub=sample[8];value.epoch=epoch;value.payload=sample[33];
 u32 i=wine_source_version_index(resource);
 if(!epoch||epoch!=wine_source_versions[i].epoch||wine_source_versions[i].writers)wine_source_version_refuse(&value,2);
 if(!value.generation||wine_lifetime_find(texture,1)!=value.generation)wine_source_version_refuse(&value,3);
 u32 free=2048;
 for(u32 n=0;n<2048;++n){if(wine_source_version_pending[n].submission==value.submission)wine_source_version_refuse(&value,4);if(!wine_source_version_pending[n].submission&&free==2048)free=n;}
 if(free==2048)wine_source_version_refuse(&value,5);wine_source_version_pending[free]=value;++wine_source_version_samples;wine_reserve_step(value.submission,2,value.payload);
 u32 r[32]={0};wine_source_version_row(r,&value,3);wine_source_version_emit(r);LeaveCriticalSection(journal_lock);
}
static struct WineSourceVersion* wine_source_version_find(u32 submission){
 for(u32 i=0;i<2048;++i)if(submission&&wine_source_version_pending[i].submission==submission)return wine_source_version_pending+i;
 return 0;
}
/* Exact packet binding supplies the submission; never pair by order or count. */
static void wine_source_version_packet(u32 submission,u32 command){
 if(!wine_source_version_enabled)return;struct WineSourceVersion* s=wine_source_version_find(submission);
 if(!s){struct WineSourceVersion missing={0};missing.resource=(void*)1;missing.submission=submission;missing.command=command;wine_source_version_refuse(&missing,6);}
 if(!command||s->command)wine_source_version_refuse(s,7);s->command=command;wine_reserve_step(submission,3,command);u32 r[32]={0};wine_source_version_row(r,s,4);wine_source_version_emit(r);
}
/* Called under journal_lock before the original selected worker and on return. */
static void wine_source_version_worker(const u32* packet,u32 submission,u32 command,u32 worker,u32 phase){
 if(!wine_source_version_enabled)return;struct WineSourceVersion* s=wine_source_version_find(submission);
 if(!s){struct WineSourceVersion missing={0};missing.resource=(void*)packet[9];missing.submission=submission;missing.command=command;missing.worker=worker;wine_source_version_refuse(&missing,6);}
 if(s->command!=command||!worker||s->resource!=(void*)packet[9]||s->sub!=packet[10])wine_source_version_refuse(s,7);
 u32 i=wine_source_version_index(s->resource);
 if(wine_lifetime_find(s->texture,1)!=s->generation)wine_source_version_refuse(s,3);
 if(phase==2){
  if(s->worker)wine_source_version_refuse(s,7);s->worker=worker;wine_reserve_step(submission,4,worker);
  if(s->epoch!=wine_source_versions[i].epoch||wine_source_versions[i].writers)wine_source_version_refuse(s,8);
  s->destination=(void*)packet[1];s->alias=s->destination==s->resource;
  u32 r[32]={0};wine_source_version_row(r,s,5);wine_source_version_emit(r);
  wine_source_version_write(s->destination,packet[2],3,1);
 }else if(phase==3){
  if(s->worker!=worker)wine_source_version_refuse(s,7);
  u32 okay=wine_source_versions[i].epoch==s->epoch+s->alias&&wine_source_versions[i].writers==s->alias;
  wine_source_version_write(s->destination,packet[2],3,2);
  if(!okay)wine_source_version_refuse(s,9);
  u32 r[32]={0};wine_source_version_row(r,s,6);wine_source_version_emit(r);wine_reserve_step(submission,6,worker);memset(s,0,sizeof(*s));
 }else wine_source_version_refuse(s,7);
}
static u32 __attribute__((cdecl)) wine_source_version_rebind(void* texture,u32 sub,void* memory,u32 pitch){
 if(!wine_source_version_enabled)return ((u32(__attribute__((cdecl))*)(void*,u32,void*,u32))wine_source_version_rebind_original)(texture,sub,memory,pitch);
 u32 error=GetLastError();u8 fp[512] __attribute__((aligned(16)));snapshot_fp(fp);
 void* resource=((void*(__attribute__((cdecl))*)(void*))wine_resource_original[0])(texture);
 EnterCriticalSection(journal_lock);wine_source_version_write(resource,sub,4,1);LeaveCriticalSection(journal_lock);restore_fp(fp);SetLastError(error);
 u32 result=((u32(__attribute__((cdecl))*)(void*,u32,void*,u32))wine_source_version_rebind_original)(texture,sub,memory,pitch);
 error=GetLastError();snapshot_fp(fp);EnterCriticalSection(journal_lock);wine_source_version_write(resource,sub,4,2);LeaveCriticalSection(journal_lock);restore_fp(fp);SetLastError(error);return result;
}
static int wine_source_version_open(void);
static int wine_source_version_install(void* ddraw,void* module){
 if(!wine_source_version_enabled)return 1;void** slot=(void**)((u32)ddraw+0x6d700);void* expected=GetProcAddress(module,"wined3d_texture_update_desc");
 const u8 prefix[16]={0x8d,0x4c,0x24,0x04,0x83,0xe4,0xf8,0xff,0x71,0xfc,0x55,0x89,0xe5,0x57,0x56,0x53};
 if(expected!=(void*)((u32)module+0x221460)||!readable(slot,4)||*slot!=expected||!readable(expected,16)||!equal(expected,prefix,16))return 0;
 u32 old,unused;if(!VirtualProtect(slot,4,4,&old))return 0;wine_source_version_rebind_original=*slot;*slot=(void*)&wine_source_version_rebind;return VirtualProtect(slot,4,old,&unused)!=0;
}
static void wine_source_version_close(u32 reason){
 if(!wine_source_version_file)return;
 for(u32 i=0;i<2048;++i)if(wine_source_version_pending[i].submission)wine_source_version_refuse(wine_source_version_pending+i,10);
 u32 leases=0,resources=0;
 for(u32 i=0;i<2048;++i)if(wine_source_versions[i].writers){u32 boundary[32]={0};boundary[1]=9;boundary[8]=wine_resource_id(wine_source_versions[i].resource,1);boundary[11]=wine_source_versions[i].epoch;boundary[12]=wine_source_versions[i].writers;wine_source_version_emit(boundary);leases+=boundary[12];++resources;}
 u32 r[32]={0};r[1]=7;r[4]=reason;r[13]=wine_source_version_samples;r[14]=leases;r[15]=resources;wine_source_version_emit(r);
 if(!CloseHandle(wine_source_version_file))ExitProcess(99);wine_source_version_file=0;wine_source_version_enabled=0;
}

#include "wine_source_reservations.h"
