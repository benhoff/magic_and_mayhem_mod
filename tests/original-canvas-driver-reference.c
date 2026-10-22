/* Selected unchanged game wrappers with real Wine COM outcomes. The forwarding
 * proxy records results; it never scripts HRESULTs or substitutes driver pixels.
 * UI errors, hardware reset and the original full game remain outside scope. */
#define start unused_surface_restore_entry
#include "surface-driver-restore-reference.c"
#undef start
#include "original-pe32-image.h"
API u32 WIN GetLastError(void);
API void WIN SetLastError(u32);
struct Proxy {void** table;void* real;u32 id;};
static u32 phase;
static u32 proxy_result(struct Proxy* p,u32 kind,u32 result,u32 flags,u32 value){
    u32 error=GetLastError();emit(kind,phase,p->id,result,flags,value);SetLastError(error);return result;
}
static u32 WIN proxy_blt(struct Proxy* p,void* dest,void* source,void* rect,u32 flags,void* fx){
    u32 h=((u32(WIN*)(void*,void*,void*,void*,u32,void*))vt(p->real)[5])(p->real,dest,source,rect,flags,fx);
    return proxy_result(p,41,h,flags,fx?((u32*)fx)[20]:0);
}
static u32 WIN proxy_lock(struct Proxy* p,void* rect,void* desc,u32 flags,HANDLE event){
    u32 h=((u32(WIN*)(void*,void*,void*,u32,HANDLE))vt(p->real)[25])(p->real,rect,desc,flags,event);
    return proxy_result(p,42,h,flags,h?0:((u32*)desc)[4]);
}
static u32 WIN proxy_restore(struct Proxy* p){
    u32 h=((u32(WIN*)(void*))vt(p->real)[27])(p->real);
    return proxy_result(p,43,h,0,0);
}
static void original_fill(u32* object,u32 colour){
    typedef void(__attribute__((thiscall))*Fill)(void*,u32);
    ((Fill)0x58bc10)(object,colour);
}
static void original_read(u32* object,struct Proxy* p,u32 expected,int defined){
    typedef void*(__attribute__((thiscall))*Lock)(void*);
    void* pixels=((Lock)0x58b660)(object);
    original_require(pixels&&object[6]>=16&&object[6]<=8192&&*(u32*)0x6f8150==0,130);
    u32 hash=2166136261u,mismatch=0;
    for(u32 y=0;y<6;++y)for(u32 x=0;x<8;++x){
        u16 word=*((u16*)((u8*)pixels+y*object[6])+x);
        if(defined&&word!=(u16)expected)++mismatch;
        hash=(hash^word)*16777619u;
    }
    emit(44,phase,p->id,0,mismatch,hash);
    u32 h=((u32(WIN*)(void*,void*))vt(p->real)[32])(p->real,0);
    emit(45,phase,p->id,h,48,defined);original_require(!h&&!mismatch,131);
}
static void* create_window(void){
    user_dll=LoadLibraryA("user32.dll");original_require(user_dll!=0,132);
    typedef void*(WIN*Window)(u32,const char*,const char*,u32,i32,i32,i32,i32,void*,void*,void*,void*);
    Window window=(Window)GetProcAddress(user_dll,"CreateWindowExA");
    typedef u16(WIN*Register)(void*);Register reg=(Register)GetProcAddress(user_dll,"RegisterClassA");
    def_window=(i32(WIN*)(void*,u32,u32,i32))GetProcAddress(user_dll,"DefWindowProcA");
    u32 cls[10];zero(cls,sizeof(cls));cls[1]=(u32)window_proc;cls[9]=(u32)"MnmOriginalDriver";
    original_require(window&&reg&&def_window&&reg(cls),133);
    return window(0,"MnmOriginalDriver","original wrapper driver control",0x10cf0000,0,0,640,480,0,0,0,0);
}
void start(void){
    original_map();output=CreateFileA("original-driver.bin",0x40000000,0,0,1,0,0);
    original_require(output!=(HANDLE)-1,134);
    void* hwnd=create_window();original_require(hwnd!=0,135);
    void* dll=LoadLibraryA("ddraw.dll");original_require(dll!=0,136);
    typedef u32(WIN*Create)(void*,void**,void*);
    Create create=(Create)GetProcAddress(dll,"DirectDrawCreate");void* dd=0;void* dd2=0;
    original_require(create&&create(0,&dd,0)==0,137);
    static const u8 iid[16]={0xe0,0xf3,0xa6,0xb3,0x43,0x2b,0xcf,0x11,0xa2,0xde,0,0xaa,0,0xb9,0x33,0x56};
    original_require(((u32(WIN*)(void*,const void*,void**))vt(dd)[0])(dd,iid,&dd2)==0&&dd2,138);
    *(void**)0x6f98c4=dd;*(void**)0x6f98c8=dd2;
    *(u32*)0x6e1f1c=0;*(u32*)0x6de6dd=0;
    original_require(((u32(WIN*)(void*,void*,u32))vt(dd)[20])(dd,hwnd,0x11)==0,139);
    typedef u32(WIN*Mode)(void*,u32,u32,u32);Mode mode=(Mode)vt(dd)[21];
    void* proxy_table[33];zero(proxy_table,sizeof(proxy_table));
    proxy_table[5]=proxy_blt;proxy_table[25]=proxy_lock;proxy_table[27]=proxy_restore;
    for(u32 operation=0;operation<2;++operation){
        original_require(mode(dd,640,480,16)==0,140);activate(hwnd,operation*10);
        u32 objects[2][16];struct Proxy proxy[2];zero(objects,sizeof(objects));
        for(u32 id=0;id<2;++id){
            u32 desc[27];zero(desc,sizeof(desc));desc[0]=108;desc[1]=0x1007;desc[2]=6;desc[3]=8;
            desc[18]=32;desc[19]=0x40;desc[21]=16;desc[22]=0xf800;desc[23]=0x7e0;desc[24]=31;
            desc[26]=id?0x4040:0x840;
            typedef void(__attribute__((thiscall))*Surface)(void*,void*,u32,u32);
            ((Surface)0x58ad90)(objects[id],desc,0,0);
            original_require(objects[id][1]&&objects[id][2]&&objects[id][4]==8&&objects[id][5]==6,141);
            proxy[id].table=proxy_table;proxy[id].real=(void*)objects[id][2];proxy[id].id=id;
            objects[id][2]=(u32)&proxy[id];phase=operation*10;
            emit(40,phase,id,0,desc[26],*(u32*)0x6f8154);
            original_fill(objects[id],0x1234);original_read(objects[id],&proxy[id],0x1234,1);
        }
        original_require(mode(dd,800,600,16)==0,142);activate(hwnd,operation*10+1);
        phase=operation*10+1;
        for(u32 id=0;id<2;++id){
            u32 lost=((u32(WIN*)(void*))vt(proxy[id].real)[24])(proxy[id].real);
            emit(46,phase,id,lost,operation,0);
            original_require(lost==(id?0x887601c2u:0),143);
            if(operation){original_read(objects[id],&proxy[id],0,0);original_fill(objects[id],0x1abcd);}
            else original_fill(objects[id],0x1abcd);
            original_read(objects[id],&proxy[id],0xabcd,1);
        }
        phase=operation*10+2;
        for(u32 id=0;id<2;++id){
            objects[id][2]=(u32)proxy[id].real;
            typedef void(__attribute__((thiscall))*Release)(void*);
            ((Release)0x58b3e0)(objects[id]);
            emit(47,phase,id,0,objects[id][1]|objects[id][2]|objects[id][4]|objects[id][5]|objects[id][6],*(u32*)0x6f8154);
            original_require(!objects[id][1]&&!objects[id][2]&&!objects[id][4]&&!objects[id][5]&&!objects[id][6],144);
        }
        original_require(*(u32*)0x6f8154==0,145);
    }
    original_require(((u32(WIN*)(void*))vt(dd)[19])(dd)==0,146);
    original_require(((u32(WIN*)(void*,void*,u32))vt(dd)[20])(dd,hwnd,8)==0,147);
    release(dd2);release(dd);
    typedef u32(WIN*Destroy)(void*);Destroy destroy=(Destroy)GetProcAddress(user_dll,"DestroyWindow");
    original_require(destroy&&destroy(hwnd),148);emit(99,99,0,0,4,0);CloseHandle(output);ExitProcess(0);
}
