#include "../shadow/win32_min.h"
API i32 WIN AudioCreateForTest(void**);
#define CHECK(x) do {if(!(x))ExitProcess(__LINE__);}while(0)
struct Com {void** v;};
#define METHOD(o,n,type) ((type)((struct Com*)(o))->v[n])
typedef u32 (WIN *Ref)(void*);
typedef i32 (WIN *CreateBuffer)(void*,void*,void**,void*);
typedef i32 (WIN *Caps)(void*,u32*);
typedef i32 (WIN *Duplicate)(void*,void*,void**);
typedef i32 (WIN *Value)(void*,u32);
typedef i32 (WIN *Read)(void*,u32*);
typedef i32 (WIN *Play)(void*,u32,u32,u32);
typedef i32 (WIN *Stop)(void*);
typedef i32 (WIN *Format)(void*,void*);
typedef i32 (WIN *Lock)(void*,u32,u32,void**,u32*,void**,u32*,u32);
typedef i32 (WIN *Unlock)(void*,void*,u32,void*,u32);
void start(void){
    char mode[8];
    if(GetEnvironmentVariableA("MNM_AUDIO_STALE_TEST",mode,sizeof(mode))){
        void* absent=0;CHECK(AudioCreateForTest(&absent)<0 && !absent);
        u32 start=0;API u32 WIN GetTickCount(void);start=GetTickCount();
        CHECK(AudioCreateForTest(&absent)<0 && GetTickCount()-start<100);
        ExitProcess(0);
    }
    void* device=0;CHECK(AudioCreateForTest(&device)==0 && device);
    u32 caps[24]={96};CHECK(METHOD(device,4,Caps)(device,caps)==0 && caps[1]==15);
    u32 primaryDesc[5]={20,0x81,0,0,0};void* primary=0;
    CHECK(METHOD(device,3,CreateBuffer)(device,primaryDesc,&primary,0)==0);
    CHECK(METHOD(primary,12,Play)(primary,0,0,1)==0);u32 value=0;
    CHECK(METHOD(primary,9,Read)(primary,&value)==0 && value==3);
    CHECK(METHOD(primary,15,Value)(primary,(u32)-2000)==0);
    CHECK(METHOD(primary,6,Read)(primary,&value)==0 && value==(u32)-2000);
    CHECK(METHOD(primary,15,Value)(primary,1)<0);
    CHECK(METHOD(primary,6,Read)(primary,&value)==0 && value==(u32)-2000);
    CHECK(METHOD(primary,12,Play)(primary,0,0,0)<0);
    CHECK(METHOD(primary,18,Stop)(primary)==0);
    CHECK(METHOD(primary,9,Read)(primary,&value)==0 && value==0);
    CHECK(METHOD(primary,15,Value)(primary,0)==0);
    u8 format[18]={1,0,1,0,0x44,0xac,0,0,0x88,0x58,1,0,2,0,16,0,0,0};
    CHECK(METHOD(primary,14,Format)(primary,format)==0);
    format[12]=1;CHECK(METHOD(primary,14,Format)(primary,format)<0);format[12]=2;
    u32 desc[5]={20,0xea,8,0,(u32)format};void* buffer=0;
    CHECK(METHOD(device,3,CreateBuffer)(device,desc,&buffer,0)==0 && buffer);
    void* memory=0;u32 bytes=0;
    CHECK(METHOD(buffer,11,Lock)(buffer,0,8,&memory,&bytes,0,0,0)==0 && bytes==8);
    ((u16*)memory)[0]=0x8000;((u16*)memory)[1]=0xffff;((u16*)memory)[2]=1000;((u16*)memory)[3]=32767;
    CHECK(METHOD(buffer,12,Play)(buffer,0,0,2)<0);
    CHECK(METHOD(buffer,19,Unlock)(buffer,memory,9,0,0)<0);
    CHECK(METHOD(buffer,19,Unlock)(buffer,memory,8,0,0)==0);
    CHECK(METHOD(buffer,19,Unlock)(buffer,memory,8,0,0)<0);
    CHECK(METHOD(buffer,15,Value)(buffer,(u32)-2000)==0);
    CHECK(METHOD(buffer,16,Value)(buffer,1000)==0);
    CHECK(METHOD(buffer,12,Play)(buffer,0,0,1)==0);
    CHECK(METHOD(buffer,9,Read)(buffer,&value)==0 && value==3);
    CHECK(METHOD(primary,18,Stop)(primary)==0);
    CHECK(METHOD(buffer,9,Read)(buffer,&value)==0 && value==3);
    void* child=0;CHECK(METHOD(device,5,Duplicate)(device,buffer,&child)==0 && child);
    CHECK(METHOD(child,9,Read)(child,&value)==0 && value==0);
    CHECK(METHOD(buffer,2,Ref)(buffer)==0);
    CHECK(METHOD(child,12,Play)(child,0,0,0)==0);
    CHECK(METHOD(child,9,Read)(child,&value)==0 && value==1);
    CHECK(METHOD(child,17,Value)(child,44000)<0);
    CHECK(METHOD(child,13,Value)(child,1)<0);
    CHECK(METHOD(child,18,Stop)(child)==0);
    CHECK(METHOD(child,13,Value)(child,0)==0);
    CHECK(METHOD(child,9,Read)(child,&value)==0 && value==0);
    CHECK(METHOD(child,1,Ref)(child)==2 && METHOD(child,2,Ref)(child)==1);
    CHECK(METHOD(child,2,Ref)(child)==0);
    CHECK(METHOD(primary,2,Ref)(primary)==0 && METHOD(device,2,Ref)(device)==0);
    ExitProcess(0);
}
