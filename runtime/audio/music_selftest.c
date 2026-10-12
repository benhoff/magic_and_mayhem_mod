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
    u8* query=image+0x818b0;static const u8 queryBytes[7]={0x51,0x8b,0x81,0x2c,3,0,0};
    u8* fade=image+0x820d8;static const u8 fadeBytes[32]={0xb8,1,0x78,0,0x78,0xf7,0x6c,0x24,0x10,0xc1,0xfa,0x0b,0x8b,0xc2,0xc1,0xe8,0x1f,3,0xd0,0x8b,0xea,0xeb,2,0x33,0xed,0x33,0xc0,0xe9,0,0,0,0};
    u8* fadeCaller=image+0x82300;static const u8 callerBytes[26]={0x51,0x57,0x8b,0xf9,0x53,0x55,0x56,0x33,0xc0,0x89,0x44,0x24,0x10,0xe9,0,0,0,0,0x8b,0xc5,0x5e,0x5d,0x5b,0x5f,0x59,0xc3};
    for(u32 i=0;i<32;++i)fade[i]=fadeBytes[i];for(u32 i=0;i<26;++i)fadeCaller[i]=callerBytes[i];
    *(u32*)(fadeCaller+14)=(u32)fade-(u32)(fadeCaller+18);*(u32*)(fade+28)=(u32)(fadeCaller+18)-(u32)(fade+32);
    for(u32 i=0;i<7;++i)mixer[i]=mixerBytes[i];for(u32 i=0;i<13;++i)volume[i]=volumeBytes[i];
    for(u32 i=0;i<7;++i)query[i]=queryBytes[i];
    *iat=0;expect(!AudioMusicInstallForTest((u32)image)&&mixer[0]==0xff&&volume[0]==0x53,226);*iat=original;
    mixer[5]=1;expect(!AudioMusicInstallForTest((u32)image)&&*iat==original&&volume[0]==0x53,227);mixer[5]=0;
    volume[5]=0;expect(!AudioMusicInstallForTest((u32)image)&&*iat==original&&mixer[0]==0xff,228);volume[5]=0xed;
    query[6]=1;expect(!AudioMusicInstallForTest((u32)image)&&*iat==original&&mixer[0]==0xff&&volume[0]==0x53,232);query[6]=0;
    fade[4]=0;expect(!AudioMusicInstallForTest((u32)image)&&*iat==original&&mixer[0]==0xff&&volume[0]==0x53&&query[0]==0x51,240);fade[4]=0x78;
    u32 old,info[7];expect(VirtualProtect(image,0x200000,0x20,&old),233);
    expect(AudioMusicInstallForTest((u32)image)&&*iat!=original&&mixer[0]==0xe8&&volume[0]==0xe9,229);
    expect(VirtualQuery(mixer,info,sizeof(info))&&info[5]==0x20,234);
    expect(VirtualQuery(query,info,sizeof(info))&&info[5]==0x20,235);
    expect(VirtualQuery(fade,info,sizeof(info))&&info[5]==0x20&&fade[0]==0xe8,241);
    expect(((u32 (WIN *)(void))mixer)()==0,230);
    static u32 object[0x330/4];object[0x32c/4]=1;
    expect(((i32 (__thiscall *)(void*,i32))volume)(object,7)==0,231);
    object[0x328/4]=1;i32 level=-1;SetLastError(0x77);
    expect(((i32 (__thiscall *)(void*,i32*))query)(object,&level)==0&&level==7&&GetLastError()==0x77,236);
    expect(((i32 (__thiscall *)(void*))fadeCaller)(object)==7&&GetLastError()==0x77,242);
    object[0x328/4]=0;level=-1;expect(((i32 (__thiscall *)(void*,i32*))query)(object,&level)==0&&level==0,237);
    expect(((i32 (__thiscall *)(void*))fadeCaller)(object)==0,243);
    object[0x328/4]=1;object[0x32c/4]=0;level=-1;expect(((i32 (__thiscall *)(void*,i32*))query)(object,&level)==0&&level==0,238);
    object[0x32c/4]=1;expect(((i32 (__thiscall *)(void*,i32*))query)(object,0)==-6,239);
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
