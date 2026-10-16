/* Exercise the production facade, admission and FIFO GPU bridge together.
 * Supplied GPU images differ from poisoned CPU backings. Expected pixels come
 * from a separate current-CPU facade invocation, never enter command inputs. */
#define MNM_RENDER_SELFTEST 1
#include "../runtime/render/bridge.c"
API void* WIN LoadLibraryA(const char*);
static struct MnmGpuCopyPlan fixture_plan;
static i32 (WIN *fixture_real_plan)(void*,const i32*,void*,const i32*,u32,struct MnmGpuCopyPlan*,u32);
static u32 fixture_calls;
static i32 WIN fixture_record_plan(void* d,const i32* dr,void* s,const i32* sr,u32 f,struct MnmGpuCopyPlan* p,u32 n){
 i32 result=fixture_real_plan(d,dr,s,sr,f,p,n);++fixture_calls;if(result>=0)copy(&fixture_plan,p,sizeof(*p));return result;
}
struct FixtureInput {u32 id,width,height,self,clip,keyed,bootstrap,current_target; i32 sr[4],dr[4];};
static struct FixtureInput fixture_input;
static u16 fixture_source[512*9],fixture_target[512*9],fixture_expected[512*9];
static u32 fixture_region[8+4*256],fixture_checks;
static void fixture_require(int ok,u32 stage){++fixture_checks;if(!ok)ExitProcess(stage);}
static void** fixture_table(void* o){return *(void***)o;}
static u32 fixture_release(void* o){return ((u32(WIN*)(void*))fixture_table(o)[2])(o);}
static void* fixture_surface(void* draw){
 u32 d[27]={108,0x1007};d[2]=fixture_input.height;d[3]=fixture_input.width;d[18]=32;d[19]=0x40;d[21]=16;d[22]=0xf800;d[23]=0x7e0;d[24]=31;d[26]=0x840;
 void* out=0;fixture_require(!((CreateSurface)fixture_table(draw)[6])(draw,d,&out,0),201);return out;
}
static u16* fixture_pixels(void* o,const u16* input,u16* output,u32 poison,u32* pitch){
 u32 d[27]={108};fixture_require(!((Lock)fixture_table(o)[25])(o,0,d,1,0),202);
 for(u32 y=0;y<fixture_input.height;++y)for(u32 x=0;x<fixture_input.width;++x){
  u16* p=(u16*)((u8*)d[9]+y*d[4])+x;u32 at=y*fixture_input.width+x;
  if(input)*p=input[at];if(poison)*p=(u16)poison;if(output)output[at]=*p;
 }
 u16* base=(u16*)d[9];*pitch=d[4];fixture_require(!((Unlock)fixture_table(o)[32])(o,base),203);return base;
}
static void fixture_clip(void* draw,void* target){
 if(!fixture_input.clip)return;void* c=0;
 fixture_require(!((i32(WIN*)(void*,u32,void**,void*))fixture_table(draw)[4])(draw,0,&c,0),204);
 zero(fixture_region,sizeof(fixture_region));fixture_region[0]=32;fixture_region[1]=1;
 u32 count=fixture_input.clip==1?1:fixture_input.clip==2?2:fixture_input.clip==4?256:0;
 fixture_region[2]=count;fixture_region[3]=count*16;fixture_region[6]=fixture_input.width;fixture_region[7]=fixture_input.height;
 if(fixture_input.clip==1){i32 r[4]={2,1,10,7};copy(fixture_region+8,r,16);}
 if(fixture_input.clip==2){i32 r[8]={0,0,5,3,7,5,13,9};copy(fixture_region+8,r,32);}
 if(fixture_input.clip==4)for(u32 i=0;i<256;++i){u32* r=fixture_region+8+4*i;r[0]=2*i;r[1]=0;r[2]=2*i+1;r[3]=1;}
 fixture_require(!((i32(WIN*)(void*,const u32*,u32))fixture_table(c)[7])(c,fixture_region,0),205);
 fixture_require(!((i32(WIN*)(void*,void*))fixture_table(target)[28])(target,c),206);fixture_release(c);
}
static struct GameSurface* fixture_owned(void* object,u16* physical,u32 pitch,u32 gpu,const u16* initial){
 struct GameSurface* s=game_surface_find(object,1);fixture_require(s!=0,207);
 s->clip_known=1;s->clip=fixture_input.clip!=0;s->layout_known=1;s->gpu_history=gpu;s->pixels.width=fixture_input.width;s->pixels.height=fixture_input.height;
 s->pixels.bits=16;s->pixels.flags=0x40;s->pixels.r=0xf800;s->pixels.g=0x7e0;s->pixels.b=31;s->pixels.length=s->pixels.width*s->pixels.height*2;
 if(!fixture_input.bootstrap || object==game_surfaces[0].object){
  struct Snapshot baseline;zero(&baseline,sizeof(baseline));fixture_require(game_surface_blank(&baseline,&s->pixels),208);
  for(u32 y=0;y<s->pixels.height;++y)copy(baseline.data+y*s->pixels.width*2,(u8*)physical+y*pitch,s->pixels.width*2);
  s->pixels.data=baseline.data;game_surface_bytes+=baseline.length;__atomic_sub_fetch(&lock_capture_reserved,baseline.length,__ATOMIC_RELAXED);
 }
 if(initial){struct Snapshot image;copy(&image,&s->pixels,sizeof(image));image.data=(u8*)initial;s->gpu_history=0;fixture_require(session_surface(object,&image,0)!=0,209);s->gpu_history=gpu;}
 return s;
}
static void fixture_refusals(void* source,void* target){
 fixture_plan.version=0xfeed1234;
 fixture_require(fixture_real_plan(target,fixture_input.dr,source,fixture_input.sr,0,&fixture_plan,sizeof(fixture_plan)-4)<0 && fixture_plan.version==0xfeed1234,233);
 fixture_require(fixture_real_plan(target,fixture_input.dr,(void*)1,fixture_input.sr,0,&fixture_plan,sizeof(fixture_plan))<0 && fixture_plan.version==0xfeed1234,234);
 fixture_require(fixture_real_plan(target,fixture_input.dr,source,fixture_input.sr,1,&fixture_plan,sizeof(fixture_plan))<0 && fixture_plan.version==0xfeed1234,235);
 u32 descriptor[27]={108};fixture_require(!((Lock)fixture_table(source)[25])(source,0,descriptor,1,0),236);
 fixture_require(fixture_real_plan(target,fixture_input.dr,source,fixture_input.sr,0,&fixture_plan,sizeof(fixture_plan))==(i32)0x887601aeu && fixture_plan.version==0xfeed1234,237);
 fixture_require(!((Unlock)fixture_table(source)[32])(source,(void*)descriptor[9]),238);
 u32 range[2]={3,4};fixture_require(!((i32(WIN*)(void*,u32,const u32*))fixture_table(source)[29])(source,8,range),239);
 fixture_require(fixture_real_plan(target,fixture_input.dr,source,fixture_input.sr,0x8000,&fixture_plan,sizeof(fixture_plan))<0 && fixture_plan.version==0xfeed1234,240);
 u32 key[2]={fixture_source[5],fixture_source[5]};
 fixture_require(!((i32(WIN*)(void*,u32,const u32*))fixture_table(source)[29])(source,8,fixture_input.keyed?key:0),241);
 if(!fixture_input.keyed)fixture_require(fixture_real_plan(target,fixture_input.dr,source,fixture_input.sr,0x8000,&fixture_plan,sizeof(fixture_plan))<0 && fixture_plan.version==0xfeed1234,242);
}
void start(void){
 u32 bytes;HANDLE in=CreateFileA("input.bin",0x80000000,1,0,3,0,0);
 fixture_require(in!=(HANDLE)-1 && ReadFile(in,&fixture_input,sizeof(fixture_input),&bytes,0) && bytes==sizeof(fixture_input),210);CloseHandle(in);
 fixture_require(fixture_input.width && fixture_input.width<=512 && fixture_input.height && fixture_input.height<=9,211);
 const u32 count=fixture_input.width*fixture_input.height;
 for(u32 i=0;i<count;++i){fixture_source[i]=(u16)(0x1000+i*73);fixture_target[i]=(u16)(0xa000+i);}
 void* module=LoadLibraryA("MnmDraw.dll");fixture_require(module!=0,212);
 CreateDraw create=(CreateDraw)GetProcAddress(module,"NativeDrawCreate");
 game_cpu_history=(i32(WIN*)(const void*,u32,u32,u32,u32,u32))GetProcAddress(module,"NativeDrawCpuHistoryV1");
 game_native_gpu_copy=(i32(WIN*)(void*,const i32*,void*,const i32*,u32,u16*,u32))GetProcAddress(module,"NativeDrawGpuCopyV1");
 fixture_real_plan=(i32(WIN*)(void*,const i32*,void*,const i32*,u32,struct MnmGpuCopyPlan*,u32))GetProcAddress(module,"NativeDrawGpuCopyPlanV1");game_native_gpu_plan=fixture_record_plan;
 fixture_require(create && game_cpu_history && game_native_gpu_copy && fixture_real_plan,213);
 void *draw=0,*source=0,*target=0,*alias=0;fixture_require(!create(0,&draw,0),214);source=fixture_surface(draw);target=fixture_input.self?source:fixture_surface(draw);
 u32 sp,dp;u16* sb=fixture_pixels(source,fixture_source,0,0,&sp);u16* db=fixture_input.self?sb:fixture_pixels(target,fixture_target,0,0,&dp);if(fixture_input.self)dp=sp;
 fixture_clip(draw,target);
 u32 key[2]={fixture_source[5],fixture_source[5]};if(fixture_input.keyed)fixture_require(!((i32(WIN*)(void*,u32,const u32*))fixture_table(source)[29])(source,8,key),215);
 i32 expected=((Blt)fixture_table(target)[5])(target,fixture_input.dr,source,fixture_input.sr,fixture_input.keyed?0x8000:0,0);
 fixture_pixels(target,0,fixture_expected,0,&dp);
 /* Distinct CPU backings deliberately do not contain either GPU image. */
 fixture_pixels(source,0,0,0x4d4d,&sp);
 if(!fixture_input.self)fixture_pixels(target,fixture_input.current_target?fixture_target:0,0,fixture_input.current_target?0:0x5e5e,&dp);
 game_session_enabled=game_session_archive=game_resident_world=game_copy_order_enabled=1;lock_capture_path[0]='.';lock_capture_path_length=1;
 struct GameSurface* src=fixture_owned(source,sb,sp,1,fixture_source);
 struct GameSurface* dst=fixture_input.self?src:fixture_owned(target,db,dp,!fixture_input.current_target && !fixture_input.bootstrap,fixture_input.bootstrap?0:fixture_target);
 fixture_require((src==dst)==(fixture_input.self!=0),244);
 src->clip=fixture_input.self && fixture_input.clip;src->key_known=fixture_input.keyed;src->key=fixture_input.keyed?key[0]:0;
 fixture_plan.version=0xfeed1234;
 fixture_require(fixture_real_plan(target,fixture_input.dr,source,fixture_input.sr,0,&fixture_plan,sizeof(fixture_plan))<0 && fixture_plan.version==0xfeed1234,243);
 fixture_require(!game_cpu_history(sb,1,2,0,5,6),216);
 if(!fixture_input.self && !fixture_input.current_target)fixture_require(!game_cpu_history(db,1,1,0,5,6),217);
 fixture_refusals(source,target);
 if(fixture_input.self==2){
  static const u8 iid[16]={0x30,0x86,0x2b,0x0b,0x35,0xad,0xd0,0x11,0x8e,0xa6,0,0x60,0x97,0x97,0xea,0x5b};
  fixture_require(!((Query)fixture_table(source)[0])(source,iid,&alias),218);game_alias_observed(source,alias,14);target=alias;
 }
 struct GameCopyLease lease=game_copy_enter(target);fixture_require(lease.observe && lease.held,219);
 struct GameBlit p;game_blit_before(target,fixture_input.dr,source,fixture_input.sr,fixture_input.keyed?0x8000:0,0,0,0,0,&p);
 fixture_require(p.valid && p.gpu,220);i32 result=game_gpu_blit(&p);game_copy_leave(&lease);
 fixture_require(result==expected && p.gpu==2 && fixture_calls==1,221);
 for(u32 y=0;y<fixture_input.height;++y)for(u32 x=0;x<fixture_input.width;++x){
  fixture_require(*(u16*)((u8*)sb+y*sp+x*2)==0x4d4d,222);
  if(!fixture_input.self)fixture_require(*(u16*)((u8*)db+y*dp+x*2)==(fixture_input.current_target?fixture_target[y*fixture_input.width+x]:0x5e5e),223);
 }
 const int stale=game_cpu_history(db,2,0,0,0,0)<0;
 fixture_require(stale==(!fixture_input.current_target || fixture_plan.count!=0),224);
 fixture_require(session_active && lock_capture_reserved==0,225);
 fixture_require(game_surface_pixels_enter(target),226);fixture_require(session_present(session_find(target)),227);session_finish_owned();game_tracker_release();
 fixture_require(!session_active && session_pixels==2*count-(fixture_input.self?count:0),228);
 HANDLE out=CreateFileA("output.bin",0x40000000,0,0,1,0,0);
 u32 header[12]={0x50435047,1,fixture_input.id,fixture_input.width,fixture_input.height,(u32)result,fixture_plan.count,fixture_checks,session_sequence,stale,0,0};
 fixture_require(out!=(HANDLE)-1 && WriteFile(out,header,sizeof(header),&bytes,0) && bytes==sizeof(header),229);
 fixture_require(WriteFile(out,&fixture_plan,sizeof(fixture_plan),&bytes,0) && bytes==sizeof(fixture_plan),230);
 fixture_require(WriteFile(out,fixture_expected,count*2,&bytes,0) && bytes==count*2,231);CloseHandle(out);
 for(u32 i=0;i<GAME_SURFACE_COUNT;++i)if(game_surfaces[i].object)game_surface_drop(game_surfaces+i);
 if(alias)fixture_release(alias);if(!fixture_input.self)fixture_release(target);fixture_release(source);fixture_release(draw);
 u32(WIN* owners)(void)=(u32(WIN*)(void))GetProcAddress(module,"NativeDrawObjects");fixture_require(owners && owners()==0 && game_surface_bytes==0 && lock_capture_reserved==0,232);
 ExitProcess(0);
}
