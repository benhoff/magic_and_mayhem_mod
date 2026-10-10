#include "../shadow/win32_min.h"
#include "../../renderer/sprites/word_raster.h"
#include "word_workspace.h"
#include "../../renderer/sprites/word_clipped.h"
#include "../../reconstruction/rendering/word_backend_state.h"

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
/* Mode3 uses MNMWCL01/MNMWAD01. Mode4 uses MNMWTK01/MNMWAT01:
 * accepted counts describe native bypasses, never original comparisons. */
static u32 clip_stats[20]={0x574d4e4d,0x31304c43,1,80}; /* MNMWCL01 */
static HANDLE clip_log=(HANDLE)-1;
/* MNMWAD01: bounded admission reasons/backend counts, separate from v1 logs. */
static u32 admission_stats[32]={0x574d4e4d,0x31304441,1,128};
static HANDLE admission_log=(HANDLE)-1;
static u32 sampled_hash[8],sampled_backend[8];
static u32 frame_hash(const u8* frame,u32 length){
    u32 hash=2166136261u;while(length--){hash^=*frame++;hash*=16777619u;}return hash;
}
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
static void report(void){
    stats[4]=mode;stats[5]=installed;stats[15]=stopped;
    if(log_file!=(HANDLE)-1&&!write_all(log_file,stats,64))++stats[14];
    if(mode>=3){clip_stats[4]=installed;clip_stats[15]=stopped;
        if(clip_log!=(HANDLE)-1&&!write_all(clip_log,clip_stats,80)){++stats[14];++clip_stats[13];}
        if(admission_log!=(HANDLE)-1&&!write_all(admission_log,admission_stats,128)){++stats[14];++clip_stats[13];}}
}
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
int word_route(u32* registers,const u8* caller_fx){
    u32 error=GetLastError(),handled=0;
    u32 backend=get((void*)(registers[3]+4));
    ++stats[6];word_original=backend==0x197086?scalar_trampoline:forward_trampoline;
    if(mode>=3){++clip_stats[5];++admission_stats[4];}
    if(!__sync_bool_compare_and_swap(&busy,0,1)){++stats[9];++stats[12];
        if(mode>=3){++clip_stats[10];++clip_stats[11];++admission_stats[13];}SetLastError(error);return 0;}
    u8* allocation=0;u8* before=0;u32 refusal=5;
    if(stopped){refusal=14;goto fallback;}
    /* pushal's saved ESP follows the adapter's backend tag and pushfl. */
    u32* arguments=(u32*)(registers[3]+12);u32 frame=arguments[0];i32 ax=arguments[1],ay=arguments[2];
    u32 minimum=mode>=3?40:48;
    if(!extent(frame,minimum,0))goto fallback;
    u32 length=get((void*)frame);if(length<minimum||length>4*1024*1024||!extent(frame,length,0))goto fallback;
    if(mode>=3&&(!get((void*)(frame+4))||!get((void*)(frame+8)))){++clip_stats[16];refusal=6;goto fallback;}
    MnmWordCanvas canvas={(u8*)get((void*)(image_base+0x258174)),0,
        get((void*)(image_base+0x2a49b8)),get((void*)(image_base+0x256618)),get((void*)(image_base+0x2a2dc8))};
    refusal=7;
    if(!canvas.width||!canvas.height||canvas.width>2048||canvas.height>2048||
        canvas.stride_words<canvas.width||canvas.stride_words>4096)goto fallback;
    canvas.bytes=canvas.stride_words*canvas.height*2;
    refusal=8;
    if(!extent((u32)canvas.pixels,canvas.bytes,1))goto fallback;
    if(mode>=3){
        /* Native policy: original FILD needs its next physical stack slot free.
         * Forward unmasked exception controls and occupied push slots unchanged. */
        if(mode==4){
            refusal=22;
            if(!caller_fx||(caller_fx[0]&63)!=63||(caller_fx[1]&3)==1)goto fallback;
            refusal=23;
            u32 top=(caller_fx[3]>>3)&7;
            if(caller_fx[4]&(1u<<((top+7)&7)))goto fallback;
        }
        MnmWordClip clip={(i32)get((void*)(image_base+0x2e0008)),
            (i32)get((void*)(image_base+0x2cbb6c)),(i32)canvas.width,(i32)canvas.height};
        MnmWordClippedDraw clipped;
        refusal=9;
        if(mnm_word_sprite_clip_admit((void*)frame,length,&canvas,&clip,ax,ay,&clipped))goto fallback;
        u32 old_clip[16];copy(old_clip,(void*)(image_base+0x1f1e50),64);
        MnmWordBackendInput input={(void*)frame,length,frame,(u32)canvas.pixels,canvas.width,
            canvas.height,canvas.stride_words,backend,clip.left,clip.top,ax,ay};
        MnmWordBackendState state;
        refusal=10;
        if(mnm_word_backend_state(&input,old_clip,&state))goto fallback;
        refusal=11;
        allocation=HeapAlloc(GetProcessHeap(),0,canvas.bytes*2+68);if(!allocation)goto fallback;
        before=allocation;copy(before,canvas.pixels,canvas.bytes);
        u8* guarded=allocation+canvas.bytes;
        for(u32 i=0;i<canvas.bytes+68;++i)guarded[i]=0xa5;
        u32 offset=32+((u32)canvas.pixels&3);
        MnmWordCanvas trial=canvas;trial.pixels=guarded+offset;copy(trial.pixels,before,canvas.bytes);
        refusal=12;
        if(mnm_word_sprite_clip_draw((void*)frame,length,&trial,&clip,ax,ay,&clipped))goto fallback;
        int edge=clipped.left<clip.left||clipped.top<clip.top||
            (int64_t)clipped.left+clipped.width>=clip.right||(int64_t)clipped.top+clipped.height>=clip.bottom;
        int hidden=clipped.source_left==clipped.source_right||clipped.source_top==clipped.source_bottom;
        typedef u32 (*OriginalClip)(void*,i32,i32);
        int guard_ok=1;
        for(u32 i=0;i<canvas.bytes+68;++i)
            if((i<offset||i>=offset+canvas.bytes)&&guarded[i]!=0xa5)guard_ok=0;
        if(mode==4&&!guard_ok)goto fallback;
        u32 returned=0;
        if(mode==4){
            /* Commit only a fully preflighted and guarded result. Original callers
             * retain their canvas identity and subsequent auxiliary-plane work. */
            copy(canvas.pixels,trial.pixels,canvas.bytes);
            copy((void*)(image_base+0x1f1e50),state.words,64);
            ++stats[8];
            u32 precision=(caller_fx[1]>>0)&3;
            if(precision==0)++admission_stats[24];
            else if(precision==2)++admission_stats[25];
            else if(precision==3)++admission_stats[26];
        }else{
            returned=((OriginalClip)word_original)((void*)frame,ax,ay);
            ++stats[9];++stats[10];++clip_stats[10];++clip_stats[18];
        }
        ++stats[7];++clip_stats[6];
        int scalar=backend==0x197086;
        ++admission_stats[scalar?16:17];
        if(edge)++admission_stats[scalar?18:19];
        if(get((void*)(frame+32))||get((void*)(frame+36)))++admission_stats[15];
        if(edge&&(get((void*)(frame+32))||get((void*)(frame+36))))++admission_stats[20];
        if(edge)++clip_stats[7];else ++clip_stats[8];if(hidden)++clip_stats[9];
        if(returned||!guard_ok||!equal(trial.pixels,canvas.pixels,canvas.bytes)||
            !equal(state.words,(void*)(image_base+0x1f1e50),64)){
            ++stats[11];++clip_stats[12];stopped=1;
        }
        /* Retain two distinct clipped and six distinct interior inputs, rather than repeated
         * small menu/banner requests filling all eight diagnostic slots. */
        u32 hash=0,duplicate=0;
        int candidate=(get((void*)(frame+32))||get((void*)(frame+36)))&&clip_stats[14]<8&&
            (edge?clip_stats[17]<2:clip_stats[14]-clip_stats[17]<6);
        if(candidate){
            hash=frame_hash((void*)frame,length);
            for(u32 i=0;i<clip_stats[14];++i)
                if(sampled_hash[i]==hash&&sampled_backend[i]==backend)duplicate=1;
            if(duplicate)++admission_stats[21];
        }
        if(candidate&&!duplicate){
            u32 previous=stats[13];
            sample(clip_stats[14]+1,backend,frame,length,&canvas,ax,ay,before,old_clip,(void*)(image_base+0x1f1e50));
            if(stats[13]>previous){sampled_hash[clip_stats[14]]=hash;sampled_backend[clip_stats[14]]=backend;++clip_stats[14];if(edge)++clip_stats[17];}
            else{++clip_stats[13];stopped=1;}
        }
        arguments[1]=(u32)state.argument_x;arguments[2]=(u32)state.argument_y;handled=1;
        if(clip_stats[6]<=8||!(clip_stats[6]%64)||stopped)report();
        goto done;
    }
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
    if(mode>=3){++clip_stats[10];++clip_stats[11];++admission_stats[refusal];}
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
    if(reason==0){if(log_file!=(HANDLE)-1){report();CloseHandle(log_file);}
        if(clip_log!=(HANDLE)-1)CloseHandle(clip_log);
        if(admission_log!=(HANDLE)-1)CloseHandle(admission_log);
        return 1;}
    if(reason!=1)return 1;
    char selection[16];u32 n=GetEnvironmentVariableA("MNM_WORD_SPRITES",selection,sizeof(selection));
    if(n==6&&equal(selection,"shadow",6))mode=1;
    else if(n==8&&equal(selection,"takeover",8))mode=2;
    else if(n==11&&equal(selection,"clip-shadow",11))mode=3;
    else if(n==13&&equal(selection,"clip-takeover",13))mode=4;else return 1;
    if(mode==4){clip_stats[1]=0x31304b54;admission_stats[1]=0x31305441;}
    n=GetEnvironmentVariableA("MNM_WORD_DIRECTORY",directory,sizeof(directory));if(!n||n>=sizeof(directory))return 1;
    char filename[260];path(filename,"stats.bin");log_file=CreateFileA(filename,0x40000000,0,0,1,0x80,0);
    if(log_file==(HANDLE)-1)return 1;
    if(mode>=3){path(filename,"clip-stats.bin");clip_log=CreateFileA(filename,0x40000000,0,0,1,0x80,0);
        if(clip_log==(HANDLE)-1){++stats[14];report();return 1;}
        path(filename,"admission-stats.bin");admission_log=CreateFileA(filename,0x40000000,0,0,1,0x80,0);
        if(admission_log==(HANDLE)-1){++stats[14];report();return 1;}}
    installed=install((u32)GetModuleHandleA(0));report();return 1;
}
