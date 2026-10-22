/* Actual Wine aliases and unchanged original release. Proxies forward real COM
 * calls and record their returned values. No API is called after final Release. */
#define start unused_surface_restore_entry
#include "surface-driver-restore-reference.c"
#undef start
#include "original-pe32-image.h"
static void* create_window(void){
    user_dll=LoadLibraryA("user32.dll");original_require(user_dll!=0,132);
    typedef void*(WIN*Window)(u32,const char*,const char*,u32,i32,i32,i32,i32,void*,void*,void*,void*);
    Window window=(Window)GetProcAddress(user_dll,"CreateWindowExA");
    typedef u16(WIN*Register)(void*);Register reg=(Register)GetProcAddress(user_dll,"RegisterClassA");
    def_window=(i32(WIN*)(void*,u32,u32,i32))GetProcAddress(user_dll,"DefWindowProcA");
    u32 cls[10];zero(cls,sizeof(cls));cls[1]=(u32)window_proc;cls[9]=(u32)"MnmOriginalAlias";
    original_require(window&&reg&&def_window&&reg(cls),133);
    return window(0,"MnmOriginalAlias","original alias lifetime control",0x10cf0000,0,0,640,480,0,0,0,0);
}
struct AliasProxy {void** table;void* real;u32 id,calls;};
static u32 alias_phase;
static u32 WIN alias_release(struct AliasProxy* p){
    u32 result=((u32(WIN*)(void*))vt(p->real)[2])(p->real);
    ++p->calls;emit(64,alias_phase,p->id,result,0,0);return result;
}
static u32 WIN alias_page_unlock(struct AliasProxy* p,u32 flags){
    u32 result=((u32(WIN*)(void*,u32))vt(p->real)[38])(p->real,flags);
    emit(63,alias_phase,p->id,result,flags,0);return result;
}
static void alias_shared_pixels(void* writer,void* reader,u32 id,u32 colour){
    u32 fx[25],desc[27];zero(fx,sizeof(fx));fx[0]=100;fx[20]=colour;
    original_require(((u32(WIN*)(void*,void*,void*,void*,u32,void*))vt(writer)[5])
        (writer,0,0,0,0x1000400,fx)==0,181);
    zero(desc,sizeof(desc));desc[0]=108;
    original_require(((u32(WIN*)(void*,void*,void*,u32,HANDLE))vt(reader)[25])
        (reader,0,desc,0x11,0)==0,182);
    original_require(desc[21]==16&&desc[4]>=16&&desc[2]==6&&desc[3]==8,183);
    u32 mismatch=0;
    for(u32 y=0;y<6;++y)for(u32 x=0;x<8;++x)
        mismatch+=*((u16*)((u8*)desc[9]+y*desc[4])+x)!=(u16)colour;
    original_require(((u32(WIN*)(void*,void*))vt(reader)[32])(reader,0)==0,184);
    emit(62,alias_phase,id,0,mismatch,48);original_require(!mismatch,185);
}
void start(void){
    original_map();output=CreateFileA("original-alias.bin",0x40000000,0,0,1,0,0);
    original_require(output!=(HANDLE)-1,186);
    void* hwnd=create_window();original_require(hwnd!=0,187);
    HANDLE dll=LoadLibraryA("ddraw.dll");original_require(dll!=0,188);
    typedef u32(WIN*Create)(void*,void**,void*);
    Create create=(Create)GetProcAddress(dll,"DirectDrawCreate");void* dd=0;void* dd2=0;
    original_require(create&&create(0,&dd,0)==0,189);
    static const u8 draw2iid[16]={0xe0,0xf3,0xa6,0xb3,0x43,0x2b,0xcf,0x11,0xa2,0xde,0,0xaa,0,0xb9,0x33,0x56};
    static const u8 unknowniid[16]={0,0,0,0,0,0,0,0,0xc0,0,0,0,0,0,0,0x46};
    original_require(((u32(WIN*)(void*,const void*,void**))vt(dd)[0])(dd,draw2iid,&dd2)==0&&dd2,190);
    original_require(((u32(WIN*)(void*,void*,u32))vt(dd)[20])(dd,hwnd,0x11)==0,191);
    original_require(((u32(WIN*)(void*,u32,u32,u32))vt(dd)[21])(dd,640,480,16)==0,198);
    activate(hwnd,0);
    *(void**)0x6f98c4=dd;*(void**)0x6f98c8=dd2;*(u32*)0x6de6dd=0;
    void* table[39];zero(table,sizeof(table));table[2]=alias_release;table[38]=alias_page_unlock;
    const u32 held[3]={0,1,3};
    for(u32 first=0;first<3;++first)for(u32 second=0;second<3;++second)for(u32 locked=0;locked<2;++locked){
        u32 object[16],desc[27];zero(object,sizeof(object));zero(desc,sizeof(desc));
        desc[0]=108;desc[1]=0x1007;desc[2]=6;desc[3]=8;desc[18]=32;desc[19]=0x40;
        desc[21]=16;desc[22]=0xf800;desc[23]=0x7e0;desc[24]=31;desc[26]=0x840;
        *(u32*)0x6e1f1c=locked;
        typedef void(__attribute__((thiscall))*Surface)(void*,void*,u32,u32);
        ((Surface)0x58ad90)(object,desc,0,0);
        original_require(object[1]&&object[2]&&*(u32*)0x6f8154==1,192);
        emit(60,alias_phase,0,0,held[first],held[second]);
        struct AliasProxy a={table,(void*)object[1],1,0},b={table,(void*)object[2],2,0};
        void* identity1=0;void* identity2=0;
        original_require(((u32(WIN*)(void*,const void*,void**))vt(a.real)[0])(a.real,unknowniid,&identity1)==0&&identity1,193);
        original_require(((u32(WIN*)(void*,const void*,void**))vt(b.real)[0])(b.real,unknowniid,&identity2)==0&&identity2,194);
        original_require(identity1==identity2,195);release(identity2);release(identity1);
        emit(61,alias_phase,0,0,1,locked);
        alias_shared_pixels(a.real,b.real,1,0x1234);alias_shared_pixels(b.real,a.real,2,0xabcd);
        for(u32 i=0;i<held[first];++i)((u32(WIN*)(void*))vt(a.real)[1])(a.real);
        for(u32 i=0;i<held[second];++i)((u32(WIN*)(void*))vt(b.real)[1])(b.real);
        object[1]=(u32)&a;object[2]=(u32)&b;
        typedef void(__attribute__((thiscall))*Release)(void*);((Release)0x58b3e0)(object);
        /* Borrowed credits were drained by original release. Calling them now
         * would violate the lifetime this experiment is measuring. */
        emit(65,alias_phase,0,0,a.calls,b.calls);
        u32 dirty=object[1]|object[2]|object[4]|object[5]|object[6]|object[10];
        emit(66,alias_phase,0,0,dirty,*(u32*)0x6f8154);
        original_require(!dirty&&*(u32*)0x6f8154==0&&a.calls==held[first]+1&&b.calls==held[second]+1,196);
        ++alias_phase;
    }
    original_require(((u32(WIN*)(void*))vt(dd)[19])(dd)==0,199);
    original_require(((u32(WIN*)(void*,void*,u32))vt(dd)[20])(dd,hwnd,8)==0,200);
    release(dd2);release(dd);
    typedef u32(WIN*Destroy)(void*);Destroy destroy=(Destroy)GetProcAddress(user_dll,"DestroyWindow");
    original_require(destroy&&destroy(hwnd),197);emit(99,99,0,0,18,0);CloseHandle(output);ExitProcess(0);
}
