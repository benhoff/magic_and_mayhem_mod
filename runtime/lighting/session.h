#pragma once
#include "../../protocols/include/mnm/terrain_light_session_v1.h"
API u32 WIN GetFileSize(HANDLE,u32*);
API HANDLE WIN CreateFileMappingA(HANDLE,void*,u32,u32,u32,const char*);
API void* WIN MapViewOfFile(HANDLE,u32,u32,u32,u32);
API int WIN UnmapViewOfFile(const void*);
static u32* session;static HANDLE session_file,session_mapping;
static u32 sessionPhase(void){return __atomic_load_n(session+MNM_LS_PHASE,__ATOMIC_ACQUIRE);}
static void sessionPublish(u32 value){__atomic_store_n(session+MNM_LS_PHASE,value,__ATOMIC_RELEASE);}
static void sessionCount(u32 word){if(session[word]!=0xffffffffu)++session[word];}
static int sessionStart(const char* name){
 session_file=CreateFileA(name,0xc0000000,3,0,3,0x80,0);if(session_file==(HANDLE)-1)return 0;
 u32 high=0,size=GetFileSize(session_file,&high);if(high||size!=MNM_LIGHT_SESSION_BYTES){CloseHandle(session_file);return 0;}
 session_mapping=CreateFileMappingA(session_file,0,4,0,MNM_LIGHT_SESSION_BYTES,0);if(!session_mapping){CloseHandle(session_file);return 0;}
 session=MapViewOfFile(session_mapping,2|4,0,0,MNM_LIGHT_SESSION_BYTES);
 if(!session){CloseHandle(session_mapping);CloseHandle(session_file);return 0;}
 if(!equal(session,MNM_LIGHT_SESSION_MAGIC,8)||session[2]!=1||session[3]!=MNM_LIGHT_V1_MAX_BYTES||sessionPhase()!=MNM_LS_IDLE||session[11]>1||session[12]>2||session[13]||session[31]||(!session[9]&&!session[10]))goto refused;
 for(u32 i=14;i<31;++i)if(session[i])goto refused;
 nonce[0]=session[9];nonce[1]=session[10];replace_mode=session[11];return 1;
refused:UnmapViewOfFile(session);session=0;CloseHandle(session_mapping);CloseHandle(session_file);return 0;
}
static void sessionDisable(void){session[MNM_LS_DISABLED]=1;sessionPublish(MNM_LS_CLOSED);}
static int sessionWait(u32 wanted){
 u32 start=GetTickCount();
 do{u32 phase=sessionPhase();if(phase==wanted){u32 n=GetTickCount()-start;if(n>session[MNM_LS_MAX_WAIT])session[MNM_LS_MAX_WAIT]=n;return 1;}if(phase==MNM_LS_DECLINED||phase==MNM_LS_CLOSED)break;Sleep(1);}while(GetTickCount()-start<MNM_LS_WAIT_MS);
 u32 n=GetTickCount()-start;if(n>session[MNM_LS_MAX_WAIT])session[MNM_LS_MAX_WAIT]=n;sessionDisable();return 0;
}
static int sessionReply(const u8* input,u32 map,u32 count){
 u32 size=80+count*5;const u8* b=(const u8*)session+128+MNM_LIGHT_V1_MAX_BYTES;
 if(session[MNM_LS_REPLY_LENGTH]!=size||!equal(b,MNM_LIGHT_V1_REPLY,8)||get(b+8)!=1||get(b+12)!=size||get(b+16)!=sequence||get(b+20)!=nonce[0]||get(b+24)!=nonce[1]||get(b+28)!=get(input+120)||get(b+32)||get(b+36)>=8||get(b+52)>1||get(b+48)>65536||get(b+56)>1||get(b+68)>65536||get(b+72)>1||get(b+76)>1){sessionDisable();return 0;}
 if(replace_mode){
  for(u32 i=0;i<5;++i)copy((void*)get((void*)(map+0x7cc+i*4)),b+80+i*count,count);
  const u32 a[11]={0x6e8d90,0x6e8d94,0x6e8d98,0x6e8d9c,0,0x6e8da0,0x6e8da4,0x6e8da8,0x6e8dac,0,0};
  for(u32 i=0;i<11;++i)put((void*)(a[i]?a[i]:map+(i==4?0x642e:i==9?0x642a:0x6426)),get(b+36+i*4));put((void*)0x5e41a8,0);
 }
 return 1;
}
static void sessionHook(void* receiver,u32 caller){
 typedef void(__attribute__((thiscall))*Call)(void*);u32 map=(u32)receiver;
 sessionCount(MNM_LS_CALLS);session[MNM_LS_CALLER]=caller;session[MNM_LS_CLOCK]=get((void*)0x6c4830);
 if(session[MNM_LS_DISABLED]||sequence==0xffffffffu){
  sessionDisable();sessionCount(MNM_LS_FALLBACK);busy=1;u8* witness=0;u32 witness_length=0;
  if(session[MNM_LS_TRACE]&&__atomic_load_n(session+MNM_LS_FALLBACK_SAMPLE,__ATOMIC_ACQUIRE)==1)witness=capture(map,caller,&witness_length);
  ((Call)trampoline)(receiver);
  if(session[MNM_LS_TRACE]&&__atomic_load_n(session+MNM_LS_FALLBACK_SAMPLE,__ATOMIC_ACQUIRE)==1){
   if(witness){copy((u8*)session+128,witness,witness_length);session[MNM_LS_INPUT_LENGTH]=witness_length;u32 count=get(witness+44);readResult((u8*)session+128+2*MNM_LIGHT_V1_MAX_BYTES,map,count);session[MNM_LS_RESULT_LENGTH]=48+count*5;__atomic_store_n(session+MNM_LS_FALLBACK_SAMPLE,2,__ATOMIC_RELEASE);}
   else __atomic_store_n(session+MNM_LS_FALLBACK_SAMPLE,3,__ATOMIC_RELEASE);
  }
  if(witness)HeapFree(GetProcessHeap(),0,witness);busy=0;return;
 }
 busy=1;++sequence;u32 length=0;u8* input=capture(map,caller,&length);int accepted=0;
 if(input&&sessionPhase()==MNM_LS_IDLE){
  copy((u8*)session+128,input,length);session[MNM_LS_INPUT_LENGTH]=length;session[MNM_LS_SEQUENCE]=sequence;sessionPublish(MNM_LS_REQUEST);
  accepted=sessionWait(MNM_LS_REPLY)&&sessionReply(input,map,get(input+44));
 }else{sessionCount(MNM_LS_REFUSED);if(input)sessionDisable();else --sequence;}
 if(accepted&&replace_mode)sessionCount(MNM_LS_APPLIED);else{if(replace_mode)sessionCount(MNM_LS_FALLBACK);((Call)trampoline)(receiver);}
 if(accepted){
  if(session[MNM_LS_TRACE]==1||(session[MNM_LS_TRACE]==2&&session[MNM_LS_ARM]&&caller==0x472201))session[MNM_LS_TRACE_ACTIVE]=1;
  if(session[MNM_LS_TRACE_ACTIVE]){u32 count=get(input+44);readResult((u8*)session+128+2*MNM_LIGHT_V1_MAX_BYTES,map,count);session[MNM_LS_RESULT_LENGTH]=48+count*5;sessionPublish(MNM_LS_RESULT);if(sessionWait(MNM_LS_ACK))sessionPublish(MNM_LS_IDLE);}
  else sessionPublish(MNM_LS_IDLE);
 }
 session[MNM_LS_CHANGED]=get((void*)0x5e41a8);if(input)HeapFree(GetProcessHeap(),0,input);busy=0;
}
