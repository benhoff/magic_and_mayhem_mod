/* Process-local raw queue diagnostics only; never a native rendering input. */
#include "../shadow/win32_min.h"
static u32 limit,calls,current;
static u8* bytes;
static char directory[220];
static u32 get(const void* p){const u8* b=p;return b[0]|(u32)b[1]<<8|(u32)b[2]<<16|(u32)b[3]<<24;}
static void put(void* p,u32 v){u8* b=p;b[0]=v;b[1]=v>>8;b[2]=v>>16;b[3]=v>>24;}
static int readable(u32 p,u32 n){u32 end=p+n;if(!p||end<p)return 0;while(p<end){u32 m[7];if(VirtualQuery((void*)p,m,28)!=28||m[4]!=0x1000||(m[5]&0x101)||!(m[5]&0xee))return 0;u32 next=m[0]+m[3];if(next<=p)return 0;p=next<end?next:end;}return 1;}
void startup_queue_init(void){
    char number[8];u32 n=GetEnvironmentVariableA("MNM_STARTUP_QUEUES",number,8),value=0;
    if(!n||n>=8)return;
    for(u32 i=0;i<n;++i){if(number[i]<'0'||number[i]>'9')return;value=value*10+number[i]-'0';}
    if(!value||value>256)return;
    n=GetEnvironmentVariableA("MNM_SCENE_DIR",directory,sizeof(directory));if(!n||n>=sizeof(directory))return;
    limit=value;
}
void startup_queue_begin(u32* registers){
    if(!limit||calls>=limit)return;
    current=++calls;
    /* Allocate the fixed header even for an invalid/empty queue. */
    bytes=HeapAlloc(GetProcessHeap(),0,64+12320*36);if(!bytes)return;
    for(u32 i=0;i<64;++i)bytes[i]=0;
    const char* magic="MNMSTQ01";for(u32 i=0;i<8;++i)bytes[i]=magic[i];
    put(bytes+8,1);put(bytes+12,64);put(bytes+16,64);put(bytes+20,current);put(bytes+52,36);
    u32 queue=registers[6];put(bytes+44,queue);
    if(!readable(queue,24)){put(bytes+28,1);return;}
    u32 base=get((void*)queue),count=get((void*)(queue+8)),capacity=get((void*)(queue+12));
    put(bytes+32,get((void*)(queue+20)));put(bytes+36,count);put(bytes+40,capacity);put(bytes+48,base);
    if(count>12320||count>capacity){put(bytes+28,2);return;}
    if(count&&!readable(base,count*36)){put(bytes+28,3);return;}
    put(bytes+56,count);put(bytes+16,64+count*36);
    for(u32 i=0;i<count*36;++i)bytes[64+i]=((u8*)base)[i];
}
void startup_queue_end(u32 sample){
    if(!current)return;
    if(bytes){
        put(bytes+24,sample);
        char path[260];u32 n=0;while(directory[n]){path[n]=directory[n];++n;}path[n++]='\\';
        const char* name="startup-queue-";while(*name)path[n++]=*name++;
        for(u32 d=1000;d;d/=10)path[n++]=(char)('0'+current/d%10);
        name=".bin";while(*name)path[n++]=*name++;path[n]=0;
        HANDLE file=CreateFileA(path,0x40000000,0,0,1,0x80,0);
        if(file!=(HANDLE)-1){u32 size=get(bytes+16),done=0;
            while(done<size){u32 wrote=0;if(!WriteFile(file,bytes+done,size-done,&wrote,0)||!wrote||wrote>size-done)break;done+=wrote;}
            CloseHandle(file);
        }
        HeapFree(GetProcessHeap(),0,bytes);bytes=0;
    }
    /* Missing/partial output is a harness failure, never a skipped queue. */
    current=0;
}
