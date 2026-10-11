/* Opt-in fixed-build fade splice. Original Lock/Unlock and other writers stay. */
#include "../shadow/win32_min.h"
#include "fade_route.h"
API u32 WIN GetCurrentThreadId(void);
extern void mnm_fade_enter(void),mnm_fade_finish_entry(void);
uintptr_t mnm_fade_tail;
static HANDLE capture;
static char mode[16],path[1024];
static struct MnmFadePlane current;
static u32 active,records,refusals,captured_bytes=8;
static volatile u32 owner;
static int span(u32 address,u32 bytes,int write) {
    if(!address || !bytes || address>0xffffffffu-bytes)return 0;
    u32 end=address+bytes;
    while(address<end){
        u32 m[7]={0};
        if(VirtualQuery((void*)address,m,28)!=28 || m[4]!=0x1000 || (m[5]&0x101) ||
           !(m[5]&0xee) || (write && !(m[5]&0xcc)))return 0;
        if(m[0]>address || !m[3] || m[0]>0xffffffffu-m[3] || m[0]+m[3]<=address)return 0;
        address=m[0]+m[3];
    }
    return 1;
}
static void output(const void *p,u32 bytes) {
    u32 written=0;
    if(!WriteFile(capture,p,bytes,&written,0) || written!=bytes)ExitProcess(121);
}
static int resolve(const u32 *r,struct MnmFadePlane *p) {
    u32 error=GetLastError();int ok=0;
    u32 thread=GetCurrentThreadId();
    if(!owner)__sync_bool_compare_and_swap(&owner,0,thread);
    if(owner!=thread)goto done;
    if(active || records>=128 || !span(r[0],28,0))goto done;
    const u32 *object=(const u32*)r[0];
    p->pixels=(u16*)r[7];p->width=object[4];p->height=object[5];p->stride=object[6]/2;p->format=*(const u32*)0x6e1f88;
    if(!p->width || p->width>2048 || !p->height || p->height>2048 || (object[6]&1) ||
       p->stride<p->width || p->stride>4096 || r[2] || r[5]!=p->height || r[6]!=r[7] ||
       r[12]!=r[7] || r[1]!=(p->format?0xfdef7defu:0xfbef7befu) ||
       !span(r[7],p->stride*p->height*2,1) ||
       captured_bytes>256u*1024u*1024u-(48+p->stride*p->height*4))goto done;
    captured_bytes+=48+p->stride*p->height*4;
    current=*p;active=1;ok=1;
    // Closed, bounded, physical-plane diagnostic: never a native input oracle.
    u32 header[8]={++records,p->width,p->height,p->stride,p->format,
                   p->stride*p->height,(p->width/2)*2*p->height,0};
    output(header,sizeof(header));output(p->pixels,header[5]*2);
done:
    if(!ok)++refusals;
    SetLastError(error);return ok;
}
void mnm_fade_finish(void) {
    u32 error=GetLastError();
    if(!active)ExitProcess(122);
    output(current.pixels,current.stride*current.height*2);active=0;
    // Count records includes refusal diagnostics before this successful route.
    u32 footer[4]={mnm_fade_native,mnm_fade_forwarded,refusals,0x31444e45};output(footer,sizeof(footer));
    SetLastError(error);
}
static void jump(u8 *at,void *target,u32 length) {
    at[0]=0xe9;*(u32*)(at+1)=(u32)target-(u32)at-5;
    for(u32 i=5;i<length;++i)at[i]=0x90;
}
static int equal(const u8 *a,const u8 *b,u32 bytes) {
    for(u32 i=0;i<bytes;++i)if(a[i]!=b[i])return 0;return 1;
}
__declspec(dllexport) void CanvasWritersAnchor(void) {}
static int install(void) {
    u32 n=GetEnvironmentVariableA("MNM_FADE_TAKEOVER",mode,sizeof(mode));
    if(!n)return 1;
    if(n!=1 || mode[0]!='1')return 0;
    n=GetEnvironmentVariableA("MNM_FADE_CAPTURE",path,sizeof(path));if(!n || n>=sizeof(path))return 0;
    if((u32)GetModuleHandleA(0)!=0x400000)return 0;
    const u8 entry[]={0x51,0x55,0x56,0x8b,0x35,0x88,0x1f,0x6e,0};
    const u8 loop[]={0x53,0x8b,0x47,0x10,0x33,0xdb,0x99,0x2b,0xc2,0xd1,0xf8,0x85,0xc0,0x7e,0x19,
      0x8b,0x01,0x83,0xc1,0x04,0xd1,0xe8,0x23,0xc6,0x43,0x89,0x41,0xfc,0x8b,0x47,0x10,0x99,
      0x2b,0xc2,0xd1,0xf8,0x3b,0xd8,0x7c,0xe7,0x8b,0x47,0x14,0x45,0x3b,0xe8,0x7c,0xd1,0x8b,0x44,0x24,0x10,0x5b};
    const u8 tail[]={0x8b,0x7f,0x04,0x50,0x57,0x8b,0x0f,0xff,0x91,0x80,0,0,0,0x5f,0x5e,0x5d,0x59,0xc3};
    if(!equal((const u8*)0x58ed80,entry,sizeof(entry)) || !equal((const u8*)0x58edb0,loop,sizeof(loop)) ||
       !equal((const u8*)0x58ede5,tail,sizeof(tail)))return 0;
    u8 *trampoline=VirtualAlloc(0,4096,0x3000,0x40);if(!trampoline)return 0;
    for(u32 i=0;i<sizeof(loop);++i)trampoline[i]=loop[i];jump(trampoline+sizeof(loop),(void*)0x58ede5,5);
    u32 old=0;if(!VirtualProtect(trampoline,4096,0x20,&old))return 0;
    capture=CreateFileA(path,0x40000000,1,0,1,0x80,0);if(capture==(HANDLE)-1)return 0;
    output("MNMFDLV1",8);
    mnm_fade_resolve=resolve;mnm_fade_original=(uintptr_t)trampoline;
    mnm_fade_resume=(uintptr_t)mnm_fade_finish_entry;mnm_fade_tail=0x58ede5;
    if(!VirtualProtect((void*)0x58edb0,6,0x40,&old))return 0;
    jump((u8*)0x58edb0,(void*)mnm_fade_enter,6);
    u32 ignored=0;
    if(!VirtualProtect((void*)0x58edb0,6,old,&ignored) ||
       !FlushInstructionCache(GetCurrentProcess(),(void*)0x58edb0,6))ExitProcess(123);
    return 1;
}
#ifdef MNM_CANVAS_WRITER_SELFTEST
__declspec(dllexport) int WIN CanvasWritersInstallForTest(void) { return install(); }
__declspec(dllexport) int WIN CanvasWritersRouteForTest(u32 *r) {
    int result=mnm_fade_route(r);if(result)mnm_fade_finish();return result;
}
#endif
int WIN DllMain(HANDLE module,u32 reason,void *reserved) {
    (void)module;(void)reserved;
    if(reason!=1)return 1;
#ifdef MNM_CANVAS_WRITER_SELFTEST
    return 1;
#else
    u32 error=GetLastError();int result=install();SetLastError(error);return result;
#endif
}
