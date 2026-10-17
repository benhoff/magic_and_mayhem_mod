#include "../../protocols/include/mnm/drawing_copy_order_v1.h"
API u32 WIN GetTickCount(void);
static HANDLE copy_order_file;
static u32 copy_order_rows,copy_order_requests,copy_order_active;
static int copy_order_open(void){
 char flag[2];SetLastError(0);u32 n=GetEnvironmentVariableA("MNM_DRAW_COPY_ORDER",flag,sizeof(flag));
 if(!n)return GetLastError()==203;
 if(n!=1||flag[0]!='1'||!producer_trace_mode)return 0;
 char name[260];path(name,"copy-order-",0,".bin");
 copy_order_file=CreateFileA(name,0x40000000,3,0,1,0x80,0);
 if(copy_order_file==(HANDLE)-1){copy_order_file=0;return 0;}
 u32 h[16]={0};copy(h,MNM_COPY_ORDER_MAGIC,8);h[2]=1;h[3]=128;h[4]=0x40209ca7;h[5]=MNM_COPY_ORDER_MAX_RECORDS;
 return write(copy_order_file,h,sizeof(h));
}
static void copy_order_emit(u32* row){
 if(copy_order_rows>=MNM_COPY_ORDER_MAX_RECORDS-1)ExitProcess(99);
 row[0]=++copy_order_rows;
 if(!write(copy_order_file,row,128))ExitProcess(99);
}
static void copy_order_record(struct Frame* f,u32 phase){
 if(!copy_order_file||!f->record||((u32*)f->record)[2]!=MNM_PRODUCER_COPY)return;
 u32 error=GetLastError();const u32* request=(const u32*)f->record;
 if(phase==1){if(copy_order_requests==~0u)ExitProcess(99);f->copy_order_id=++copy_order_requests;++copy_order_active;}
 else if(!f->copy_order_id||!copy_order_active)ExitProcess(99);
 else --copy_order_active;
 u32 row[32]={0};row[1]=phase;row[2]=f->copy_order_id;row[3]=f->owner;row[4]=f->tag;row[5]=GetTickCount();row[6]=sequence;row[7]=queue;
 for(u32 i=0;i<12;++i)row[8+i]=request[3+i];
 if(f->caller>=0x400000&&f->caller<0x5c0000)row[20]=f->caller-0x400000;
 if(phase==2)row[21]=request[1];row[24]=copy_order_active;copy_order_emit(row);SetLastError(error);
}
static void copy_order_close(void){
 if(!copy_order_file)return;u32 error=GetLastError();u32 row[32]={0};row[1]=3;row[2]=copy_order_requests;row[6]=sequence;row[7]=queue;row[24]=copy_order_active;
 row[0]=++copy_order_rows;if(copy_order_rows>MNM_COPY_ORDER_MAX_RECORDS||!write(copy_order_file,row,128)||!CloseHandle(copy_order_file))ExitProcess(99);
 copy_order_file=0;SetLastError(error);
}
