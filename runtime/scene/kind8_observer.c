/* Read-only, finite object/class diagnostics. Never a native raster input. */
#include "../shadow/win32_min.h"
static u32 limit,calls;
static char directory[220];
static u32 get(const void* p){const u8* b=p;return b[0]|(u32)b[1]<<8|(u32)b[2]<<16|(u32)b[3]<<24;}
static void put(void* p,u32 v){u8* b=p;b[0]=v;b[1]=v>>8;b[2]=v>>16;b[3]=v>>24;}
static int readable(u32 p,u32 n){u32 end=p+n;if(!p||end<p)return 0;while(p<end){u32 m[7];if(VirtualQuery((void*)p,m,28)!=28||m[4]!=0x1000||(m[5]&0x101)||!(m[5]&0xee))return 0;u32 next=m[0]+m[3];if(next<=p)return 0;p=next<end?next:end;}return 1;}
void kind8_init(void){
    char number[8];u32 n=GetEnvironmentVariableA("MNM_KIND8_QUEUES",number,8),value=0;if(!n||n>=8)return;
    for(u32 i=0;i<n;++i){if(number[i]<'0'||number[i]>'9')return;value=value*10+number[i]-'0';}
    if(!value||value>3600)return;
    n=GetEnvironmentVariableA("MNM_SCENE_DIR",directory,sizeof(directory));if(!n||n>=sizeof(directory))return;limit=value;
}
void kind8_queue(u32* registers){
    if(!limit||calls>=limit)return;u32 sequence=++calls,error=GetLastError(),queue=registers[6];
    if(!readable(queue,24))goto done;
    u32 base=get((void*)queue),count=get((void*)(queue+8)),capacity=get((void*)(queue+12));
    if(count>12320||count>capacity||!readable(base,count*36))goto done;
    u32 total=0;for(u32 i=0;i<count;++i)if(get((void*)(base+i*36+24))==8)++total;
    if(!total)goto done;
    u32 captured=total>512?512:total,size=80+captured*160;
    u8* bytes=HeapAlloc(GetProcessHeap(),0,size);if(!bytes)goto done;
    for(u32 i=0;i<size;++i)bytes[i]=0;
    const char* magic="MNMK8OB1";for(u32 i=0;i<8;++i)bytes[i]=magic[i];
    put(bytes+8,1);put(bytes+12,80);put(bytes+16,size);put(bytes+20,sequence);put(bytes+24,captured);put(bytes+28,total);put(bytes+32,total>captured);put(bytes+36,0x40209ca7);
    const u32 globals[]={0x5e1404,0x6de6e1,0x6e1f88,0x658174,0x6a49b8,0x656618,0x6a2dc8,0x6e0008,0x6cbb6c};
    for(u32 i=0;i<9;++i)if(readable(globals[i],4))put(bytes+40+i*4,get((void*)globals[i]));
    u32 row=0;
    for(u32 i=0;i<count&&row<captured;++i){const u8* original=(const u8*)(base+i*36);if(get(original+24)!=8)continue;
        u8* r=bytes+80+row++*160;put(r,i);for(u32 j=0;j<36;++j)r[24+j]=original[j];
        u32 object=get(original+4);put(r+8,object);
        if(!readable(object,80)){put(r+4,1);continue;}
        for(u32 j=0;j<80;++j)r[60+j]=((u8*)object)[j];u32 vtable=get((void*)object);put(r+12,vtable);
        if(!readable(vtable,20)){put(r+4,2);continue;}
        for(u32 j=0;j<20;++j)r[140+j]=((u8*)vtable)[j];put(r+16,get((void*)(vtable+12)));
    }
    char path[260];u32 n=0;while(directory[n]){path[n]=directory[n];++n;}path[n++]='\\';
    const char* name="kind8-";while(*name)path[n++]=*name++;for(u32 d=1000;d;d/=10)path[n++]=(char)('0'+sequence/d%10);
    name=".bin";while(*name)path[n++]=*name++;path[n]=0;
    HANDLE file=CreateFileA(path,0x40000000,0,0,1,0x80,0);
    if(file!=(HANDLE)-1){u32 at=0;while(at<size){u32 wrote=0;if(!WriteFile(file,bytes+at,size-at,&wrote,0)||!wrote||wrote>size-at)break;at+=wrote;}CloseHandle(file);}
    HeapFree(GetProcessHeap(),0,bytes);
done:SetLastError(error);
}
