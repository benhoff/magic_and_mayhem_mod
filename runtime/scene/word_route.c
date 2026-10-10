#include "../shadow/win32_min.h"
#include "../../renderer/sprites/word_raster.h"
#include "word_workspace.h"

/* Single engine-thread backend adapter. Native pixels remain in the engine's
 * borrowed canvas, preserving downstream readback and auxiliary drawing. */
extern void word_enter_scalar(void);
extern void word_enter_forward(void);
u32 word_original;
static u32 scalar_trampoline,forward_trampoline;
static u32 image_base,mode,installed,stopped;
static volatile u32 busy;
static char directory[220];
static u32 stats[16]={0x574d4e4d,0x31304452,1,64}; /* MNMWRD01 */
static HANDLE log_file=(HANDLE)-1;
static u32 get(const void* p){const u8* b=p;return b[0]|(u32)b[1]<<8|(u32)b[2]<<16|(u32)b[3]<<24;}
static void put(void* p,u32 n){u8* b=p;b[0]=n;b[1]=n>>8;b[2]=n>>16;b[3]=n>>24;}
static void copy(void* a,const void* b,u32 n){u8* d=a;const u8* s=b;while(n--)*d++=*s++;}
static int equal(const void* a,const void* b,u32 n){const u8* x=a;const u8* y=b;while(n--)if(*x++!=*y++)return 0;return 1;}
static int extent(u32 at,u32 size,int writable){
    u32 end=at+size;if(!at||end<at)return 0;
    while(at<end){u32 m[7];if(VirtualQuery((void*)at,m,28)!=28||m[4]!=0x1000||
        (m[5]&0x101)||!(m[5]&(writable?0xcc:0xee)))return 0;
        u32 next=m[0]+m[3];if(next<=at)return 0;at=next<end?next:end;}return 1;
}
static int write_all(HANDLE file,const void* p,u32 n){
    const u8* b=p;while(n){u32 wrote=0;if(!WriteFile(file,b,n,&wrote,0)||!wrote||wrote>n)return 0;b+=wrote;n-=wrote;}return 1;
}
static void path(char* out,const char* name){u32 n=0;while(directory[n]){out[n]=directory[n];++n;}out[n++]='\\';while(*name)out[n++]=*name++;out[n]=0;}
static void report(void){stats[4]=mode;stats[5]=installed;stats[15]=stopped;if(log_file!=(HANDLE)-1&&!write_all(log_file,stats,64))++stats[14];}
/* Scratch indices span the original 0x5f1e50..0x5f1e8c workspace. Mirror only
 * bytes written by the admitted unclipped backend; retain all other words. */
static void sample(u32 sequence,u32 backend,u32 frame,u32 length,const MnmWordCanvas* c,
                   i32 ax,i32 ay,const void* before,const u32* old,const u32* after){
    char name[]="word-0000.bin",filename[260];
    for(u32 i=0,d=1000;i<4;++i,d/=10)name[5+i]=(char)('0'+sequence/d%10);
    path(filename,name);HANDLE f=CreateFileA(filename,0x40000000,0,0,1,0x80,0);
    u32 h[20]={0x574d4e4d,0x31304352,1,80+128+length+(u32)c->bytes*2,mode,sequence,
        frame,(u32)c->pixels,length,(u32)c->bytes,c->width,c->height,c->stride_words,
        (u32)ax,(u32)ay,backend,0,0,0,0}; /* MNMWRC01 */
    h[16]=get((void*)(image_base+0x2e0008));h[17]=get((void*)(image_base+0x2cbb6c));
    int ok=f!=(HANDLE)-1&&write_all(f,h,80)&&write_all(f,old,64)&&write_all(f,after,64)&&
        write_all(f,(void*)frame,length)&&write_all(f,before,(u32)c->bytes)&&write_all(f,c->pixels,(u32)c->bytes);
    if(f!=(HANDLE)-1&&!CloseHandle(f))ok=0;
    if(!ok)++stats[14];else ++stats[13];
}
int word_route(u32* registers){
    u32 error=GetLastError(),handled=0;
    u32 backend=get((void*)(registers[3]+4));
    ++stats[6];word_original=backend==0x197086?scalar_trampoline:forward_trampoline;
    if(!__sync_bool_compare_and_swap(&busy,0,1)){++stats[9];++stats[12];SetLastError(error);return 0;}
    u8* allocation=0;u8* before=0;
    if(stopped)goto fallback;
    /* pushal's saved ESP follows the adapter's backend tag and pushfl. */
    u32* arguments=(u32*)(registers[3]+12);u32 frame=arguments[0];i32 ax=arguments[1],ay=arguments[2];
    if(!extent(frame,48,0))goto fallback;
    u32 length=get((void*)frame);if(length<48||length>4*1024*1024||!extent(frame,length,0))goto fallback;
    MnmWordCanvas canvas={(u8*)get((void*)(image_base+0x258174)),0,
        get((void*)(image_base+0x2a49b8)),get((void*)(image_base+0x256618)),get((void*)(image_base+0x2a2dc8))};
    if(!canvas.width||!canvas.height||canvas.width>2048||canvas.height>2048||
        canvas.stride_words<canvas.width||canvas.stride_words>4096)goto fallback;
    canvas.bytes=canvas.stride_words*canvas.height*2;
    if(!extent((u32)canvas.pixels,canvas.bytes,1))goto fallback;
    MnmWordDraw draw;
    if(mnm_word_sprite_admit((void*)frame,length,&canvas,ax,ay,&draw)!=MNM_WORD_OK||
        draw.left<(i32)get((void*)(image_base+0x2e0008))||draw.top<(i32)get((void*)(image_base+0x2cbb6c)))goto fallback;
    u32 old[16],predicted[16];copy(old,(void*)(image_base+0x1f1e50),64);copy(predicted,old,64);
    mnm_word_unclipped_workspace(predicted,&draw,frame,(u32)canvas.pixels,canvas.stride_words,backend);
    u32 sequence=stats[7]+1;
    if(mode==1||sequence<=8){
        allocation=HeapAlloc(GetProcessHeap(),0,canvas.bytes+4);if(!allocation)goto fallback;
        before=allocation+((u32)canvas.pixels&3);copy(before,canvas.pixels,canvas.bytes);
    }
    ++stats[7];
    if(mode==1){
        MnmWordCanvas trial=canvas;trial.pixels=before;
        if(mnm_word_sprite_draw((void*)frame,length,&trial,ax,ay,&draw)!=MNM_WORD_OK)goto fallback;
        typedef u32 (*Original)(void*,i32,i32);
        u32 returned=((Original)word_original)((void*)frame,ax,ay);++stats[9];++stats[10];
        if(returned||!equal(before,canvas.pixels,canvas.bytes)||!equal(predicted,(void*)(image_base+0x1f1e50),64)){
            ++stats[11];stopped=1;
        }
    }else{
        if(mnm_word_sprite_draw((void*)frame,length,&canvas,ax,ay,&draw)!=MNM_WORD_OK)goto fallback;
        copy((void*)(image_base+0x1f1e50),predicted,64);++stats[8];
        if(sequence<=8)sample(sequence,backend,frame,length,&canvas,ax,ay,before,old,predicted);
    }
    /* Original backend mutates its cdecl argument slots before returning zero. */
    arguments[1]=draw.left;arguments[2]=draw.top;handled=1;
    if(sequence<=8||!(sequence%64)||stopped)report();
    goto done;
fallback:
    ++stats[9];++stats[12];if(stats[6]<=8||!(stats[6]%64))report();
done:
    if(allocation)HeapFree(GetProcessHeap(),0,allocation);
    __sync_lock_release(&busy);SetLastError(error);return handled;
}
static int install(u32 base){
    const u8 expected[6]={0x55,0x8b,0xec,0x56,0x57,0x53};
    u32 a,b,c,d;__asm__ volatile("cpuid":"=a"(a),"=b"(b),"=c"(c),"=d"(d):"a"(1));
    u32 sites[2]={base+0x197086,base+0x196cb8},old[2],unused;
    if(base!=0x400000||!(d&(1u<<24)))return 0;
    for(u32 i=0;i<2;++i)if(!extent(sites[i],6,0)||!equal((void*)sites[i],expected,6))return 0;
    u8* trampolines=VirtualAlloc(0,22,0x3000,0x40);if(!trampolines)return 0;
    for(u32 i=0;i<2;++i){u8* t=trampolines+i*11;copy(t,expected,6);t[6]=0xe9;put(t+7,sites[i]+6-(u32)t-11);}
    FlushInstructionCache(GetCurrentProcess(),trampolines,22);
    if(!VirtualProtect((void*)sites[0],6,0x40,&old[0]))return 0;
    if(!VirtualProtect((void*)sites[1],6,0x40,&old[1])){VirtualProtect((void*)sites[0],6,old[0],&unused);return 0;}
    image_base=base;scalar_trampoline=(u32)trampolines;forward_trampoline=(u32)trampolines+11;
    u32 entries[2]={(u32)&word_enter_scalar,(u32)&word_enter_forward};
    for(u32 i=0;i<2;++i){u8 patch[6]={0xe9,0,0,0,0,0x90};put(patch+1,entries[i]-sites[i]-5);copy((void*)sites[i],patch,6);
        FlushInstructionCache(GetCurrentProcess(),(void*)sites[i],6);VirtualProtect((void*)sites[i],6,old[i],&unused);}
    return 1;
}
__declspec(dllexport) void WordAnchor(void){}
int WIN DllMain(void* instance,u32 reason,void* reserved){
    (void)instance;(void)reserved;
    if(reason==0){if(log_file!=(HANDLE)-1){report();CloseHandle(log_file);}return 1;}
    if(reason!=1)return 1;
    char selection[16];u32 n=GetEnvironmentVariableA("MNM_WORD_SPRITES",selection,sizeof(selection));
    if(n==6&&equal(selection,"shadow",6))mode=1;
    else if(n==8&&equal(selection,"takeover",8))mode=2;else return 1;
    n=GetEnvironmentVariableA("MNM_WORD_DIRECTORY",directory,sizeof(directory));if(!n||n>=sizeof(directory))return 1;
    char filename[260];path(filename,"stats.bin");log_file=CreateFileA(filename,0x40000000,0,0,1,0x80,0);
    if(log_file==(HANDLE)-1)return 1;
    installed=install((u32)GetModuleHandleA(0));report();return 1;
}
