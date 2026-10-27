/* Compare production CPU preparation/commands against the facade's actual
 * clipped result, then replay those commands through the native GL backend. */
#define start gpu_fixture_start
#include "native-gpu-copy-plan-probe.c"
#undef start
void start(void){
 u32 bytes;HANDLE in=CreateFileA("input.bin",0x80000000,1,0,3,0,0);
 fixture_require(in!=(HANDLE)-1 && ReadFile(in,&fixture_input,sizeof(fixture_input),&bytes,0) && bytes==sizeof(fixture_input),210);CloseHandle(in);
 const u32 count=fixture_input.width*fixture_input.height,fill=fixture_input.current_target;
 for(u32 i=0;i<count;++i){fixture_source[i]=(u16)(0x1000+i*73);fixture_target[i]=(u16)(0xa000+i);}
 void* module=LoadLibraryA("MnmDraw.dll");fixture_require(module!=0,212);
 CreateDraw create=(CreateDraw)GetProcAddress(module,"NativeDrawCreate");
 game_native_cpu_plan=(i32(WIN*)(void*,const i32*,void*,const i32*,u32,struct MnmGpuCopyPlan*,u32))GetProcAddress(module,"NativeDrawCpuCopyPlanV1");
 fixture_require(create && game_native_cpu_plan,213);
 void *draw=0;fixture_require(!create(0,&draw,0),214);
 void* source=fixture_surface(draw);void* target=fixture_surface(draw);
 u32 sp,dp;u16* sb=fixture_pixels(source,fixture_source,0,0,&sp);u16* db=fixture_pixels(target,fixture_target,0,0,&dp);
 fixture_clip(draw,target);
 u32 key[2]={fixture_source[5],fixture_source[5]};if(fixture_input.keyed)fixture_require(!((i32(WIN*)(void*,u32,const u32*))fixture_table(source)[29])(source,8,key),215);
 u32 fx[25]={100};fx[20]=0x3c5a;
 const u32 flags=fill?0x400:fixture_input.keyed?0x8000:0;
 i32 expected=((Blt)fixture_table(target)[5])(target,fixture_input.dr,fill?0:source,fill?0:fixture_input.sr,flags,fill?fx:0);
 fixture_pixels(target,0,fixture_expected,0,&dp);fixture_pixels(target,fixture_target,0,0,&dp);
 game_session_enabled=game_session_archive=game_resident_world=game_copy_order_enabled=1;lock_capture_path[0]='.';lock_capture_path_length=1;
 struct GameSurface* src=fixture_owned(source,sb,sp,0,0);fixture_owned(target,db,dp,0,0);
 src->clip=0;src->key_known=fixture_input.keyed;src->key=key[0];
 struct GameCopyLease lease=game_copy_enter(target);fixture_require(lease.observe && lease.held,219);
 struct GameBlit p;game_blit_before(target,fixture_input.dr,fill?0:source,fill?0:fixture_input.sr,flags,fill?fx:0,0,0,0,&p);
 fixture_require(p.valid && !p.gpu,220);
 zero(&fixture_plan,sizeof(fixture_plan));fixture_plan.version=1;fixture_plan.bytes=sizeof(fixture_plan);
 if(p.plan)copy(&fixture_plan,p.plan,sizeof(fixture_plan));
 i32 result=((Blt)fixture_table(target)[5])(target,fixture_input.dr,fill?0:source,fill?0:fixture_input.sr,flags,fill?fx:0);
 fixture_require(result==expected,221);game_blit_after(&p,result);game_copy_leave(&lease);
 struct GameSurface* dst=game_surface_find(target,0);
 fixture_require(dst && dst->pixels.data && !dst->gpu_history && session_active,222);
 for(u32 i=0;i<count;++i)fixture_require(((u16*)dst->pixels.data)[i]==fixture_expected[i],223);
 fixture_require(game_surface_pixels_enter(target),226);fixture_require(session_present(session_find(target)),227);session_finish_owned();game_tracker_release();
 fixture_require(!session_active && lock_capture_reserved==0,228);
 HANDLE out=CreateFileA("output.bin",0x40000000,0,0,1,0,0);
 u32 header[12]={0x50435047,1,fixture_input.id,fixture_input.width,fixture_input.height,(u32)result,fixture_plan.count,fixture_checks,session_sequence,0,0,0};
 fixture_require(out!=(HANDLE)-1 && WriteFile(out,header,sizeof(header),&bytes,0) && bytes==sizeof(header),229);
 fixture_require(WriteFile(out,&fixture_plan,sizeof(fixture_plan),&bytes,0) && bytes==sizeof(fixture_plan),230);
 fixture_require(WriteFile(out,fixture_expected,count*2,&bytes,0) && bytes==count*2,231);CloseHandle(out);
 for(u32 i=0;i<GAME_SURFACE_COUNT;++i)if(game_surfaces[i].object)game_surface_drop(game_surfaces+i);
 fixture_release(target);fixture_release(source);fixture_release(draw);
 u32(WIN* owners)(void)=(u32(WIN*)(void))GetProcAddress(module,"NativeDrawObjects");fixture_require(owners && owners()==0 && game_surface_bytes==0 && lock_capture_reserved==0,232);
 ExitProcess(0);
}
