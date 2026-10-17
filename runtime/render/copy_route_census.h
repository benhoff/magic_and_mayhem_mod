#include "../../protocols/include/mnm/native_copy_routes_v1.h"
static struct MnmCopyRoutes* game_copy_routes;
static void game_copy_routes_init(void){
 if(!game_resident_world || !lock_capture_path_length)return;
 u32 error=GetLastError();char enabled[2];
 if(!GetEnvironmentVariableA("MNM_NATIVE_COPY_DIAGNOSTICS",enabled,sizeof(enabled))){SetLastError(error);return;}
 char path[544];copy(path,lock_capture_path,lock_capture_path_length);
 static const char suffix[]="\\copy-routes-v1.bin";
 copy(path+lock_capture_path_length,suffix,sizeof(suffix));
 HANDLE f=CreateFileA(path,0xc0000000,1,0,1,0x80,0);
 if(f!=(HANDLE)-1){HANDLE m=CreateFileMappingA(f,0,4,0,MNM_COPY_ROUTES_BYTES,0);CloseHandle(f);
  if(m){game_copy_routes=MapViewOfFile(m,2,0,0,MNM_COPY_ROUTES_BYTES);CloseHandle(m);}}
 if(game_copy_routes){u32* h=game_copy_routes->header;copy(h,"MNMCRT01",8);
  h[2]=1;h[3]=128;h[4]=160;h[5]=MNM_COPY_ROUTES_CAPACITY;h[15]=(u32)GetModuleHandleA(0);
  if(readable((void*)h[15],64)){u32 pe=*(u32*)(h[15]+0x3c);
   if(pe<=4096 && readable((void*)(h[15]+pe),84) && *(u32*)(h[15]+pe)==0x4550)h[16]=*(u32*)(h[15]+pe+80);}}
 SetLastError(error);
}
static void game_copy_routes_close(void){
 if(game_copy_routes){game_copy_routes->header[14]=1;UnmapViewOfFile(game_copy_routes);game_copy_routes=0;}
}
static void game_copy_routes_missed(u32 reason){
 if(game_copy_routes)mnm_copy_routes_skipped(game_copy_routes,reason,0);
}
static void game_copy_routes_prepare(struct GameBlit* p,const struct GameSurface* src,const struct GameSurface* dst,
 u32 fast,u32 flags,void* effects,void* destination,void* rectangle,const struct Rect* sr,const struct Rect* dr,const char* reason){
 if(!game_copy_routes)return;
 u32 error=GetLastError(),*r=p->routes;r[3]=GetCurrentThreadId();r[4]=(u32)p->target;r[5]=(u32)p->source;
 r[6]=dst?(u32)dst->object:0;r[7]=src?(u32)src->object:0;
 r[8]=dst?dst->generation:0;r[9]=src?src->generation:0;
 r[10]=(src?1u:0u)|(dst?2u:0u)|(src && src->gpu_history?4u:0u)|(dst && dst->gpu_history?8u:0u)|
  (dst && dst->clip_known?16u:0u)|(dst && dst->clip?32u:0u)|(src && src->key_known?64u:0u)|
  (src && src==dst?128u:0u)|(p->source && p->source==p->target?256u:0u)|
  (src && src->pixels.data?512u:0u)|(dst && dst->pixels.data?1024u:0u)|
  (rectangle?2048u:0u)|(destination?4096u:0u)|(effects?8192u:0u)|(p->source?16384u:0u)|32768u;
 r[11]=fast;r[12]=flags;r[14]=p->valid;
 r[15]=p->valid?(p->fill?3u:p->gpu?2u:1u):0;
 if(src){r[16]=src->pixels.width;r[17]=src->pixels.height;r[18]=src->pixels.bits;r[22]=src->key;r[34]=src->generation_origin;r[35]=src->generation_caller;}
 if(dst){r[19]=dst->pixels.width;r[20]=dst->pixels.height;r[21]=dst->pixels.bits;r[23]=dst->clip;r[36]=dst->generation_origin;r[37]=dst->generation_caller;}
 copy(r+24,sr,16);copy(r+28,dr,16);r[32]=2166136261u;
 for(const char* s=reason;*s;++s)r[32]=(r[32]^(u8)*s)*16777619u;
 r[38]=__atomic_load_n(&game_lock_epoch,__ATOMIC_RELAXED);
 SetLastError(error);
}
static void game_copy_routes_finish(struct GameBlit* p,i32 result,u32 gpu_row){
 if(!game_copy_routes)return;
 u32 error=GetLastError(),*r=p->routes,base=game_copy_routes->header[15],extent=game_copy_routes->header[16];
 r[1]=p->caller;r[2]=p->caller>=base && p->caller-base<extent?p->caller-base:0xffffffffu;
 r[13]=(u32)result;r[33]=p->gpu==2?gpu_row:0;
 if(!(r[10]&32768u))game_copy_routes_missed(4);
 mnm_copy_routes_record(game_copy_routes,r);SetLastError(error);
}
#define COPY_COMMAND_CALLBACK(fallback) \
 struct CommandLease command_lease __attribute__((cleanup(command_gate_leave)))=command_gate_enter(); \
 if(!command_lease.observe){if(game_copy_routes)mnm_copy_routes_skipped(game_copy_routes,1,command_lease.retired);return (fallback);}
