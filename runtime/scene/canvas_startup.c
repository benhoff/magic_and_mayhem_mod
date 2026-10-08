/* Opt-in, bounded startup diagnostics. Never a native renderer input. */
#include "../shadow/win32_min.h"
#define HOOKS 12
#define LIMIT 16384
extern void canvas_leave(void);
#define ENTRY(n) extern void canvas_enter_##n(void)
ENTRY(0);ENTRY(1);ENTRY(2);ENTRY(3);ENTRY(4);ENTRY(5);
ENTRY(6);ENTRY(7);ENTRY(8);ENTRY(9);ENTRY(10);ENTRY(11);
u32 canvas_trampolines[HOOKS];
static const struct Hook {u32 address,length,pop;u8 bytes[10];void (*entry)(void);} hooks[HOOKS]={
    {0x58ad90,5,12,{0xa1,0x54,0x81,0x6f,0},canvas_enter_0},
    {0x58b3e0,5,0,{0x53,0x56,0x8b,0xf1,0x57},canvas_enter_1},
    {0x58b660,5,0,{0x83,0xec,8,0x33,0xc0},canvas_enter_2},
    {0x57dc20,10,4,{0x8b,0x44,0x24,4,0x89,0x0d,0x74,0x81,0x65,0},canvas_enter_3},
    {0x57dc60,10,8,{0x8b,0x44,0x24,4,0x89,0x0d,0x6c,0xbb,0x6c,0},canvas_enter_4},
    {0x58bc10,6,4,{0x83,0xec,0x6c,0x56,0x8b,0xf1},canvas_enter_5},
    {0x58bac0,6,8,{0x83,0xec,0x6c,0x56,0x8b,0xf1},canvas_enter_6},
    {0x58bf40,5,12,{0x83,0xec,0x30,0x53,0x55},canvas_enter_7},
    {0x58c140,5,12,{0x83,0xec,0x30,0x53,0x55},canvas_enter_8},
    {0x58d320,9,4,{0x56,0x57,0x8b,0xf1,0xe8,0x37,0xe3,0xff,0xff},canvas_enter_9},
    {0x581ec0,7,16,{0x83,0xec,0x28,0x8b,0x44,0x24,0x30},canvas_enter_10},
    {0x57e1b0,5,12,{0x83,0xec,0x14,0x53,0x55},canvas_enter_11},
};
static HANDLE file;
static u32 enabled,stopped,sequence;
/* Return slots are matched by their exact post-ret stack address, including
   callee argument cleanup. Nested calls do not share a single return address. */
static struct Frame {u32 sp,tag,caller,ecx,edx,args[6];} frames[128];
static struct Lock {u32 object,pixels;} locks[128];
static u32 get(const void* p){const u8* b=p;return b[0]|(u32)b[1]<<8|(u32)b[2]<<16|(u32)b[3]<<24;}
static void put(void* p,u32 v){u8* b=p;b[0]=v;b[1]=v>>8;b[2]=v>>16;b[3]=v>>24;}
static void copy(void* to,const void* from,u32 n){u8* d=to;const u8* s=from;while(n--)*d++=*s++;}
static int readable(u32 p,u32 n){u32 end=p+n;if(!p||end<p)return 0;while(p<end){u32 m[7];if(VirtualQuery((void*)p,m,28)!=28||m[4]!=0x1000||(m[5]&0x101)||!(m[5]&0xee))return 0;u32 next=m[0]+m[3];if(next<=p)return 0;p=next<end?next:end;}return 1;}
void canvas_startup_close(void){if(file){CloseHandle(file);file=0;}}
static void snapshot(u32* out,u32 pixels,u32 width,u32 height,u32 stride){
    out[0]=pixels;out[1]=width;out[2]=height;out[3]=stride;out[4]=0xffffffff;out[5]=0;
    if(!width||width>2048||!height||height>2048||stride<width||stride>4096||!readable(pixels,stride*height*2))return;
    out[4]=0;out[5]=2166136261u;
    for(u32 y=0;y<height;++y)for(u32 x=0;x<width;++x){u8* p=(u8*)pixels+(y*stride+x)*2;
        if(p[0]||p[1])++out[4];out[5]=(out[5]^p[0])*16777619u;out[5]=(out[5]^p[1])*16777619u;}
}
static void record(u32 tag,u32 phase,const struct Frame* f,u32 result){
    if(!file||stopped)return;
    if(sequence>=LIMIT){stopped=1;return;}
    u32 row[40]={0};row[0]=++sequence;row[1]=tag;row[2]=phase;row[3]=f?f->caller:0;row[4]=result;
    if(f){row[5]=f->ecx;row[6]=f->edx;for(u32 i=0;i<6;++i)row[7+i]=f->args[i];}
    for(u32 i=0;i<HOOKS;++i)if(row[3]>=canvas_trampolines[i]&&row[3]<=canvas_trampolines[i]+hooks[i].length){
        row[3]=hooks[i].address+row[3]-canvas_trampolines[i];break;}
    row[13]=get((void*)0x658174);row[14]=get((void*)0x6a2dc8);row[15]=get((void*)0x6def50);
    row[16]=get((void*)0x6e0008);row[17]=get((void*)0x6cbb6c);row[18]=get((void*)0x6a49b8);row[19]=get((void*)0x656618);
    row[20]=get((void*)0x6e1f68);
    u32 object=f&&tag<12&&tag!=3&&tag!=4&&tag!=10&&tag!=11?f->ecx:0;
    if(readable(object,48))for(u32 i=0;i<6;++i)row[21+i]=get((void*)(object+4+i*4));
    row[31]=0xffffffff;
    if(tag==3&&f)snapshot(row+27,f->ecx,f->edx,f->args[0],f->edx);
    else if(tag==12)snapshot(row+27,row[13],row[18],row[19],row[14]);
    else if(tag==2&&phase==1)snapshot(row+27,result,row[24],row[25],row[26]/2);
    else if((tag==5||tag==6||tag==7||tag==8||tag==9)&&object){
        for(u32 i=0;i<128;++i)if(locks[i].object==object){snapshot(row+27,locks[i].pixels,row[24],row[25],row[26]/2);break;}
    }
    u32 wrote;if(!WriteFile(file,row,sizeof(row),&wrote,0)||wrote!=sizeof(row))canvas_startup_close();
}
void canvas_enter_observe(u32* r,u32 tag){
    u32 error=GetLastError();if(!enabled||stopped)goto done;
    u32* stack=r+9;struct Frame f={0};f.sp=(u32)stack+4+hooks[tag].pop;f.tag=tag;f.caller=stack[0];f.ecx=r[6];f.edx=r[5];
    for(u32 i=0;i<6;++i)f.args[i]=stack[i+1];
    record(tag,0,&f,0);
    for(u32 i=0;i<128;++i)if(!frames[i].sp){frames[i]=f;stack[0]=(u32)&canvas_leave;goto done;}
    record(13,0,&f,1);stopped=1;
done:SetLastError(error);
}
void canvas_leave_observe(u32* r){
    u32 error=GetLastError(),sp=(u32)r+40;
    for(u32 i=0;i<128;++i)if(frames[i].sp==sp){
        struct Frame f=frames[i];frames[i].sp=0;r[9]=f.caller;
        if(f.tag==2){for(u32 j=0;j<128;++j)if(locks[j].object==f.ecx||!locks[j].object){locks[j].object=f.ecx;locks[j].pixels=r[7];break;}}
        if(f.tag==0||f.tag==1){for(u32 j=0;j<128;++j)if(locks[j].object==f.ecx)locks[j].object=0;}
        record(f.tag,1,&f,r[7]);SetLastError(error);return;
    }
    /* A return without its frame cannot safely continue. Never guess a target. */
    ExitProcess(93);
}
void canvas_startup_world(u32* r){
    u32 error=GetLastError();if(enabled&&!stopped){struct Frame f={0};f.caller=r[9];f.ecx=r[6];
        record(12,0,&f,0);stopped=1;canvas_startup_close();}SetLastError(error);
}
int canvas_startup_install(void){
    char flag[2],path[260];u32 old,unused,wrote;
    if(!GetEnvironmentVariableA("MNM_CANVAS_STARTUP",flag,2))return 1;
#ifndef MNM_SCENE_SELFTEST
    if((u32)GetModuleHandleA(0)!=0x400000)return 0;
#endif
    /* Validate every entry before any code mutation. Staging pins the full hash. */
    for(u32 i=0;i<HOOKS;++i){const struct Hook* h=hooks+i;if(!readable(h->address,h->length))return 0;
        for(u32 j=0;j<h->length;++j)if(((u8*)h->address)[j]!=h->bytes[j])return 0;
        u8* t=VirtualAlloc(0,h->length+5,0x3000,0x40);if(!t)return 0;copy(t,h->bytes,h->length);
        if(i==9)put(t+5,0x58b660-(u32)t-9); /* relocate the copied direct call */
        t[h->length]=0xe9;put(t+h->length+1,h->address+h->length-(u32)t-h->length-5);
        FlushInstructionCache(GetCurrentProcess(),t,h->length+5);canvas_trampolines[i]=(u32)t;
    }
    u32 n=GetEnvironmentVariableA("MNM_SCENE_DIR",path,220);if(!n||n>=220)return 0;
    const char* suffix="\\canvas-startup.bin";while(*suffix)path[n++]=*suffix++;path[n]=0;
    file=CreateFileA(path,0x40000000,3,0,1,0x80,0);if(file==(HANDLE)-1){file=0;return 0;}
    static u32 header[40];copy(header,"MNMCST01",8);header[2]=1;header[3]=160;header[4]=160;header[5]=LIMIT;header[6]=HOOKS;
    if(!WriteFile(file,header,160,&wrote,0)||wrote!=160)return 0;
    for(u32 i=0;i<HOOKS;++i){const struct Hook* h=hooks+i;
        if(!VirtualProtect((void*)h->address,h->length,0x40,&old))return 0;
        u8 patch[10]={0xe9};put(patch+1,(u32)h->entry-h->address-5);for(u32 j=5;j<h->length;++j)patch[j]=0x90;
        copy((void*)h->address,patch,h->length);FlushInstructionCache(GetCurrentProcess(),(void*)h->address,h->length);
        VirtualProtect((void*)h->address,h->length,old,&unused);
    }
    enabled=1;return 1;
}
