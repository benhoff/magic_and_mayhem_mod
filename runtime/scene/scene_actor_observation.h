/* Included after continuous generation state. Passive queue-entry sidecar only. */
#include "../../protocols/include/mnm/scene_actor_observation_v2.h"
static HANDLE sao_file;
static u32 sao_checked,sao_enabled,sao_frames,sao_bytes,sao_failed,sao_owner;
static u32 sao_pool,sao_capacity,sao_epoch;
static u32 sao_tokens[MNM_SAO_TOKENS],sao_token_count;
static u8 sao_body[MNM_SAO_ACTOR*MNM_SAO_ACTORS+MNM_SAO_DRAW*MNM_SAO_DRAWS];
static u8 sao_frame_bytes[1024u*1024u];
typedef u32(WIN* SaoGameplayPoint)(u32,u32*,u32);
static SaoGameplayPoint sao_gameplay;
static int sao_open(void){
 if(sao_checked)return sao_enabled;
 sao_checked=1;
 char flag[2];u32 n=GetEnvironmentVariableA("MNM_SCENE_ACTORS",flag,2);
 if(!n)return 0;
 if(n!=1||flag[0]!='1'||!continuous_map||!continuous_resident||!continuous_public_bind){sao_failed=1;return 0;}
 char name[260];path(name,"scene-actors-",1,".bin");
 sao_file=CreateFileA(name,0x40000000,1,0,1,0x80,0);
 if(sao_file==(HANDLE)-1){sao_file=0;sao_failed=2;return 0;}
 u32 h[16]={0};copy(h,MNM_SAO_MAGIC,8);h[2]=MNM_SAO_VERSION;h[3]=64;h[4]=MNM_SAO_BUILD;
 h[5]=MNM_SAO_FRAMES;h[6]=MNM_SAO_ACTOR;h[7]=MNM_SAO_DRAW;h[8]=MNM_SAO_BYTES;
 if(!write(sao_file,h,64)){sao_failed=3;return 0;}sao_bytes=64;sao_enabled=1;return 1;
}
static void sao_close(void){
 if(!sao_file)return;
 u32 h[16]={0};copy(h,MNM_SAO_END,8);h[2]=MNM_SAO_VERSION;h[3]=64;h[4]=sao_frames;h[5]=sao_bytes;
 h[6]=sao_failed;h[7]=sao_owner;
 write(sao_file,h,64);CloseHandle(sao_file);sao_file=0;sao_enabled=0;
}
static u32 sao_token(u32 p){
 for(u32 i=0;i<sao_token_count;++i)if(sao_tokens[i]==p)return i+1;
 if(sao_token_count>=MNM_SAO_TOKENS)return 0;
 sao_tokens[sao_token_count++]=p;return sao_token_count;
}
/* Same inclusive-bound/frame-zero choice as the selected original helper.
 * Unreadable/oversized inputs are observations with absent validity, not calls
 * into the original null-display initializer or a claimed original safety rule.
 */
static u32 sao_role(u32 display,u32 asset,u8* ani,u32* bound,u32* index){
 if(!readable(display,44)||get((void*)display)!=0||!readable(asset,0x114))return 0;
 copy(ani,(void*)display,44);
 u32 spr=get((void*)(asset+0x110));if(!readable(spr,16))return 0;
 u32 header=get((void*)spr),table=get((void*)(spr+8));if(!readable(header,16))return 0;
 *bound=get((void*)(header+12));if(*bound>8192||!readable(table,(*bound+1)*4))return 0;
 *index=get((void*)(display+4));if(*index>*bound)*index=0;
 u32 offset=get((void*)(table+*index*4)),data=get((void*)(spr+12));
 if(offset>0xffffffffu-data)return 0;
 u32 frame=data+offset;if(!readable(frame,40))return 0;
 u32 bytes=get((void*)frame);if(bytes<40||bytes>sizeof(sao_frame_bytes)||!readable(frame,bytes))return 0;
 return frame;
}
static void sao_capture(u32 object){
 if(!sao_open()||sao_failed)return;
 u32 owner=GetCurrentThreadId();
 if((sao_owner&&owner!=sao_owner)||sao_frames>=MNM_SAO_FRAMES||!readable(object,24)){
  sao_failed=4;sao_close();return;
 }
 sao_owner=owner;
 u32 base=get((void*)object),count=get((void*)(object+8)),capacity=get((void*)(object+12)),view=get((void*)(object+20));
 if(!count||count>12320||count>capacity||view>3||!readable(base,count*36)){
  sao_failed=5;sao_close();return;
 }
 u32 h[32]={0},actors=0,draws=0;sao_token_count=0;
 h[1]=sao_frames+1;h[2]=continuous_generation_low;h[3]=continuous_generation_high;
 h[4]=continuous_nonce_low;h[5]=continuous_nonce_high;h[6]=owner;h[7]=GetTickCount();
 h[8]=get((void*)0x6c4830);h[9]=view;h[10]=count;h[16]=sequence;
 struct Surface* target=bound();if(target){h[17]=target->id;h[18]=target->width;h[19]=target->height;}
 h[20]=get((void*)(object+16));h[21]=get((void*)(object+4));h[22]=get((void*)0x6e9850);
 if(!sao_gameplay){void* menu=GetModuleHandleA("MnmMenu.dll");
  sao_gameplay=menu?(SaoGameplayPoint)GetProcAddress(menu,"MenuGameplayPointV1"):0;}
 u32 point[4]={0};if(sao_gameplay&&sao_gameplay(owner,point,16)==1&&point[0]==1&&point[1]==16&&point[2]==MNM_SAO_BUILD){h[15]=point[3];h[23]|=2;}
 if(readable(0x6def58,8)){
  u32 pool=get((void*)0x6def58),n=get((void*)0x6def5c);
  if(pool&&n&&n<=256&&n<=0xffffffffu/0xe4bu&&readable(pool,n*0xe4b)){
   if(pool!=sao_pool||n!=sao_capacity){sao_pool=pool;sao_capacity=n;++sao_epoch;}
   h[11]=sao_epoch;h[12]=n;h[23]|=1;
   for(u32 slot=0;slot<n;++slot){u32 actor=pool+slot*0xe4b;
    if(get((void*)actor)!=slot||!get((void*)(actor+4)))continue;
    u32* out=(u32*)(sao_body+actors*MNM_SAO_ACTOR);zero(out,MNM_SAO_ACTOR);
    out[0]=slot;out[1]=get((void*)(actor+0xa8));out[2]=get((void*)(actor+0x174));out[3]=get((void*)(actor+4));
    out[4]=get((void*)(actor+0xe4));out[5]=get((void*)(actor+0xe8));
    out[6]=get((void*)(actor+8));out[7]=get((void*)(actor+12));out[8]=get((void*)(actor+16));
    out[9]=get((void*)(actor+0x5ec));out[10]=get((void*)(actor+0x5fc));out[11]=get((void*)(actor+0x64c));
    u32 type=get((void*)(actor+0xac));if(readable(type,12))out[13]=get((void*)(type+8));
    out[14]=get((void*)(actor+0x7cd));out[15]=get((void*)(actor+0x60c));
    const u32 disp[MNM_SAO_ROLES]={actor+0xb4,actor+0x7d9,actor+0x829,actor+0x130,actor+0x85d,actor+0x891,
      0x6df150,0x6df060,0x6df090,0x6df0f0,0x6df0c0,0x6df120};
    const u32 asset[MNM_SAO_ROLES]={actor+0xcc,actor+0x7f1,actor+0x841,actor+0x148,actor+0x875,actor+0x8a9,
      0x6df168,0x6df078,0x6df0a8,0x6df108,0x6df0d8,0x6df138};
    const u32 gate[MNM_SAO_ROLES]={1,out[14],1,get((void*)(actor+0x124)),get((void*)(actor+0x889)),
      get((void*)(actor+0x8bd)),get((void*)(actor+0x8d1)),get((void*)(actor+0x780)),get((void*)(actor+0x8d5)),
      get((void*)(actor+0x963)),*(u8*)(actor+0x962),get((void*)(actor+0x78c))};
    for(u32 role=0;role<MNM_SAO_ROLES;++role){
     out[MNM_SAO_GATES+role]=gate[role];
     if(!gate[role]||(role==1&&gate[role]!=1&&gate[role]!=3)||
        (role!=0&&role!=2&&role!=5&&!out[4])||!readable(disp[role],4)||!readable(asset[role],4))continue;
     u32 frame=sao_role(get((void*)disp[role]),get((void*)asset[role]),(u8*)(out+MNM_SAO_ANI+role*11),
                       out+MNM_SAO_BOUNDS+role,out+MNM_SAO_INDICES+role);
     if(frame){out[16+role]=sao_token(frame);out[12]|=1u<<role;}
    }
    ++actors;
   }
  }
 }
 /* Actor region is fixed first; draws follow only the used actor bytes. */
 u32 at=actors*MNM_SAO_ACTOR;
 for(u32 a=0;a<actors;++a){u32* actor=(u32*)(sao_body+a*MNM_SAO_ACTOR);
  for(u32 role=0;role<MNM_SAO_ROLES;++role){u32 token=actor[16+role];if(!token)continue;
   u32 frame=sao_tokens[token-1],length=get((void*)frame);
   copy(sao_frame_bytes,(void*)frame,length);zero(sao_frame_bytes+28,4);
   u32 hash=mnm_cont_crc32c(sao_frame_bytes,length),matched=0;
   for(u32 row=0;row<count;++row){const u8* r=(const u8*)base+row*36;
    if(get(r+4)!=frame)continue;
    if(draws>=MNM_SAO_DRAWS||++matched>16){sao_failed=6;sao_close();return;}
    u32* d=(u32*)(sao_body+at);zero(d,MNM_SAO_DRAW);
    d[0]=actor[0];d[1]=role;d[2]=row;d[3]=token;d[4]=get(r);d[5]=get(r+8);d[6]=get(r+12);
    d[7]=get(r+24);d[8]=get(r+16);d[9]=get(r+28);d[10]=get(r+32);
    d[11]=length;d[12]=hash;d[13]=get((void*)(frame+4));d[14]=get((void*)(frame+8));
    d[15]=get((void*)(frame+12));d[16]=get((void*)(frame+16));d[17]=actor[MNM_SAO_INDICES+role];
    d[18]=get((void*)(frame+28))!=0xffffffffu;
    ++draws;at+=MNM_SAO_DRAW;
   }
  }
 }
 h[0]=MNM_SAO_FRAME+at;h[13]=actors;h[14]=draws;h[24]=mnm_cont_crc32c(sao_body,at);
 if(h[0]>MNM_SAO_BYTES-sao_bytes-64||!write(sao_file,h,128)||!write(sao_file,sao_body,at)){
  sao_failed=7;sao_close();return;
 }
 ++sao_frames;sao_bytes+=h[0];
}
