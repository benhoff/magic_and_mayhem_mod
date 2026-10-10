#include "../shadow/win32_min.h"
#include "../../protocols/include/mnm/scene_snapshot_v1.h"
#include "../../protocols/include/mnm/canvas_producers_v3.h"
extern void scene_enter(void);
extern void world_begin(u32*,u32);
extern int canvas_producers_install(void);
extern void canvas_producers_close(void),canvas_producers_queue(u32*),canvas_producers_protect(void);
extern int world_stream_init(void);
extern void world_stream_close(void);
extern int world_stream_history(void),world_stream_queue(u32);
extern void world_stream_missing(void),world_stream_fail(u32);
extern void world_lifetime_observe(u32*);
extern void world_lifetime_close(void);
extern int canvas_startup_install(void);
extern void canvas_startup_world(u32*);
extern void canvas_startup_close(void);
extern void startup_queue_init(void);
extern void kind8_init(void),kind8_queue(u32*);
extern void startup_queue_begin(u32*);
extern void startup_queue_end(u32);
u32 scene_trampoline;
static u32 image_base,samples,limit=4;
static volatile u32 busy;
static u32 calls,skip,interval=1,continuous,kind8_samples;
static char directory[220];
static const u8 expected[9]={0x83,0xec,0x18,0x53,0xb8,0x00,0x6c,0xca,0x88};
static u32 pointers[MNM_SCENE_V1_MAX_BLOBS];
static u32 get(const void* p){const u8* b=p;return b[0]|(u32)b[1]<<8|(u32)b[2]<<16|(u32)b[3]<<24;}
static void put(void* p,u32 v){u8* b=p;b[0]=v;b[1]=v>>8;b[2]=v>>16;b[3]=v>>24;}
static void copy(void* to,const void* from,u32 n){u8* d=to;const u8* s=from;while(n--)*d++=*s++;}
static void zero(void* to,u32 n){u8* d=to;while(n--)*d++=0;}
static int equal(const void* a,const void* b,u32 n){const u8* x=a;const u8* y=b;while(n--)if(*x++!=*y++)return 0;return 1;}
static int readable(u32 at,u32 size){
    u32 end=at+size;if(!at||end<at)return 0;
    while(at<end){u32 m[7];if(VirtualQuery((void*)at,m,28)!=28||m[4]!=0x1000||(m[5]&0x101)||!(m[5]&0xee))return 0;
        u32 next=m[0]+m[3];if(next<=at)return 0;at=next<end?next:end;}return 1;
}
static int save(const u8* bytes,u32 size){
    char path[260];u32 at=0;while(directory[at]){path[at]=directory[at];++at;}path[at++]='\\';
    const char* name="scene-";while(*name)path[at++]=*name++;
    for(u32 d=1000;d;d/=10)path[at++]=(char)('0'+samples/d%10);
    const char* suffix=".bin";while(*suffix)path[at++]=*suffix++;path[at]=0;
    HANDLE file=CreateFileA(path,0x40000000,0,0,1,0x80,0);if(file==(HANDLE)-1)return 0;
    u32 done=0;while(done<size){u32 wrote=0;if(!WriteFile(file,bytes+done,size-done,&wrote,0)||!wrote||wrote>size-done){CloseHandle(file);return 0;}done+=wrote;}
    return CloseHandle(file)!=0;
}
void scene_observe(u32* registers){
    u32 error=GetLastError();if(!__sync_bool_compare_and_swap(&busy,0,1)){SetLastError(error);return;}
    u8* bytes=0;u32 saved_sample=0;
    startup_queue_begin(registers);
    kind8_queue(registers);
    canvas_producers_queue(registers);
    canvas_startup_world(registers);
    world_lifetime_observe(registers);
    if(!continuous&&samples>=limit)goto done;
    if(calls++<skip||(calls-skip-1)%interval)goto done;
    if(continuous){if(world_stream_queue(calls)){world_begin(registers,0);world_stream_missing();}goto done;}
    /* pushal ECX is word 6. All reads happen on the original engine thread. */
    u32 queue=registers[6];if(!readable(queue,24))goto done;
    u32 base=get((void*)queue),count=get((void*)(queue+8)),capacity=get((void*)(queue+12)),view=get((void*)(queue+20));
    if(!count||count>MNM_SCENE_V1_MAX_DRAWS||count>capacity||view>3||!readable(base,count*36))goto done;
    if(kind8_samples){u32 found=0;for(u32 i=0;i<count;++i)if(get((void*)(base+i*36+24))==8){found=1;break;}if(!found)goto done;}
    bytes=HeapAlloc(GetProcessHeap(),0,MNM_SCENE_V1_MAX_BYTES);if(!bytes)goto done;
    u32 blobs=0,at=64+count*32;zero(bytes,at);copy(bytes,MNM_SCENE_V1_MAGIC,8);
    put(bytes+8,1);put(bytes+12,64);put(bytes+24,count);put(bytes+32,view);put(bytes+40,MNM_SCENE_V1_BUILD);put(bytes+48,64);put(bytes+52,at);
    if(readable(image_base+0x1e1404,4))put(bytes+36,get((void*)(image_base+0x1e1404)));
    if(readable(image_base+0x2a49b8,4))put(bytes+56,get((void*)(image_base+0x2a49b8)));
    if(readable(image_base+0x256618,4))put(bytes+60,get((void*)(image_base+0x256618)));
    for(u32 i=0;i<count;++i){
        const u8* r=(const u8*)base+i*36;u8* out=bytes+64+i*32;u32 kind=get(r+24),pointer=get(r+4),token=0,flags=0;
        put(out+4,get(r));put(out+8,get(r+8));put(out+12,get(r+12));put(out+16,kind);put(out+20,get(r+16));put(out+24,get(r+28));
        if(kind==0xfffffffe)flags=MNM_SCENE_V1_HIDDEN;
        else if(readable(pointer,40)){
            u32 length=get((void*)pointer);
            if(length>=40&&length<=MNM_SCENE_V1_MAX_FRAME&&readable(pointer,length)){
                for(u32 j=0;j<blobs;++j)if(pointers[j]==pointer){token=j+1;break;}
                if(!token){
                    if(blobs>=MNM_SCENE_V1_MAX_BLOBS||length>MNM_SCENE_V1_MAX_BYTES-at-16)goto done;
                    pointers[blobs++]=pointer;token=blobs;put(bytes+at,token);put(bytes+at+4,length);
                    put(bytes+at+8,get((void*)(pointer+28))!=0xffffffff?MNM_SCENE_V1_INDEXED:0);put(bytes+at+12,0);
                    copy(bytes+at+16,(void*)pointer,length);put(bytes+at+16+28,0);at+=16+length;
                }
            }
        }
        if(!token&&!(flags&MNM_SCENE_V1_HIDDEN))flags|=MNM_SCENE_V1_NO_FRAME;
        put(out,token);put(out+28,flags);
    }
    ++samples;put(bytes+16,at);put(bytes+20,samples);put(bytes+28,blobs);if(save(bytes,at)){saved_sample=samples;world_begin(registers,samples);}
done:
    startup_queue_end(saved_sample);
    if(bytes)HeapFree(GetProcessHeap(),0,bytes);canvas_producers_protect();__sync_lock_release(&busy);SetLastError(error);
}
static int install(u32 base){
    u32 a,b,c,d;__asm__ volatile("cpuid":"=a"(a),"=b"(b),"=c"(c),"=d"(d):"a"(1));if(!(d&(1u<<24)))return 0;
    u32 site=base+0x1002a0,old,unused;
    if(scene_trampoline||!readable(site,9)||!equal((void*)site,expected,9))return 0;
    u8* trampoline=VirtualAlloc(0,14,0x3000,0x40);if(!trampoline)return 0;
    copy(trampoline,expected,9);trampoline[9]=0xe9;put(trampoline+10,site+9-(u32)trampoline-14);FlushInstructionCache(GetCurrentProcess(),trampoline,14);
    if(!VirtualProtect((void*)site,9,0x40,&old))return 0;
    image_base=base;scene_trampoline=(u32)trampoline;
    u8 patch[9]={0xe9,0,0,0,0,0x90,0x90,0x90,0x90};put(patch+1,(u32)&scene_enter-site-5);copy((void*)site,patch,9);
    FlushInstructionCache(GetCurrentProcess(),(void*)site,9);VirtualProtect((void*)site,9,old,&unused);return 1;
}
__declspec(dllexport) void SceneAnchor(void){}
#ifdef MNM_SCENE_SELFTEST
__declspec(dllexport) int WIN SceneInstallForTest(u32 base){return install(base);}
#endif
int WIN DllMain(void* instance,u32 reason,void* reserved){
    (void)instance;(void)reserved;if(reason==0){canvas_producers_close();canvas_startup_close();world_lifetime_close();world_stream_close();return 1;}if(reason!=1)return 1;
    u32 n=GetEnvironmentVariableA("MNM_SCENE_DIR",directory,sizeof(directory));if(!n||n>=sizeof(directory))return 1;
    startup_queue_init();kind8_init();
    char number[8];kind8_samples=GetEnvironmentVariableA("MNM_SCENE_KIND8_SAMPLES",number,8)!=0;
    n=GetEnvironmentVariableA("MNM_SCENE_SAMPLES",number,8);
    if(n){if(n>=8)return 1;limit=0;for(u32 i=0;i<n;++i){if(number[i]<'0'||number[i]>'9')return 1;limit=limit*10+number[i]-'0';}if(!limit||limit>MNM_PRODUCER_V3_QUEUES)return 1;
      if(limit>16){char flag[2];if(limit!=MNM_PRODUCER_V3_QUEUES||GetEnvironmentVariableA("MNM_CANVAS_PRODUCERS",flag,2)!=1||flag[0]!='1'||GetEnvironmentVariableA("MNM_WORLD_RASTER_BATCH",flag,2)!=1||flag[0]!='1')return 0;}}
    n=GetEnvironmentVariableA("MNM_SCENE_SKIP",number,8);if(n){if(n>=8)return 1;for(u32 i=0;i<n;++i){if(number[i]<'0'||number[i]>'9')return 1;skip=skip*10+number[i]-'0';}if(skip>3600)return 1;}
    n=GetEnvironmentVariableA("MNM_SCENE_INTERVAL",number,8);if(n){if(n>=8)return 1;interval=0;for(u32 i=0;i<n;++i){if(number[i]<'0'||number[i]>'9')return 1;interval=interval*10+number[i]-'0';}if(!interval||interval>3600)return 1;}
#ifndef MNM_SCENE_SELFTEST
    continuous=world_stream_init();
    if(world_stream_history()&&(skip||interval!=1||!GetEnvironmentVariableA("MNM_SCENE_STARTUP_REPLAY",number,8))){world_stream_fail(110);return 1;}
    if(!canvas_startup_install()||!canvas_producers_install())return 0;
    install((u32)GetModuleHandleA(0));
#endif
    return 1;
}
