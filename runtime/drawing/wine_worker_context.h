#include "../../protocols/include/mnm/drawing_worker_context_v1.h"
API HANDLE WIN CreateFileMappingA(HANDLE,void*,u32,u32,u32,const char*);
API void* WIN MapViewOfFile(HANDLE,u32,u32,u32,u32);
static HANDLE wine_worker_context_file,wine_worker_context_mapping;
static volatile u32* wine_worker_context_words;
static u32 wine_worker_context_sequence;
static int wine_worker_context_open(void){
 char flag[2];u32 n=GetEnvironmentVariableA("MNM_DRAW_WINE_WORKER_CONTEXT",flag,2);if(!n)return 1;if(n!=1||flag[0]!='1'||!wine_blit_packet_enabled)return 0;
 char name[600];path(name,"wine-worker-context.bin");wine_worker_context_file=CreateFileA(name,0xc0000000,3,0,1,0x80,0);if(wine_worker_context_file==(HANDLE)-1)return 0;
 u32 h[16]={0};copy(h,MNM_DRAW_WORKER_CONTEXT_MAGIC,8);h[2]=1;h[3]=64;if(!write(wine_worker_context_file,h,64))return 0;
 wine_worker_context_mapping=CreateFileMappingA(wine_worker_context_file,0,4,0,64,0);if(!wine_worker_context_mapping)return 0;wine_worker_context_words=MapViewOfFile(wine_worker_context_mapping,2,0,0,64);return wine_worker_context_words!=0;
}
static void wine_worker_context_publish(const void* packet,u32 worker,u32 phase){
 if(!wine_worker_context_words)return;
 if(phase==2&&wine_worker_context_words[5])ExitProcess(99);
 if(phase==3&&(!wine_worker_context_words[5]||wine_worker_context_words[6]!=worker))ExitProcess(99);
 u32 command=0,submission=0;if(phase==2)for(u32 i=0;i<2048;++i)if(wine_blit_packets[i].pointer==packet){command=wine_blit_packets[i].command;submission=wine_blit_packets[i].submission;break;}
 wine_worker_context_sequence+=2;__atomic_store_n(wine_worker_context_words+4,wine_worker_context_sequence-1,__ATOMIC_SEQ_CST);
 wine_worker_context_words[5]=phase==2;wine_worker_context_words[6]=phase==2?worker:0;wine_worker_context_words[7]=command;wine_worker_context_words[8]=submission;wine_worker_context_words[9]=phase==2?GetCurrentThreadId():0;wine_worker_context_words[10]=phase==2?wine_lifetime_records:0;wine_worker_context_words[11]=phase==2&&!command?1:0;
 __atomic_store_n(wine_worker_context_words+4,wine_worker_context_sequence,__ATOMIC_SEQ_CST);
}
