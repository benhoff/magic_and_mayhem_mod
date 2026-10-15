/* Diagnostic read-onlypages; no originalbyte mutation/unobtrusiveclaim. */
static void* source_lease_handler;
#include "source_write_trace.h"
static i32 WIN source_lease_fault(struct BatchExceptionPointers* pointers){
 struct BatchExceptionRecord* e=pointers?pointers->record:0;
 if(!e||e->code!=0xc0000005u||e->count<2||e->information[0]!=1||!source_lease_active||e->information[1]<source_lease_base||e->information[1]-source_lease_base>=source_lease_extent)return 0;
 source_write_trace_fault(e);
 char name[260];path(name,"source-lease-fault-",source_lease_id,".bin");u32 r[12]={0};copy(r,"MNMSLFT1",8);r[2]=source_lease_id;r[3]=source_lease_canvas;r[4]=e->address;r[5]=e->information[1];r[6]=GetCurrentThreadId();r[7]=source_lease_owner;r[8]=source_lease_base;r[9]=source_lease_extent;r[10]=source_lease_old;
 HANDLE file=CreateFileA(name,0x40000000,0,0,1,0x80,0);if(file!=(HANDLE)-1){write(file,r,48);CloseHandle(file);}ExitProcess(99);return 0;
}
u32 WIN SceneDrawingSourceLeaseV1(void* com,u32 lease,u32* metadata,u32 size){
 u32 error=GetLastError();if(size!=64||!bypass_writable((u32)metadata,64)||!producer_trace_mode){SetLastError(error);return 1;}PRODUCER_TRACE_SCOPE;
 if(lease){
  if(!source_lease_active||lease!=source_lease_id||GetCurrentThreadId()!=source_lease_owner){SetLastError(error);return 2;}
  struct Surface* s=0;for(u32 i=0;i<MAX_SURFACES;++i)if(surfaces[i].id==source_lease_canvas&&surfaces[i].object){s=surfaces+i;break;}
  if(!s||s->pixels!=source_lease_pixels||s->object!=source_lease_object||s->width!=800||s->height!=600||s->stride!=800||!readable(s->object+4,8)||((u32)com!=get((void*)(s->object+4))&&(u32)com!=get((void*)(s->object+8)))){SetLastError(error);return 3;}
  for(u32 at=source_lease_base;at<source_lease_base+source_lease_extent;){u32 m[7];if(VirtualQuery((void*)at,m,28)!=28||m[4]!=0x1000||m[5]!=2||m[0]+m[3]<=at){SetLastError(error);return 4;}at=m[0]+m[3];}
  if(source_write_trace_enabled)source_write_trace_phase=3;
  u32 old;if(!VirtualProtect((void*)source_lease_base,source_lease_extent,source_lease_old,&old)||old!=2){SetLastError(error);return 4;}source_lease_active=0;source_write_trace_marker(3);SceneDrawingSourcePointV1(com,metadata,64);SetLastError(error);return 0;
 }
 char flag[2];if(GetEnvironmentVariableA("MNM_DRAW_SOURCE_LEASE",flag,2)!=1||flag[0]!='1'||source_lease_active){SetLastError(error);return 2;}
 if(!SceneDrawingSourcePointV1(com,metadata,64)||metadata[2]!=19||metadata[5]||metadata[13]){SetLastError(error);return 3;}
 struct Surface* s=0;for(u32 i=0;i<MAX_SURFACES;++i)if(surfaces[i].id==metadata[6]&&surfaces[i].object){s=surfaces+i;break;}
 if(!s||s->width!=800||s->height!=600||s->stride!=800||!s->pixels||s->pixels>~0u-960000-4095){SetLastError(error);return 3;}
 if(metadata[12]&&source_pending_check(s,1)){SetLastError(error);return 3;}
 u32 base=s->pixels&~4095u,bytes=((s->pixels+960000+4095u)&~4095u)-base,m[7],end[7];
 if(VirtualQuery((void*)base,m,28)!=28||VirtualQuery((void*)(base+bytes-1),end,28)!=28||m[1]!=end[1]||m[4]!=0x1000||m[5]!=4||base<m[0]){SetLastError(error);return 4;}
 for(u32 at=base;at<base+bytes;){u32 region[7];if(VirtualQuery((void*)at,region,28)!=28||region[1]!=m[1]||region[4]!=0x1000||region[5]!=4||region[0]+region[3]<=at){SetLastError(error);return 4;}at=region[0]+region[3];}
 if(!source_lease_handler){source_lease_handler=AddVectoredExceptionHandler(1,(void*)&source_lease_fault);if(!source_lease_handler){SetLastError(error);return 4;}}
 source_lease_base=base;source_lease_extent=bytes;source_lease_old=4;source_lease_owner=GetCurrentThreadId();source_lease_canvas=s->id;source_lease_pixels=s->pixels;source_lease_object=s->object;source_lease_id=++source_lease_next;
 if(!source_write_trace_prepare()){SetLastError(error);return 4;}
 source_lease_active=1;u32 old;if(!VirtualProtect((void*)base,bytes,2,&old)||old!=4){source_lease_active=0;SetLastError(error);return 4;}
 source_lease_old=old;source_write_trace_marker(2);SetLastError(error);return source_lease_id|0x80000000u;
}
