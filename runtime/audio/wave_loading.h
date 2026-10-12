#pragma once
/* Only the eight calls in 0x570140 are redirected. Generic file helpers and
   unrelated callers retain their original implementation. Host pointers never
   cross the channel; stack-instance associations are local to this PE module. */
#define WAVE_CALL __attribute__((thiscall))
struct WaveHandle {void* instance;struct NativeWave wave;struct WaveHandle* next;};
static struct WaveHandle* wave_handles;
static struct WaveHandle* wave_find(void* instance){
    for(struct WaveHandle* h=wave_handles;h;h=h->next)if(h->instance==instance)return h;
    return 0;
}
static void WAVE_CALL wave_close(void* instance){
    struct WaveHandle** link=&wave_handles;
    while(*link){struct WaveHandle* h=*link;if(h->instance==instance){
        *link=h->next;HeapFree(GetProcessHeap(),0,h->wave.pcm);HeapFree(GetProcessHeap(),0,h);return;
    }link=&h->next;}
}
static u32 WAVE_CALL wave_open(void* instance,const char* path){
    if(!instance || !path)return 0;wave_close(instance);
    u32 size=0,start=0;
    for(;size<512 && path[size];++size){
        if((u8)path[size]<32 || (u8)path[size]>=127)return 0;
        if(path[size]=='/' || path[size]=='\\')start=size+1;
        if(path[size]=='.' && path[size+1]=='.' &&
           (size==0 || path[size-1]=='/' || path[size-1]=='\\') &&
           (!path[size+2] || path[size+2]=='/' || path[size+2]=='\\'))return 0;
    }
    if(!size || size>=512 || size-start>259 || size==start)return 0;
    struct WaveHandle* h=HeapAlloc(GetProcessHeap(),8,sizeof(*h));if(!h)return 0;
    if(request_wave(MNM_AUDIO_LOAD_WAVE,0,0,size-start,0,0,path+start,0,&h->wave,0)){
        HeapFree(GetProcessHeap(),0,h);return 0;
    }
    h->instance=instance;h->next=wave_handles;wave_handles=h;return 1;
}
static u32 WAVE_CALL wave_size(void* instance){struct WaveHandle* h=wave_find(instance);return h?h->wave.bytes:0;}
static u32 WAVE_CALL wave_format(void* instance,u8* out){
    struct WaveHandle* h=wave_find(instance);if(!h || !out)return 0;copy(out,h->wave.format,18);return 1;
}
static u32 WAVE_CALL wave_duration(void* instance,u32* out){
    struct WaveHandle* h=wave_find(instance);if(!h || !out)return 0;*out=h->wave.duration;return 1;
}
static u32 WAVE_CALL wave_read(void* instance,void** out,u32 capacity){
    struct WaveHandle* h=wave_find(instance);if(!h || !out || !*out)return 0;
    u32 count=capacity<h->wave.bytes?capacity:h->wave.bytes;copy(*out,h->wave.pcm,count);return count;
}
#ifndef MNM_AUDIO_SELFTEST
static int wave_install(u32 base){
    static const u32 calls[]={0x1701bf,0x1701e2,0x170216,0x170229,0x1702ca,0x170305,0x17031d,0x170349};
    static const u32 targets[]={0x18fbb0,0x18fb30,0x1900d0,0x18ff10,0x190140,0x18fb30,0x18ffb0,0x18fb30};
    void* hooks[]={wave_open,wave_close,wave_size,wave_format,wave_read,wave_close,wave_duration,wave_close};
    for(u32 i=0;i<8;++i){u8* at=(u8*)(base+calls[i]);i32 relative;
        if(at[0]!=0xe8)return 0;copy(&relative,at+1,4);
        if((u32)(at+5)+relative!=base+targets[i])return 0;
    }
    u32 protection,ignored;
    /* One containing region: all guards precede the first mutation. */
    if(!VirtualProtect((void*)(base+0x170140),0x225,0x40,&protection))return 0;
    for(u32 i=0;i<8;++i){u8* at=(u8*)(base+calls[i]);i32 relative=(i32)((u32)hooks[i]-(u32)(at+5));copy(at+1,&relative,4);}
    FlushInstructionCache(GetCurrentProcess(),(void*)(base+0x170140),0x225);
    VirtualProtect((void*)(base+0x170140),0x225,protection,&ignored);return 1;
}
#endif
