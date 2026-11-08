/* Opt-in read-only UI observations. Original callbacks still execute once.
 * Numeric addresses are session-local observations, never stable identities. */
static HANDLE ui_acceptance_file;
static u32 ui_acceptance_count,ui_acceptance_call,ui_acceptance_menu_tick;
static u32 ui_acceptance_health_object;
typedef u32 (THIS *UiTextFn)(void*,const char*);
typedef u32 (THIS *UiHealthFn)(void*,u32);
static UiTextFn ui_acceptance_text_original;
static UiHealthFn ui_acceptance_health_original;
static char ui_acceptance_text[256];
static void ui_acceptance_sample(u32 kind,u32 call,u32 argument,u32 result,u32 error){
 if(!ui_acceptance_file)return;
 u32 now=GetTickCount();
 if(kind==4){if(now-ui_acceptance_menu_tick<50)return;ui_acceptance_menu_tick=now;}
 u8 fp[512] __attribute__((aligned(16)));region_input_fp_save(fp);
 u32 r[128];for(u32 i=0;i<128;++i)((volatile u32*)r)[i]=0;
 r[0]=++ui_acceptance_count;r[1]=kind;r[2]=call;r[3]=now;r[4]=GetCurrentThreadId();
 r[5]=admission_world_returns;r[12]=menu_screen;r[13]=menu_generation;r[14]=menu_request;r[15]=menu_status;
 if(frame_input_identity&&frame_input_identity(r[4],r+56,32)==1)r[50]|=4;
 if(readable((void*)0x6903e8,4))r[18]=get((void*)0x6903e8);
 /* Match the existing semantic-state reader: +fd62 is the render sequence;
  * the pause mode is the byte at +fd5a, not either input or render state. */
 if(readable((void*)MNM_STATE_WORLD,0xfd66)&&get((void*)MNM_STATE_WORLD)==0x5c5dd8){
  r[50]|=32;r[10]=*(u8*)(MNM_STATE_WORLD+0xfd5a);r[7]=get((void*)(MNM_STATE_WORLD+0xfd62));
 }
 if(readable((void*)0x6c4830,4))r[11]=get((void*)0x6c4830);
 if(readable((void*)0x6f34e0,4)){u32 top=get((void*)0x6f34e0);if(top&&readable((void*)top,12))r[16]=get((void*)(top+4));}
 if(readable((void*)0x6f349c,4))r[17]=get((void*)0x6f349c);
 if(readable((void*)0x657fe8,0x18a)){
  u32 p=0x657fe8;r[50]|=1;r[20]=get((void*)(p+4));r[21]=get((void*)(p+0x17a));
  r[22]=get((void*)(p+8));r[25]=get((void*)(p+0x128));r[26]=get((void*)(p+0x17e));r[27]=get((void*)(p+0x14));
  for(u32 i=0;i<8;++i)r[28+i]=get((void*)(p+0x15a+i*4));
  if(r[22]&&readable((void*)r[22],24)){r[23]=get((void*)r[22]);r[24]=get((void*)(r[22]+20));}
 }
 if(readable((void*)0x6def58,8)){
  u32 base=get((void*)0x6def58),capacity=get((void*)0x6def5c);
  if(base&&capacity&&capacity<=256&&readable((void*)base,0x178)&&get((void*)base)==0){
   r[50]|=2;r[37]=base;r[38]=0;r[39]=get((void*)(base+0xa8));r[40]=get((void*)(base+0x174));
   r[41]=get((void*)(base+4));r[42]=get((void*)(base+0xe4));r[43]=get((void*)(base+0xe8));r[44]=get((void*)(base+0x68));r[6]=get((void*)(base+0xec));
  }
 }
 r[48]=result;r[49]=error;r[51]=ui_acceptance_health_object;r[55]=argument;
 if(r[51]&&readable((void*)r[51],0x1f3)){
  r[52]=get((void*)(r[51]+0xc7));r[53]=get((void*)(r[51]+0xcb));r[54]=get((void*)(r[51]+0x1ef));r[50]|=16;
 }
 for(u32 i=0;i<256;++i){((u8*)(r+64))[i]=ui_acceptance_text[i];if(!ui_acceptance_text[i]){r[47]=i;r[50]|=8;break;}}
 if(ui_acceptance_count>=8192)r[50]|=0x80000000u;
 u32 wrote=0;
 if(!WriteFile(ui_acceptance_file,r,sizeof(r),&wrote,0)||wrote!=sizeof(r)||r[50]&0x80000000u){CloseHandle(ui_acceptance_file);ui_acceptance_file=0;}
 region_input_fp_restore(fp);SetLastError(error);
}
static u32 THIS ui_acceptance_text_callback(void* object,const char* text){
 u8 fp[512] __attribute__((aligned(16)));u32 error=GetLastError(),call=++ui_acceptance_call;region_input_fp_save(fp);
 for(u32 i=0;i<256;++i){ui_acceptance_text[i]=0;if(!text||!readable(text+i,1))break;ui_acceptance_text[i]=text[i];if(!text[i])break;}
 ui_acceptance_sample(2,call,(u32)object,0,error);region_input_fp_restore(fp);SetLastError(error);
 u32 result=ui_acceptance_text_original(object,text);error=GetLastError();region_input_fp_save(fp);
 ui_acceptance_sample(3,call,(u32)object,result,error);region_input_fp_restore(fp);SetLastError(error);return result;
}
static u32 THIS ui_acceptance_health_callback(void* object,u32 value){
 u8 fp[512] __attribute__((aligned(16)));u32 error=GetLastError(),call=++ui_acceptance_call;region_input_fp_save(fp);
 ui_acceptance_health_object=(u32)object;ui_acceptance_sample(5,call,value,0,error);region_input_fp_restore(fp);SetLastError(error);
 u32 result=ui_acceptance_health_original(object,value);error=GetLastError();region_input_fp_save(fp);
 ui_acceptance_sample(6,call,value,result,error);region_input_fp_restore(fp);SetLastError(error);return result;
}
static int ui_acceptance_open(void){
 char path[2048];u32 n=GetEnvironmentVariableA("MNM_DRAW_UI_ACCEPTANCE_FILE",path,sizeof(path));if(!n)return 1;if(n>=sizeof(path))return 0;
 const u8 text[]={0x83,0xec,0x34,0x53,0x55,0x56};
 const u8 health[]={0x83,0xec,0x10,0x56,0x8b,0xf1};
 if(!readable((void*)0x4dba60,6)||!equal((void*)0x4dba60,text,6)||!readable((void*)0x4d3a30,6)||!equal((void*)0x4d3a30,health,6))return 0;
 ui_acceptance_text_original=(UiTextFn)trampoline(0x4dba60,6,0);
 ui_acceptance_health_original=(UiHealthFn)trampoline(0x4d3a30,6,0);
 if(!ui_acceptance_text_original||!ui_acceptance_health_original)return 0;
 HANDLE f=CreateFileA(path,0x40000000,3,0,1,0x80,0);if(f==(HANDLE)-1)return 0;
 u32 header[4]={0},wrote=0;copy(header,"MNMUIA01",8);header[2]=3;header[3]=512;
 if(!WriteFile(f,header,16,&wrote,0)||wrote!=16){CloseHandle(f);return 0;}
 u32 a,b,restore;if(!VirtualProtect((void*)0x4dba60,6,0x40,&a)){CloseHandle(f);return 0;}
 if(!VirtualProtect((void*)0x4d3a30,6,0x40,&b)){VirtualProtect((void*)0x4dba60,6,a,&restore);CloseHandle(f);return 0;}
 ui_acceptance_file=f;jump(0x4dba60,(u32)&ui_acceptance_text_callback,6);jump(0x4d3a30,(u32)&ui_acceptance_health_callback,6);
 VirtualProtect((void*)0x4d3a30,6,b,&restore);VirtualProtect((void*)0x4dba60,6,a,&restore);
 FlushInstructionCache(GetCurrentProcess(),(void*)0x4dba60,6);FlushInstructionCache(GetCurrentProcess(),(void*)0x4d3a30,6);return 1;
}
static void ui_acceptance_close(void){if(ui_acceptance_file){CloseHandle(ui_acceptance_file);ui_acceptance_file=0;}}
