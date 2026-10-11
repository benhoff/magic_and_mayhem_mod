/* Synthetic PE32 memory admission; never maps or executes game media. */
#include "../runtime/shadow/win32_min.h"
API int WIN CanvasWritersInstallForTest(void);
API int WIN CanvasWritersRouteForTest(u32*);
static u32 object[12],regs[13];
static u16 *pixels;
// Own the preferred test address range as a synthetic PE section. Wine may
// otherwise put its process heap there before a requested VirtualAlloc.
__declspec(allocate(".space")) u8 synthetic_image_space[0x320000];
static const u8 entry[]={0x51,0x55,0x56,0x8b,0x35,0x88,0x1f,0x6e,0};
static const u8 loop[]={0x53,0x8b,0x47,0x10,0x33,0xdb,0x99,0x2b,0xc2,0xd1,0xf8,0x85,0xc0,0x7e,0x19,
  0x8b,0x01,0x83,0xc1,0x04,0xd1,0xe8,0x23,0xc6,0x43,0x89,0x41,0xfc,0x8b,0x47,0x10,0x99,
  0x2b,0xc2,0xd1,0xf8,0x3b,0xd8,0x7c,0xe7,0x8b,0x47,0x14,0x45,0x3b,0xe8,0x7c,0xd1,0x8b,0x44,0x24,0x10,0x5b};
static const u8 tail[]={0x8b,0x7f,0x04,0x50,0x57,0x8b,0x0f,0xff,0x91,0x80,0,0,0,0x5f,0x5e,0x5d,0x59,0xc3};
static void copy(u8 *to,const u8 *from,u32 n) {for(u32 i=0;i<n;++i)to[i]=from[i];}
static void reset(void) {
    for(u32 i=0;i<13;++i)regs[i]=0;
    object[4]=3;object[5]=2;object[6]=8;
    regs[0]=(u32)object;regs[1]=0xfbef7bef;regs[5]=2;regs[6]=regs[7]=regs[12]=(u32)pixels;regs[8]=0x602;
    for(u32 i=0;i<8;++i)pixels[i]=(u16)(i*1273+113);
    *(u32*)0x6e1f88=0;
}
static int unchanged(void) {for(u32 i=0;i<8;++i)if(pixels[i]!=(u16)(i*1273+113))return 0;return 1;}
void start(void) {
    u32 old_code,old_globals;
    if(!VirtualProtect((void*)0x580000,0x10000,0x40,&old_code) ||
       !VirtualProtect((void*)0x6e0000,0x10000,4,&old_globals))ExitProcess(1);
    pixels=VirtualAlloc(0,8192,0x3000,4);if(!pixels)ExitProcess(2);
    copy((u8*)0x58ed80,entry,sizeof(entry));copy((u8*)0x58edb0,loop,sizeof(loop));copy((u8*)0x58ede5,tail,sizeof(tail));
    // Unsupported mode or disabled default may not patch an instruction.
    u32 before=*(u32*)0x58edb0;
    char mode[16];u32 n=GetEnvironmentVariableA("MNM_FADE_TAKEOVER",mode,sizeof(mode));
    char bad[8];u32 bad_anchor=GetEnvironmentVariableA("MNM_WRITER_TEST_BAD_ANCHOR",bad,sizeof(bad));
    if(bad_anchor)*(u8*)0x58ed80=0x90;
    int installed=CanvasWritersInstallForTest();
    if((n && (n!=1 || mode[0]!='1')) || bad_anchor){
        if(installed || *(u32*)0x58edb0!=before)ExitProcess(3);ExitProcess(0);
    }
    if(!installed)ExitProcess(3);
    if(!n){if(*(u32*)0x58edb0!=before)ExitProcess(4);ExitProcess(0);}
    if(*(u8*)0x58edb0!=0xe9)ExitProcess(5);
    for(u32 test=0;test<13;++test){
        reset();u32 old=0;
        switch(test){
          case 0:object[4]=0;break;case 1:object[4]=2049;break;
          case 2:object[5]=0;break;case 3:object[6]=7;break;
          case 4:object[6]=2;break;case 5:object[6]=8194;break;
          case 6:regs[12]+=2;break;case 7:regs[6]+=2;break;
          case 8:regs[1]=0xfdef7def;break;case 9:regs[2]=1;break;
          case 10:regs[5]=3;break;
          case 11:if(!VirtualProtect(pixels,4096,2,&old))ExitProcess(6);break;
          case 12:if(!VirtualProtect(pixels,4096,0x104,&old))ExitProcess(6);break;
        }
        SetLastError(0x76543210);int result=CanvasWritersRouteForTest(regs);u32 error=GetLastError();
        if(test>=11){u32 ignored;if(!VirtualProtect(pixels,4096,old,&ignored))ExitProcess(7);}
        if(result || error!=0x76543210 || !unchanged())ExitProcess(10+test);
    }
    reset();SetLastError(0x76543210);
    if(!CanvasWritersRouteForTest(regs) || GetLastError()!=0x76543210)ExitProcess(30);
    for(u32 i=0;i<8;++i){u16 expected=(u16)(i*1273+113);if(i<4)expected=(expected>>1)&0x7bef;if(pixels[i]!=expected)ExitProcess(31);}
    if(regs[2]!=2 || regs[5] || regs[6]!=(u32)pixels+8 || regs[8]!=0x646)ExitProcess(32);
    // A lease crossing a readable first page and inaccessible second page must
    // be refused completely, even though its processed prefix is accessible.
    reset();object[4]=1;object[5]=2;object[6]=8192;u32 old;
    if(!VirtualProtect((u8*)pixels+4096,4096,1,&old))ExitProcess(33);
    SetLastError(0x76543210);if(CanvasWritersRouteForTest(regs) || GetLastError()!=0x76543210 || !unchanged())ExitProcess(34);
    u32 ignored;if(!VirtualProtect((u8*)pixels+4096,4096,old,&ignored))ExitProcess(35);
    for(u32 i=1;i<128;++i){reset();if(!CanvasWritersRouteForTest(regs))ExitProcess(36);}
    reset();SetLastError(0x76543210);
    if(CanvasWritersRouteForTest(regs) || GetLastError()!=0x76543210 || !unchanged())ExitProcess(37);
    ExitProcess(0);
}
