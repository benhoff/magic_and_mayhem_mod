/* Optional metadata only. Map at startup and flush/close at DLL retirement;
 * callbacks copy the already acquired immutable RGNDATA, with no extra GDI
 * query, pixel access, clip change or file IO. Disabled by default. */
#include "../../protocols/include/mnm/native_copy_clip_regions_v1.h"
API HANDLE WIN CreateFileMappingA(HANDLE,void*,u32,u32,u32,const char*);
API void* WIN MapViewOfFile(HANDLE,u32,u32,u32,u32);
API i32 WIN FlushViewOfFile(const void*,u32);
API i32 WIN UnmapViewOfFile(const void*);
static HANDLE nd_clip_file,nd_clip_mapping;
static struct MnmCopyClips* nd_clip_view;
static volatile i32 nd_clip_busy;
static void nd_clip_start(void){
 u32 error=GetLastError();char value[2];
 if(GetEnvironmentVariableA("MNM_NATIVE_COPY_CLIP_REGIONS",value,sizeof(value))!=1 || value[0]!='1')goto done;
 nd_clip_file=CreateFileA("native-copy-clip-regions-v1.bin",0xc0000000,1,0,1,0x80,0);
 if(nd_clip_file==(HANDLE)-1){nd_clip_file=0;goto done;}
 nd_clip_mapping=CreateFileMappingA(nd_clip_file,0,4,0,MNM_COPY_CLIPS_BYTES,0);
 if(!nd_clip_mapping)goto done;
 nd_clip_view=MapViewOfFile(nd_clip_mapping,2,0,0,MNM_COPY_CLIPS_BYTES);
 if(nd_clip_view)mnm_copy_clips_init(nd_clip_view);
done:SetLastError(error);
}
static void nd_clip_observe(u32 phase,const struct NsObject* dst,const struct NsObject* src,
 const i32* dr,const i32* sr,u32 flags,const u32* clips,u32 result,u32 pieces,u32 fill_value,u32 fill_observation){
 if(!nd_clip_view || !dst || !dst->state->clipper || !clips)return;
 if(!__sync_bool_compare_and_swap(&nd_clip_busy,0,1)){
  __atomic_add_fetch(&nd_clip_view->header[7],1,__ATOMIC_RELAXED);
  __atomic_fetch_or(&nd_clip_view->header[9],8,__ATOMIC_RELAXED);return;
 }
 const struct NsState *d=dst->state,*s=src?src->state:0;
 u32 fields[64];for(u32 i=0;i<64;++i)((volatile u32*)fields)[i]=0;
 fields[1]=phase;fields[2]=ns_lifetime_thread();fields[3]=(u32)dst;fields[4]=(u32)src;
 fields[5]=(u32)&d->aliases[0];fields[6]=s?(u32)&s->aliases[0]:0;
 fields[7]=d->lifetime_identity;fields[8]=s?s->lifetime_identity:0;
 fields[9]=(u32)d->clipper;fields[10]=(u32)d->clipper->window;fields[11]=d->clipper->refs;
 fields[12]=flags;fields[13]=result;fields[14]=clips[2];fields[15]=32+clips[2]*16;
 for(u32 i=0;i<4;++i){fields[16+i]=(u32)dr[i];fields[20+i]=(u32)sr[i];}
 fields[24]=d->width;fields[25]=d->height;fields[26]=d->bits;
 if(s){fields[27]=s->width;fields[28]=s->height;fields[29]=s->bits;}
 fields[30]=(flags&0x8000)!=0;fields[31]=fields[30]&&s?s->key_low:0;fields[32]=pieces;
 fields[33]=fill_value;fields[34]=fill_observation;
 mnm_copy_clips_record(nd_clip_view,fields,clips);
 __sync_lock_release(&nd_clip_busy);
}
static void nd_clip_finish(void){
 u32 error=GetLastError();
 if(nd_clip_view){
  mnm_copy_clips_close(nd_clip_view);
  if(!FlushViewOfFile(nd_clip_view,MNM_COPY_CLIPS_BYTES))nd_clip_view->header[9]|=16;
  UnmapViewOfFile(nd_clip_view);nd_clip_view=0;
 }
 if(nd_clip_mapping){CloseHandle(nd_clip_mapping);nd_clip_mapping=0;}
 if(nd_clip_file){CloseHandle(nd_clip_file);nd_clip_file=0;}
 SetLastError(error);
}
