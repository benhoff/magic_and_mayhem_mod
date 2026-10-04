#include "../shadow/win32_min.h"
#include "protocol.h"
API HANDLE WIN CreateFileMappingA(HANDLE,void*,u32,u32,u32,const char*);
API void* WIN MapViewOfFile(HANDLE,u32,u32,u32,u32);
API u32 WIN GetFileSize(HANDLE,u32*);
API u32 WIN GetTickCount(void);
API void WIN Sleep(u32);
static void copy(void* d,const void* s,u32 n){u8* a=d;const u8* b=s;while(n--)*a++=*b++;}
static int same(const void* a,const void* b,u32 n){const u8* x=a;const u8* y=b;while(n--)if(*x++!=*y++)return 0;return 1;}
static u32* channel;static volatile u32 busy,dead;static u32 next;
typedef i32 (WIN *Create)(void*,void**,void*);static Create legacy;
#define INVALID ((i32)0x80070057)
#define UNSUPPORTED ((i32)0x80004001)
#define FAILED ((i32)0x80004005)
static i32 request(u32 op,u32 id,u32 value,u32 length,const u8* format,u32 flags,const void* data,u32* result){
    if(!channel || dead || !__atomic_load_n(channel+20,__ATOMIC_ACQUIRE))return FAILED;
    if(__atomic_exchange_n(&busy,1,__ATOMIC_ACQUIRE))return (i32)0x887800aa;
    i32 status=FAILED;
    if(length>MNM_AUDIO_PAYLOAD)goto done;
    u32 serial=++next;if(!serial){__atomic_store_n(&dead,1,__ATOMIC_RELEASE);goto done;}
    channel[5]=op;channel[6]=id;channel[7]=value;channel[8]=length;
    for(u32 i=9;i<16;++i)channel[i]=0;channel[15]=flags;
    if(format){u16 v;u32 d;copy(&v,format,2);channel[14]=v;copy(&v,format+2,2);channel[10]=v;
        copy(&d,format+4,4);channel[9]=d;copy(&d,format+8,4);channel[13]=d;
        copy(&v,format+12,2);channel[12]=v;copy(&v,format+14,2);channel[11]=v;}
    if(data && length)copy((u8*)channel+128,data,length);
    __atomic_store_n(channel+4,serial,__ATOMIC_RELEASE);
    u32 start=GetTickCount();
    while(__atomic_load_n(channel+16,__ATOMIC_ACQUIRE)!=serial){
        if(!__atomic_load_n(channel+20,__ATOMIC_ACQUIRE) || GetTickCount()-start>1000){__atomic_store_n(&dead,1,__ATOMIC_RELEASE);goto done;}
        Sleep(1);
    }
    status=(i32)channel[17];if(!status && result)*result=channel[18];
done:if(status)__atomic_add_fetch(channel+23,1,__ATOMIC_RELAXED);
    __atomic_store_n(&busy,0,__ATOMIC_RELEASE);return status;
}
struct Object {void** table;volatile u32 refs;u32 kind,id,bytes,flags,playing;u8 format[18];i32 volume,pan;u8* staging;u32 locked;};
static void* device_table[11];static void* buffer_table[21];
static struct Object* object(u32 kind){struct Object* o=HeapAlloc(GetProcessHeap(),8,sizeof(*o));if(o){o->refs=1;o->kind=kind;o->table=kind==1?device_table:buffer_table;}return o;}
static i32 WIN query(struct Object* o,const u8* guid,void** out){
    static const u8 unknown[16]={0,0,0,0,0,0,0,0,0xc0,0,0,0,0,0,0,0x46};
    static const u8 sound[16]={0x83,0xfa,0x9a,0x27,0x81,0x49,0xce,0x11,0xa5,0x21,0,0x20,0xaf,0x0b,0xe5,0x60};
    static const u8 buffer[16]={0x85,0xfa,0x9a,0x27,0x81,0x49,0xce,0x11,0xa5,0x21,0,0x20,0xaf,0x0b,0xe5,0x60};
    if(!out || !guid)return INVALID;*out=0;
    if(!same(guid,unknown,16) && !same(guid,o->kind==1?sound:buffer,16))return (i32)0x80004002;
    __atomic_add_fetch(&o->refs,1,__ATOMIC_RELAXED);*out=o;return 0;
}
static u32 WIN addref(struct Object* o){return __atomic_add_fetch(&o->refs,1,__ATOMIC_RELAXED);}
static u32 WIN release(struct Object* o){u32 refs=__atomic_sub_fetch(&o->refs,1,__ATOMIC_ACQ_REL);if(!refs){if(o->id)request(MNM_AUDIO_RELEASE,o->id,0,0,0,0,0,0);if(o->staging)HeapFree(GetProcessHeap(),0,o->staging);HeapFree(GetProcessHeap(),0,o);}return refs;}
static i32 WIN caps(struct Object* o,u32* out){
    if(!out || out[0]!=(o->kind==1?96u:20u))return INVALID;
    for(u32 i=1;i<out[0]/4;++i)out[i]=0;
    if(o->kind==1){out[1]=15;out[2]=100;out[3]=200000;}
    else {out[1]=o->flags;out[2]=o->bytes;}
    return 0;
}
static i32 WIN create_buffer(struct Object* device,u32* desc,void** out,void* outer){
    (void)device;if(!out)return INVALID;*out=0;
    if(!desc || desc[0]!=20 || desc[3] || outer)return INVALID;
    u32 primary=desc[1]==0x81;if(primary && (desc[2] || desc[4]))return INVALID;
    if(!primary && (!desc[4] || desc[1]!=0xea))return UNSUPPORTED;
    struct Object* o=object(primary?2:3);if(!o)return (i32)0x8007000e;
    o->flags=desc[1];o->bytes=desc[2];
    i32 status=0;
    if(!primary){copy(o->format,(void*)desc[4],18);if(o->format[16] || o->format[17])status=UNSUPPORTED;
        else status=request(MNM_AUDIO_CREATE,0,0,o->bytes,o->format,o->flags,0,&o->id);}
    if(status){release(o);return status;}*out=o;return 0;
}
static i32 WIN duplicate(struct Object* device,struct Object* source,void** out){
    (void)device;if(!out)return INVALID;*out=0;
    if(!source || source->table!=buffer_table || source->kind!=3)return INVALID;
    struct Object* o=object(3);if(!o)return (i32)0x8007000e;
    o->bytes=source->bytes;o->flags=source->flags;o->volume=source->volume;o->pan=source->pan;copy(o->format,source->format,18);
    i32 status=request(MNM_AUDIO_DUPLICATE,source->id,0,0,0,0,0,&o->id);
    if(status){release(o);return status;}*out=o;return 0;
}
static i32 WIN cooperate(struct Object* o,void* window,u32 level){(void)o;return window && level==2?0:UNSUPPORTED;}
static i32 WIN compact(struct Object* o){(void)o;return 0;}
static i32 WIN get_speakers(struct Object* o,u32* value){(void)o;if(!value)return INVALID;*value=4;return 0;}
static i32 WIN set_speakers(struct Object* o,u32 value){(void)o;(void)value;return UNSUPPORTED;}
static i32 WIN initialize(struct Object* o,void* guid){(void)o;(void)guid;return UNSUPPORTED;}
static i32 WIN position(struct Object* o,u32* read,u32* write){(void)o;(void)read;(void)write;return UNSUPPORTED;}
static i32 WIN get_format(struct Object* o,void* format,u32 size,u32* written){if(written)*written=18;if(!format)return written?0:INVALID;if(size<18)return INVALID;copy(format,o->format,18);return 0;}
static i32 WIN get_volume(struct Object* o,i32* out){if(!out)return INVALID;*out=o->volume;return 0;}
static i32 WIN get_pan(struct Object* o,i32* out){if(!out)return INVALID;*out=o->pan;return 0;}
static i32 WIN get_frequency(struct Object* o,u32* out){if(!out)return INVALID;copy(out,o->format+4,4);return 0;}
static i32 WIN get_status(struct Object* o,u32* out){if(!out)return INVALID;if(o->kind==2){*out=o->playing;return 0;}return request(MNM_AUDIO_STATUS,o->id,0,0,0,0,0,out);}
static i32 WIN buffer_init(struct Object* o,void* device,void* desc){(void)o;(void)device;(void)desc;return UNSUPPORTED;}
static i32 WIN lock(struct Object* o,u32 offset,u32 length,void** a,u32* na,void** b,u32* nb,u32 flags){
    if(o->kind!=3 || offset || flags)return UNSUPPORTED;
    if(!a || !na || !length || length>o->bytes || o->staging || (!!b!=!!nb))return INVALID;
    o->staging=HeapAlloc(GetProcessHeap(),8,length);if(!o->staging)return (i32)0x8007000e;
    o->locked=length;*a=o->staging;*na=length;if(b){*b=0;*nb=0;}return 0;
}
static i32 WIN unlock(struct Object* o,void* a,u32 na,void* b,u32 nb){
    if(!o->staging || a!=o->staging || na>o->locked || b || nb)return INVALID;
    i32 status=na?request(MNM_AUDIO_UPLOAD,o->id,0,na,0,0,a,0):0;
    HeapFree(GetProcessHeap(),0,o->staging);o->staging=0;o->locked=0;return status;
}
static i32 WIN play(struct Object* o,u32 a,u32 b,u32 flags){if(a || b || flags>1)return UNSUPPORTED;if(o->kind==2){o->playing=flags?3:1;return 0;}return request(MNM_AUDIO_PLAY,o->id,flags,0,0,0,0,0);}
static i32 WIN stop(struct Object* o){if(o->kind==2){o->playing=0;return 0;}return request(MNM_AUDIO_STOP,o->id,0,0,0,0,0,0);}
static i32 WIN reset(struct Object* o,u32 pos){if(pos || o->kind!=3)return UNSUPPORTED;return request(MNM_AUDIO_RESET,o->id,0,0,0,0,0,0);}
static i32 WIN set_format(struct Object* o,const u8* f){if(o->kind!=2)return UNSUPPORTED;if(!f)return INVALID;copy(o->format,f,18);return 0;}
static i32 WIN set_volume(struct Object* o,i32 volume){if(volume< -10000 || volume>0)return INVALID;if(o->kind==2 && volume)return UNSUPPORTED;i32 status=o->kind==2?0:request(MNM_AUDIO_VOLUME,o->id,(u32)volume,0,0,0,0,0);if(!status)o->volume=volume;return status;}
static i32 WIN set_pan(struct Object* o,i32 pan){if(pan< -10000 || pan>10000)return INVALID;i32 status=o->kind==2?UNSUPPORTED:request(MNM_AUDIO_PAN,o->id,(u32)pan,0,0,0,0,0);if(!status)o->pan=pan;return status;}
static i32 WIN frequency(struct Object* o,u32 value){(void)o;(void)value;return UNSUPPORTED;}
static i32 WIN restore(struct Object* o){(void)o;return 0;}
static void tables(void){
    device_table[0]=query;device_table[1]=addref;device_table[2]=release;device_table[3]=create_buffer;device_table[4]=caps;device_table[5]=duplicate;
    device_table[6]=cooperate;device_table[7]=compact;device_table[8]=get_speakers;device_table[9]=set_speakers;device_table[10]=initialize;
    buffer_table[0]=query;buffer_table[1]=addref;buffer_table[2]=release;buffer_table[3]=caps;buffer_table[4]=position;buffer_table[5]=get_format;
    buffer_table[6]=get_volume;buffer_table[7]=get_pan;buffer_table[8]=get_frequency;buffer_table[9]=get_status;buffer_table[10]=buffer_init;
    buffer_table[11]=lock;buffer_table[12]=play;buffer_table[13]=reset;buffer_table[14]=set_format;buffer_table[15]=set_volume;
    buffer_table[16]=set_pan;buffer_table[17]=frequency;buffer_table[18]=stop;buffer_table[19]=unlock;buffer_table[20]=restore;
}
static i32 WIN create_sound(void* guid,void** out,void* outer){
    // Ownership is selected once for the whole device. No fallback on owned voices.
    if(guid || outer || !out || request(MNM_AUDIO_PING,0,0,0,0,0,0,0)){
        if(channel)__atomic_add_fetch(channel+22,1,__ATOMIC_RELAXED);
        return legacy?legacy(guid,out,outer):FAILED;
    }
    *out=object(1);if(*out)__atomic_add_fetch(channel+21,1,__ATOMIC_RELAXED);
    return *out?0:(i32)0x8007000e;
}
__declspec(dllexport) void AudioAnchor(void){}
__declspec(dllexport) i32 WIN AudioCreateForTest(void** out){tables();return create_sound(0,out,0);}
int WIN DllMain(void* instance,u32 reason,void* reserved){
    (void)instance;(void)reserved;if(reason!=1)return 1;tables();
    char path[512];u32 size=GetEnvironmentVariableA("MNM_AUDIO_CHANNEL",path,sizeof(path));if(!size || size>=sizeof(path))return 1;
    HANDLE file=CreateFileA(path,0xc0000000,3,0,3,0x80,0);if(file==(HANDLE)-1)return 1;
    if(GetFileSize(file,0)!=MNM_AUDIO_SIZE){CloseHandle(file);return 1;}
    HANDLE mapping=CreateFileMappingA(file,0,4,0,MNM_AUDIO_SIZE,0);CloseHandle(file);if(!mapping)return 1;
    channel=MapViewOfFile(mapping,2,0,0,MNM_AUDIO_SIZE);CloseHandle(mapping);
    if(!channel || !same(channel,"MNMAUD01",8) || channel[2]!=MNM_AUDIO_VERSION || channel[3]!=MNM_AUDIO_SIZE){channel=0;return 1;}
#ifndef MNM_AUDIO_SELFTEST
    u32 base=(u32)GetModuleHandleA(0),protection,ignored;
    static const u8 thunk[6]={0xff,0x25,0x1c,0x50,0x5c,0};
    if(base!=0x400000 || !same((void*)(base+0x197566),thunk,6))return 1;
    void** iat=(void**)(base+0x1c501c);
    if(!VirtualProtect(iat,4,4,&protection))return 1;
    legacy=(Create)*iat;__atomic_store_n(iat,(void*)&create_sound,__ATOMIC_RELEASE);VirtualProtect(iat,4,protection,&ignored);
#endif
    return 1;
}
