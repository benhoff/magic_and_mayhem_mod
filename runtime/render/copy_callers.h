#include "../../protocols/include/mnm/native_copy_callers_v2.h"
#include "../../protocols/include/mnm/copy_caller_handoff_v1.h"
#include "copy_caller_slots.h"
static struct MnmCopyCallersV2* game_copy_callers;
typedef u32(WIN* CopyCallerSceneQuery)(u32,u32,u32*,u32);
static CopyCallerSceneQuery game_copy_caller_scene;
struct CopyCallerContext {const struct CopyCallerLayout* layout;u32 base,slot,source,target,source_wrapper,mode,length,marker;};
static u8 game_copy_caller_code[sizeof(copy_caller_layouts)/sizeof(copy_caller_layouts[0])];
static void game_copy_callers_init(void){
 if(!game_copy_routes_v2)return;
 u32 error=GetLastError();char value[2];
 if(GetEnvironmentVariableA("MNM_NATIVE_COPY_CALLERS",value,sizeof(value))!=1 || value[0]!='1'){SetLastError(error);return;}
 char path[544];static const char suffix[]="\\copy-callers-v2.bin";
 if(lock_capture_path_length>sizeof(path)-sizeof(suffix)){SetLastError(error);return;}
 copy(path,lock_capture_path,lock_capture_path_length);copy(path+lock_capture_path_length,suffix,sizeof(suffix));
 HANDLE file=CreateFileA(path,0xc0000000,1,0,1,0x80,0);
 if(file!=(HANDLE)-1){HANDLE mapping=CreateFileMappingA(file,0,4,0,MNM_COPY_CALLERS_V2_BYTES,0);CloseHandle(file);
  if(mapping){game_copy_callers=MapViewOfFile(mapping,2,0,0,MNM_COPY_CALLERS_V2_BYTES);CloseHandle(mapping);}}
 if(game_copy_callers){zero(game_copy_caller_code,sizeof(game_copy_caller_code));u32* h=game_copy_callers->header;copy(h,"MNMCCL02",8);h[2]=2;h[3]=128;h[4]=80;h[5]=MNM_COPY_CALLERS_CAPACITY;
  h[17]=game_copy_routes_v2->header[15];h[18]=game_copy_routes_v2->header[16];}
 SetLastError(error);
}
static int game_copy_caller_raw(u32 address,void* output,u32 bytes){
 if(!readable((const void*)address,bytes))return 0;copy(output,(const void*)address,bytes);return 1;
}
static int game_copy_caller_snapshot(struct CopyCallerContext* c,u32 slot,u32* response){
 if(!game_copy_caller_scene){void* module=GetModuleHandleA("MnmScene.dll");if(module)game_copy_caller_scene=(CopyCallerSceneQuery)GetProcAddress(module,"SceneCopyCallerV1");}
 if(!game_copy_caller_scene || !game_copy_caller_scene(c->base+c->layout->entry,slot,response,32) ||
    !mnm_copy_caller_handoff_valid(response,c->base+c->layout->entry,c->layout->length,c->layout->pop) ||
    !same((u8*)response+64,c->layout->prefix,c->layout->length))return 0;
 if(response[9] && (response[13]!=slot+4+c->layout->pop || response[14]!=GetCurrentThreadId()))return 0;
 c->length=response[4];return 1;
}
static int game_copy_caller_read(void* context,u32 address,void* output,u32 bytes){
 struct CopyCallerContext* c=context;
 if(!game_copy_caller_raw(address,output,bytes))return 0;
 if(!c->layout)return 1;
 if(address==c->base+c->layout->entry && bytes==c->layout->end-c->layout->entry && *(u8*)output==0xe9){
  u32 response[32];if(game_copy_caller_snapshot(c,0,response))copy(output,(u8*)response+64,c->layout->length);
 }else if(address==c->slot && bytes==8){
  u32 response[32];
  if(game_copy_caller_snapshot(c,address,response) && response[9]){
   u32 source=0,target=0;
   if(((u32*)output)[0]!=response[7] || ((u32*)output)[1]!=response[11] ||
      response[11]>0xfffffff7u || response[12]>0xfffffff7u ||
      !game_copy_caller_raw(response[11]+8,&target,4) || !game_copy_caller_raw(response[12]+8,&source,4) ||
      target!=c->target || source!=c->source)return 0;
   ((u32*)output)[0]=response[10];((u32*)output)[1]=response[11];
   c->mode=2;c->source_wrapper=response[12];c->marker=response[7];
  }
 }
 return 1;
}
static void game_copy_callers_prepare(struct GameBlit* p,u32 slot,u32 fast){
 if(!game_copy_callers)return;
 u32 error=GetLastError();
 struct CopyCallerContext context={0};context.base=game_copy_callers->header[17];context.source=(u32)p->source;context.target=(u32)p->target;
 for(unsigned i=0;i<sizeof(copy_caller_sites)/sizeof(copy_caller_sites[0]);++i)
  if(p->caller-context.base==copy_caller_sites[i].api){context.layout=copy_caller_layouts+copy_caller_sites[i].layout;
   if(slot<=0xffffffffu-context.layout->distance)context.slot=slot+context.layout->distance;break;}
 copy_caller_resolve(game_copy_callers->header[17],game_copy_callers->header[18],p->caller,slot,fast,(u32)p->target,
  game_copy_caller_read,&context,game_copy_caller_code,p->callers);
 p->callers[16]=context.source_wrapper;p->callers[17]=context.mode?context.mode:p->callers[9]==1?1u:0u;
 p->callers[18]=context.length;p->callers[19]=context.marker;
 SetLastError(error);
}
static void game_copy_callers_finish(struct GameBlit* p,u32 ordinal){
 if(!game_copy_callers)return;
 u32* row=p->callers;row[0]=ordinal;row[10]=p->routes[15];row[11]=p->routes[11]?(p->routes[12]&1u)!=0:(p->routes[12]&0x8000u)!=0;
 row[12]=(p->routes[10]&4u)!=0;row[13]=p->routes[7];row[14]=p->routes[6];
 mnm_copy_callers_v2_record(game_copy_callers,row);
}
static void game_copy_callers_close(void){
 if(!game_copy_callers)return;
 u32 error=GetLastError();
 u32* h=game_copy_callers->header;h[19]=game_copy_routes_v2->header[6];
 if(h[19]!=h[6] || game_copy_routes_v2->header[7] || game_copy_routes_v2->header[9])h[9]|=2;
 h[16]=1;UnmapViewOfFile(game_copy_callers);game_copy_callers=0;SetLastError(error);
}
