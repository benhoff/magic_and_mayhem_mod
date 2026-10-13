#include "../runtime/shadow/win32_min.h"
API u32 WIN GetCurrentThreadId(void);
API HANDLE WIN CreateThread(void*,u32,u32(WIN*)(void*),void*,u32,u32*);
API u32 WIN WaitForSingleObject(HANDLE,u32);
static u32 depth;
#include "../runtime/scene/producer_trace_guard.h"
#define MNM_PRODUCER_COPY 6u
static u32 sequence=10,queue=1;
static char directory[512];
struct Frame{u32 owner,tag,caller,copy_order_id;u8* record;};
static void copy(void* to,const void* from,u32 n){u8* d=to;const u8* s=from;while(n--)*d++=*s++;}
static int equal(const void* a,const void* b,u32 n){const u8* x=a;const u8* y=b;while(n--)if(*x++!=*y++)return 0;return 1;}
static int write(HANDLE file,const void* data,u32 n){u32 done;return WriteFile(file,data,n,&done,0)&&done==n;}
static void path(char* out,const char* name,u32 number,const char* suffix){u32 i=0;while(directory[i]){out[i]=directory[i];++i;}out[i++]='\\';while(*name)out[i++]=*name++;for(u32 d=1000;d;d/=10)out[i++]=(char)('0'+number/d%10);while(*suffix)out[i++]=*suffix++;out[i]=0;}
#include "../runtime/scene/drawing_copy_order.h"
static void require(int condition,u32 error){if(!condition)ExitProcess(error);}
static u32 WIN worker(void* unused){
 (void)unused;u32 record[24]={96,0,6,5,1,40,40,0,0,0,440,310,480,350,0xffffffff};
 struct Frame f={GetCurrentThreadId(),33,0x4a4c26,0,(u8*)record};u32 before[24];copy(before,record,96);
 {PRODUCER_TRACE_SCOPE;sequence=11;SetLastError(0xca01);copy_order_record(&f,1);require(GetLastError()==0xca01&&equal(before,record,96)&&f.copy_order_id==2,151);}
 {PRODUCER_TRACE_SCOPE;sequence=12;record[1]=12;copy(before,record,96);SetLastError(0xca02);copy_order_record(&f,2);require(GetLastError()==0xca02&&equal(before,record,96),152);}
 return 0;
}
void start(void){
 require(GetEnvironmentVariableA("MNM_FIXTURE_DIRECTORY",directory,sizeof(directory))>0,150);
 require(producer_trace_configure()&&!*producer_trace_depth(GetCurrentThreadId()),153);char mode[16];u32 n=GetEnvironmentVariableA("MNM_COPY_ORDER_CASE",mode,sizeof(mode));
 if(n&&mode[0]=='i'){require(!copy_order_open()&&!copy_order_file,154);ExitProcess(0);}
 require(copy_order_open(),155);
 u32 record[24]={96,0,6,1,2,800,600,0,0,0,0,0,800,600,0xffffffff};
 struct Frame f={GetCurrentThreadId(),7,0x46ba01,0,(u8*)record};u32 before[24];copy(before,record,96);
 if(n&&mode[0]=='d'){copy_order_record(&f,1);require(!copy_order_file&&!f.copy_order_id&&equal(before,record,96),156);ExitProcess(0);}
 if(n&&mode[0]=='b'){copy_order_rows=MNM_COPY_ORDER_MAX_RECORDS-1;copy_order_record(&f,1);ExitProcess(157);}
 {PRODUCER_TRACE_SCOPE;SetLastError(0xca03);copy_order_record(&f,1);require(GetLastError()==0xca03&&f.copy_order_id==1&&equal(before,record,96),158);}
 if(n&&mode[0]=='p'){copy_order_close();require(copy_order_active==1,159);ExitProcess(0);}
 u32 id=0;HANDLE thread=CreateThread(0,0,worker,0,0,&id);require(thread&&WaitForSingleObject(thread,5000)==0&&CloseHandle(thread),160);
 {PRODUCER_TRACE_SCOPE;sequence=13;record[1]=13;copy(before,record,96);SetLastError(0xca04);copy_order_record(&f,2);require(GetLastError()==0xca04&&equal(before,record,96),161);}
 SetLastError(0xca05);copy_order_close();require(GetLastError()==0xca05&&!copy_order_active&&!copy_order_file&&copy_order_requests==2,162);ExitProcess(0);
}
