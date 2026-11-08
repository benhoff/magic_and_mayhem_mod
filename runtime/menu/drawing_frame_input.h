/* Opt-in, streamed, same-owner frame/input witness. Actor identities are
 * observed pool/slot identities, never allocation generations. */
static HANDLE frame_input_file;
static u32 frame_input_count;
typedef u32(WIN *FrameIdentity)(u32,u32*,u32);
static FrameIdentity frame_input_identity;
static HudInputPoint frame_input_cursor;
static int frame_input_open(void){
 char name[2048];u32 n=GetEnvironmentVariableA("MNM_DRAW_FRAME_INPUT_FILE",name,sizeof(name));
 if(!n)return 1;if(n>=sizeof(name)||!region_input_file||!region_input_track_enabled)return 0;
 frame_input_file=CreateFileA(name,0x40000000,3,0,1,0x80,0);
 if(frame_input_file==(HANDLE)-1){frame_input_file=0;return 0;}
 u32 h[4]={0};copy(h,"MNMIFR01",8);h[2]=1;h[3]=384;u32 wrote=0;
 if(!WriteFile(frame_input_file,h,16,&wrote,0)||wrote!=16){CloseHandle(frame_input_file);frame_input_file=0;return 0;}
 return 1;
}
static void frame_input_sample(u32 kind,u32 call,void* object,u32 mask,const u32* words,u32 result,u32 error){
 if(!frame_input_file)return;
 u8 fp[512] __attribute__((aligned(16)));region_input_fp_save(fp);
 u32 r[96];for(u32 i=0;i<96;++i)((volatile u32*)r)[i]=0;RegionMouseRecord mouse={{0}};
 if(words)copy(mouse.words,words,40);
 region_input_fields(r,object,mask,mouse,result,error);
 r[0]=++frame_input_count;r[1]=call;r[2]=kind;r[3]=admission_world_returns;
 if(!frame_input_identity){void* m=GetModuleHandleA("MnmScene.dll");if(m)frame_input_identity=(FrameIdentity)GetProcAddress(m,"SceneWorldFrameObservationV1");}
 if(frame_input_identity&&frame_input_identity(r[5],r+56,32)==1)r[7]|=256;
 if(!frame_input_cursor){void* m=GetModuleHandleA("MnmRender.dll");if(m)frame_input_cursor=(HudInputPoint)GetProcAddress(m,"RenderCursorObservationV1");}
 r[64]=frame_input_cursor?frame_input_cursor(r[5],r+65,64):0;
 if(readable((void*)0x642c54,4)&&readable((void*)0x641aa0,8)){
  r[81]=1;r[82]=get((void*)0x642c54);r[83]=get((void*)0x641aa0);r[84]=get((void*)0x641aa4);
 }
 if((r[45]&2)&&readable((void*)MNM_REGION_INPUT_POOL,8)){
  u32 base=get((void*)MNM_REGION_INPUT_POOL),slot=r[55];
  if(base&&slot<get((void*)(MNM_REGION_INPUT_POOL+4))&&base<=~0u-slot*0xe4bu){
   u32 actor=base+slot*0xe4bu;
   if(readable((void*)actor,20)&&get((void*)actor)==slot){r[85]=1;for(u32 i=0;i<3;++i)r[86+i]=get((void*)(actor+8+i*4));}
   if(readable((void*)actor,0x760)&&get((void*)actor)==slot){r[89]=1;r[90]=get((void*)(actor+0x754));r[91]=get((void*)(actor+0x758));r[92]=get((void*)(actor+0x75c));}
  }
 }
 if(readable((void*)0x6de6d5,4)){r[93]=1;r[94]=get((void*)0x6de6d5);}
 if(frame_input_count>=8192)r[7]|=0x80000000u;
 u32 wrote=0;
 if(!WriteFile(frame_input_file,r,sizeof(r),&wrote,0)||wrote!=sizeof(r)||r[7]&0x80000000u){CloseHandle(frame_input_file);frame_input_file=0;}
#ifdef MNM_UI_ACCEPTANCE_OBSERVATION
 if(kind==1)ui_acceptance_sample(1,0,0,result,error);
#endif
 region_input_fp_restore(fp);SetLastError(error);
}
static void frame_input_close(void){if(frame_input_file){CloseHandle(frame_input_file);frame_input_file=0;}}
