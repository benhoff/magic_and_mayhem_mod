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
    u8* fade=image+0x820d8;static const u8 fadeBytes[5]={0xb8,1,0x78,0,0x78};
    for(u32 i=0;i<7;++i)mixer[i]=mixerBytes[i];for(u32 i=0;i<13;++i)volume[i]=volumeBytes[i];
    for(u32 i=0;i<7;++i)query[i]=queryBytes[i];
    for(u32 i=0;i<5;++i)fade[i]=fadeBytes[i];
    *iat=0;expect(!AudioMusicInstallForTest((u32)image)&&mixer[0]==0xff&&volume[0]==0x53,226);*iat=original;
    mixer[5]=1;expect(!AudioMusicInstallForTest((u32)image)&&*iat==original&&volume[0]==0x53,227);mixer[5]=0;
    volume[5]=0;expect(!AudioMusicInstallForTest((u32)image)&&*iat==original&&mixer[0]==0xff,228);volume[5]=0xed;
    query[6]=1;expect(!AudioMusicInstallForTest((u32)image)&&*iat==original&&mixer[0]==0xff&&volume[0]==0x53,232);query[6]=0;
    u32 old,info[7];expect(VirtualProtect(image,0x200000,0x20,&old),233);
    expect(AudioMusicInstallForTest((u32)image)&&*iat!=original&&mixer[0]==0xe8&&volume[0]==0xe9,229);
    expect(VirtualQuery(mixer,info,sizeof(info))&&info[5]==0x20,234);
    expect(VirtualQuery(query,info,sizeof(info))&&info[5]==0x20,235);
    expect(((u32 (WIN *)(void))mixer)()==0,230);
    static u32 object[0x330/4];object[0x32c/4]=1;
    expect(((i32 (__thiscall *)(void*,i32))volume)(object,7)==0,231);
    object[0x328/4]=1;i32 level=-1;SetLastError(0x77);
    expect(((i32 (__thiscall *)(void*,i32*))query)(object,&level)==0&&level==7&&GetLastError()==0x77,236);
    object[0x328/4]=0;level=-1;expect(((i32 (__thiscall *)(void*,i32*))query)(object,&level)==0&&level==0,237);
    object[0x328/4]=1;object[0x32c/4]=0;level=-1;expect(((i32 (__thiscall *)(void*,i32*))query)(object,&level)==0&&level==0,238);
    object[0x32c/4]=1;expect(((i32 (__thiscall *)(void*,i32*))query)(object,0)==-6,239);
}
static void marker(const char* name){
    char path[512];u32 length=GetEnvironmentVariableA(name,path,sizeof(path));expect(length&&length<sizeof(path),240);
    HANDLE file=CreateFileA(path,0x40000000,0,0,2,0x80,0);expect(file!=(HANDLE)-1,241);u32 written=0;
    expect(WriteFile(file,"ready",5,&written,0)&&written==5,242);CloseHandle(file);
}
void start(void){
    install_fixture();u32 open[5]={0x11223344,0,(u32)"cdaudio",0,0};expect(!call(0,0x803,0x2102,open),243);
    u32 play[3]={0x11223344,2,3};expect(!call(open[1],0x806,13,play),244);expect(!call(open[1],0x809,2,0),245);
    u32 status[4]={0,0,4,0};expect(!call(open[1],0x814,0x102,status),246);
    marker("MNM_MUSIC_FAULT_READY");u32 started=GetTickCount(),result=0;
    do{result=call(open[1],0x814,0x102,status);if(result)break;expect(GetTickCount()-started<10000,247);Sleep(20);}while(1);
    expect(result==276,248);u32 kind=0,device=0;
    while(!AudioMusicNotifyForTest(&kind,&device)){expect(GetTickCount()-started<11000,249);Sleep(5);}
    expect(kind==8&&device==open[1],250);marker("MNM_MUSIC_FAULT_DONE");Sleep(500);
    expect(call(open[1],0x814,0x102,status)==276,251); /* host resume cannot revive ownership */
    ExitProcess(0);
}
