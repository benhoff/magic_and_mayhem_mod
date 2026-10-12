#include "../shadow/win32_min.h"
API u32 WIN GetTickCount(void);
API void WIN Sleep(u32);
API void* WIN LoadLibraryA(const char*);
API void* WIN GetProcAddress(void*,const char*);
__declspec(dllimport) u32 WIN AudioMusicForTest(u32,u32,u32,u32*);
__declspec(dllimport) u32 WIN AudioMusicNotifyForTest(u32*,u32*);
__declspec(dllimport) u32 WIN AudioMusicInstallForTest(u32);
static void expect(int valid,u32 code){if(!valid)ExitProcess(code);}
static u32 call(u32 device,u32 message,u32 flags,u32* parameters){SetLastError(0x77);u32 result=AudioMusicForTest(device,message,flags,parameters);expect(GetLastError()==0x77,201);return result;}
static void install_fixture(void){
    u8* image=VirtualAlloc(0,0x200000,0x3000,0x40);expect(image!=0,223);
    void* library=LoadLibraryA("winmm.dll");expect(library!=0,224);
    void* original=GetProcAddress(library,"mciSendCommandA");expect(original!=0,225);
    void** iat=(void**)(image+0x1c52dc);*iat=original;
    u8* mixer=image+0x8157b;static const u8 mixerBytes[7]={0xff,0x15,0xf8,0x52,0x5c,0,0xc3};
    u8* volume=image+0x82190;static const u8 volumeBytes[13]={0x53,0x8b,0xd9,0x55,0x33,0xed,0x5d,0x5b,0x33,0xc0,0xc2,4,0};
    for(u32 i=0;i<7;++i)mixer[i]=mixerBytes[i];for(u32 i=0;i<13;++i)volume[i]=volumeBytes[i];
    *iat=0;expect(!AudioMusicInstallForTest((u32)image)&&mixer[0]==0xff&&volume[0]==0x53,226);*iat=original;
    mixer[5]=1;expect(!AudioMusicInstallForTest((u32)image)&&*iat==original&&volume[0]==0x53,227);mixer[5]=0;
    volume[5]=0;expect(!AudioMusicInstallForTest((u32)image)&&*iat==original&&mixer[0]==0xff,228);volume[5]=0xed;
    expect(AudioMusicInstallForTest((u32)image)&&*iat!=original&&mixer[0]==0xe8&&volume[0]==0xe9,229);
    expect(((u32 (WIN *)(void))mixer)()==0,230);
    static u32 object[0x330/4];object[0x32c/4]=1;
    expect(((i32 (__thiscall *)(void*,i32))volume)(object,7)==0,231);
}
void start(void){
    install_fixture();
    u32 open[5]={0x11223344,0,(u32)"cdaudio",0,0};
    expect(!call(0,0x803,0x2102,open)&&open[1]==0x7321,202);u32 device=open[1];
    u32 format[3]={0,10,0};expect(!call(device,0x80d,0x402,format),203);
    u32 status[4]={0,0,3,0};expect(!call(device,0x814,0x102,status)&&status[1]==3,204);
    status[2]=0x4001;status[3]=1;expect(!call(device,0x814,0x112,status)&&status[1]==1089,205);
    status[3]=2;expect(!call(device,0x814,0x112,status)&&status[1]==1088,206);
    status[2]=1;expect(!call(device,0x814,0x112,status)&&status[1]==0x100,207); /* one second MSF */
    u32 play[3]={0x11223344,2,3};expect(!call(device,0x806,13,play),208);
    u32 started=GetTickCount(),kind=0,notifiedDevice=0,notifications=0;
    while(!(notifications=AudioMusicNotifyForTest(&kind,&notifiedDevice))){expect(GetTickCount()-started<5000,209);Sleep(5);}
    expect(notifications==1&&kind==1&&notifiedDevice==device,210);
    expect(!call(device,0x806,13,play),211);expect(!call(device,0x809,2,0),212);expect(!call(device,0x855,2,0),213);
    expect(!call(device,0x808,0,0),214);expect(AudioMusicNotifyForTest(&kind,&notifiedDevice)==2&&kind==4,215);
    play[1]=3;play[2]=4;expect(!call(device,0x806,5,play),216); /* last track, no TO */
    expect(!call(device,0x808,2,0),217);
    status[2]=0xdead;expect(call(device,0x814,0x102,status)==274,218);
    play[1]=1;expect(call(device,0x806,5,play)==282,219);
    expect(!call(device,0x804,2,0),220);expect(call(device,0x808,2,0)==257,221);
    SetLastError(0x77);expect(AudioMusicForTest(77,0x809,2,0)==23&&GetLastError()==0x88,222);
    ExitProcess(0);
}
