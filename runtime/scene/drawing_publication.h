/* Opt-in original COM result / primary readback acquisition. Included after
 * producer helpers; all pixel files are comparison oracles, never replay input. */
extern void drawing_publication_blt(void),drawing_publication_fast(void);
API u32 WIN GetCurrentThreadId(void);
static HANDLE publication_file,action_file;
static u32 action_records;
static u32 publication_enabled,publication_attempts,publication_frames,publication_drops,publication_terminal,publication_bytes;
#include "drawing_physical.h"
static int publication_install(void){
 char flag[2];SetLastError(0);u32 n=GetEnvironmentVariableA("MNM_DRAW02_PUBLICATION",flag,2);
 if(!n)return GetLastError()==203;
 if(n!=1||flag[0]!='1'||limit>16||bypass_limit||raster_first||!physical_init())return 0;
 const u32 sites[2]={0x58c03a,0x58c05a};
 const u8 expected[2][5]={{0xff,0x52,0x14,0xeb,0x1e},{0xff,0x52,0x1c,0x8b,0xf0}};
 void(*entries[2])(void)={drawing_publication_blt,drawing_publication_fast};
 for(u32 i=0;i<2;++i)if(!readable(sites[i],5))return 0;else for(u32 j=0;j<5;++j)if(*(u8*)(sites[i]+j)!=expected[i][j])return 0;
 char name[260];path(name,"drawing-publication-",0,".bin");
 publication_file=CreateFileA(name,0x40000000,3,0,1,0x80,0);if(publication_file==(HANDLE)-1){publication_file=0;return 0;}
 u32 header[8]={0};copy(header,"MNMDP002",8);header[2]=2;header[3]=96;header[4]=0x40209ca7;header[5]=65536;header[6]=1024;
 if(!write(publication_file,header,32))return 0;
 path(name,"drawing-actions-",0,".bin");action_file=CreateFileA(name,0x40000000,3,0,1,0x80,0);if(action_file==(HANDLE)-1){action_file=0;return 0;}
 u32 actions[8]={0};copy(actions,"MNMDAC01",8);actions[2]=1;actions[3]=64;actions[4]=0x40209ca7;actions[5]=8192;
 if(!write(action_file,actions,32))return 0;
 for(u32 i=0;i<2;++i){u32 old,unused;u8 patch[5]={0xe9};put(patch+1,(u32)entries[i]-sites[i]-5);
  if(!VirtualProtect((void*)sites[i],5,0x40,&old))return 0;copy((void*)sites[i],patch,5);
  int okay=FlushInstructionCache(GetCurrentProcess(),(void*)sites[i],5);if(!VirtualProtect((void*)sites[i],5,old,&unused)||!okay)return 0;
 }
 publication_enabled=1;return 1;
}
static void publication_close(void){if(action_file){CloseHandle(action_file);action_file=0;}if(publication_file){CloseHandle(publication_file);publication_file=0;}}
void drawing_publication_observe(u32* registers,u32 kind){
 u32 error=GetLastError();if(!publication_enabled||!publication_file||stopped){SetLastError(error);return;}
 /* At both sites EBX is destination and EDI source; wrapper objects are local
  * observations only. Export process-local generation IDs rather than pointers. */
 u32 dst_object=registers[4],src_object=registers[0];
 struct Surface* dst=object(dst_object),*src=object(src_object);
 u32 r[24]={0};r[0]=++publication_attempts;r[1]=kind;r[2]=registers[7];r[3]=error;r[4]=sequence;r[5]=queue;
 r[6]=src?src->id:0;r[7]=dst?dst->id:0;r[8]=get((void*)0x6f98c0)==dst_object;
 r[9]=kind==1?0x1000000:0x10; /* Flags from these exact guarded original sites. */
 r[10]=0xffffffffu;r[11]=0xffffffffu;
 if(r[8]&&(i32)r[2]>=0){
  u32 desc[27]={0};desc[0]=108;
  typedef i32(WIN *Lock)(void*,void*,void*,u32,HANDLE);
  typedef i32(WIN *Unlock)(void*,void*);
  u32 surface=readable(dst_object+8,4)?get((void*)(dst_object+8)):0;
  u32 table=surface&&readable(surface,4)?get((void*)surface):0;
  if(!table||!readable(table,132)){r[10]=0x80004005u;}
  else{
   SetLastError(error);i32 locked=((Lock)get((void*)(table+100)))((void*)surface,0,desc,0x4810,0);r[10]=locked;
   if(locked>=0){
    r[12]=desc[3];r[13]=desc[2];r[14]=desc[21];r[15]=desc[22];r[16]=desc[23];r[17]=desc[24];r[18]=desc[4];
    u32 w=desc[3],h=desc[2],pitch=desc[4],p=desc[9];
    if(w&&h&&w<=2048&&h<=2048&&desc[21]==16&&desc[22]==0xf800&&desc[23]==0x7e0&&desc[24]==31&&pitch>=w*2&&pitch<=8192&&readable(p,(h-1)*pitch+w*2)&&publication_frames<1024&&w*h*2<=oracle_byte_limit-publication_bytes&&publication_bytes<=oracle_byte_limit){
     char name[260];path(name,"publication-oracle-",publication_frames+1,".565");HANDLE out=CreateFileA(name,0x40000000,0,0,1,0x80,0);
     int okay=out!=(HANDLE)-1;if(okay){for(u32 y=0;y<h;++y)if(!write(out,(void*)(p+y*pitch),w*2)){okay=0;break;}if(!CloseHandle(out))okay=0;}
     if(okay){r[19]=++publication_frames;publication_bytes+=w*h*2;}
    }
    r[11]=((Unlock)get((void*)(table+128)))((void*)surface,(void*)desc[9]);
   }
  }
  if(!r[19]||(i32)r[11]<0)++publication_drops;
  if(queue>=limit)publication_terminal=1;
 }
 r[20]=publication_drops;r[21]=publication_frames;r[22]=GetCurrentThreadId();r[23]=action_records;
 if(publication_attempts>65536||!write(publication_file,r,96)){canvas_producers_close();ExitProcess(95);}
 physical_fence(r);
 SetLastError(error);
}

/* Entry/return bracket the actually executed adapter dispatch on the original
 * menu thread. Rejected requests are retained, not relabeled successful. */
void WIN SceneDrawingActionV1(u32 request,u32 action,u32 screen,u32 generation,u32 argument,u32 phase,u32 status){
 u32 error=GetLastError();if(publication_enabled&&action_file&&!stopped){
  u32 r[16]={0};r[0]=++action_records;r[1]=phase;r[2]=request;r[3]=action;r[4]=screen;r[5]=generation;r[6]=argument;r[7]=status;
  r[8]=sequence;r[9]=publication_attempts;r[10]=publication_frames+1;r[11]=GetCurrentThreadId();r[12]=queue;
  if(action_records>8192||!write(action_file,r,64)){canvas_producers_close();ExitProcess(95);}
 }SetLastError(error);
}
