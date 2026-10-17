#include "../../protocols/include/mnm/native_gpu_copy_census_v1.h"
static struct MnmGpuCopyCensus* game_gpu_census;
/* One bounded metadata mapping; no pixel access, logging worker dependency or
 * per-copy file I/O. Failure loses observation, never changes API admission. */
static void game_gpu_census_init(void){
 if(!game_resident_world || !lock_capture_path_length)return;
 u32 error=GetLastError();char enabled[2];
 if(!GetEnvironmentVariableA("MNM_NATIVE_COPY_DIAGNOSTICS",enabled,sizeof(enabled))){SetLastError(error);return;}
 char path[544];copy(path,lock_capture_path,lock_capture_path_length);
 copy(path+lock_capture_path_length,"\\gpu-copy-census-v1.bin",24);
 HANDLE f=CreateFileA(path,0xc0000000,1,0,1,0x80,0);
 if(f!=(HANDLE)-1){
  HANDLE m=CreateFileMappingA(f,0,4,0,MNM_GPU_COPY_CENSUS_BYTES,0);CloseHandle(f);
  if(m){game_gpu_census=MapViewOfFile(m,2,0,0,MNM_GPU_COPY_CENSUS_BYTES);CloseHandle(m);}
 }
 if(game_gpu_census){
  u32* h=game_gpu_census->header;copy(h,"MNMGCN01",8);h[2]=1;h[3]=128;h[4]=128;h[5]=MNM_GPU_COPY_CENSUS_CAPACITY;
  h[18]=(u32)GetModuleHandleA(0);
  if(readable((void*)h[18],64)){
   u32 pe=*(u32*)(h[18]+0x3c);
   if(pe<=4096 && readable((void*)(h[18]+pe),84) && *(u32*)(h[18]+pe)==0x4550)h[19]=*(u32*)(h[18]+pe+80);
  }
 }
 SetLastError(error);
}
static void game_gpu_census_close(void){
 if(game_gpu_census){game_gpu_census->header[20]=1;UnmapViewOfFile(game_gpu_census);game_gpu_census=0;}
}
static void game_gpu_census_record(const struct GameBlit* p,const struct GameSurface* src,const struct GameSurface* dst,
 const struct MnmGpuCopyPlan* plan,i32 admission,i32 result,u32 source,u32 target,u32 scratch,u32 first,u32 last){
 if(!game_gpu_census)return;
 u32 base=game_gpu_census->header[18],extent=game_gpu_census->header[19];
 u32 row[32]={0,p->caller,base,p->caller>=base && p->caller-base<extent?p->caller-base:0xffffffffu,
  source,target,scratch,first,last,session_epoch,(u32)p->source,(u32)p->target,p->source_generation,p->target_generation,dst->generation,
  (src==dst?1u:0u)|(dst->clip?2u:0u)|(p->fields[8]?4u:0u)|(p->bootstrap?8u:0u),
  admission>=0?plan->count:0,(u32)result,(u32)admission,src->pixels.width,src->pixels.height,dst->pixels.width,dst->pixels.height,
  p->fields[2],p->fields[3],p->fields[4],p->fields[5],p->fields[6],p->fields[7],
  p->fields[6]+p->fields[4]-p->fields[2],p->fields[7]+p->fields[5]-p->fields[3],dst->gpu_history};
 mnm_gpu_copy_census_record(game_gpu_census,row);
}
