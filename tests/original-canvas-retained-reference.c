/* Retained later COM interfaces and held CPU/GDI leases across real original
 * wrapper retirement. No synthetic COM results and no calls after final free. */
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
static void* query(void* p,const u8* iid,u32 phase,u32 version){
    void* result=0;u32 hr=((u32(WIN*)(void*,const void*,void**))vt(p)[0])(p,iid,&result);
    emit(70,phase,version,hr,result!=0,0);original_require(!hr&&result,210);return result;
}
static void lock_surface(void* surface,u32 version,u32* desc,u32 phase){
    zero(desc,124);desc[0]=version==3?108:124;
    u32 hr=((u32(WIN*)(void*,void*,void*,u32,HANDLE))vt(surface)[25])(surface,0,desc,0x11,0);
    emit(71,phase,version,hr,desc[4],desc[9]);
    original_require(!hr&&desc[21]==16&&desc[3]==8&&desc[2]==6&&desc[4]>=16&&desc[4]<=4096,211);
}
static void unlock_surface(void* surface,u32 phase,u32 version){
    u32 hr=((u32(WIN*)(void*,void*))vt(surface)[32])(surface,0);emit(72,phase,version,hr,0,0);original_require(!hr,212);
}
static void pixels(u32* desc,u32 phase,u32 stage,u32 lease){
    u32 bad=0;
    for(u32 y=0;y<6;++y)for(u32 x=0;x<8;++x){
        u32 value=*((u16*)((u8*)desc[9]+y*desc[4])+x),expected=0x1234;
        if(stage&&lease==1&&x==2&&y==2)expected=0x6a35;
        if(stage&&lease==2&&x==2&&y==2)expected=desc[22];
        emit(73,phase,stage,value,y*8+x,expected);bad+=value!=expected;
    }
    emit(74,phase,stage,0,bad,48);original_require(!bad,213);
}
void start(void){
    original_map();output=CreateFileA("retained-canvas.bin",0x40000000,0,0,1,0,0);original_require(output!=(HANDLE)-1,214);
    void* hwnd=create_window();original_require(hwnd!=0,215);HANDLE lib=LoadLibraryA("ddraw.dll"),gdi=LoadLibraryA("gdi32.dll");original_require(lib&&gdi,216);
    typedef u32(WIN*Create)(void*,void**,void*);Create create=(Create)GetProcAddress(lib,"DirectDrawCreate");void* dd=0;original_require(create&&create(0,&dd,0)==0,217);
    static const u8 draw2[16]={0xe0,0xf3,0xa6,0xb3,0x43,0x2b,0xcf,0x11,0xa2,0xde,0,0xaa,0,0xb9,0x33,0x56};
    static const u8 interfaces[3][16]={
        {0,0x4e,4,0xda,0xb2,0x69,0xd0,0x11,0xa1,0xd5,0,0xaa,0,0xb8,0xdf,0xbb},
        {0x30,0x86,0x2b,0x0b,0x35,0xad,0xd0,0x11,0x8e,0xa6,0,0x60,0x97,0x97,0xea,0x5b},
        {0x80,0x5a,0x67,6,0x9b,0x3b,0xd2,0x11,0xb9,0x2f,0,0x60,0x97,0x97,0xea,0x5b}};
    void* dd2=query(dd,draw2,99,2);
    original_require(((u32(WIN*)(void*,void*,u32))vt(dd)[20])(dd,hwnd,0x11)==0,218);
    original_require(((u32(WIN*)(void*,u32,u32,u32))vt(dd)[21])(dd,640,480,16)==0,219);activate(hwnd,0);
    *(void**)0x6f98c4=dd;*(void**)0x6f98c8=dd2;*(u32*)0x6de6dd=0;
    typedef u32(WIN*SetPixel)(void*,i32,i32,u32);SetPixel set_pixel=(SetPixel)GetProcAddress(gdi,"SetPixelV");original_require(set_pixel!=0,220);
    void* table[39];zero(table,sizeof(table));table[2]=alias_release;table[38]=alias_page_unlock;
    u32 phase=0;
    for(u32 cap=0;cap<2;++cap)for(u32 versionIndex=0;versionIndex<3;++versionIndex)for(u32 lease=0;lease<3;++lease)for(u32 page=0;page<2;++page){
        u32 object[16],desc[31],version=versionIndex==2?7:versionIndex+3;zero(object,sizeof(object));zero(desc,sizeof(desc));
        desc[0]=108;desc[1]=0x1007;desc[2]=6;desc[3]=8;desc[18]=32;desc[19]=0x40;desc[21]=16;desc[22]=0xf800;desc[23]=0x7e0;desc[24]=31;desc[26]=cap?0x4040:0x840;
        *(u32*)0x6e1f1c=page;alias_phase=phase;
        typedef void(__attribute__((thiscall))*Surface)(void*,void*,u32,u32);((Surface)0x58ad90)(object,desc,0,0);
        original_require(object[1]&&object[2]&&*(u32*)0x6f8154==1,221);
        emit(75,phase,version,0,lease,(cap<<1)|page);
        struct AliasProxy a={table,(void*)object[1],1,0},b={table,(void*)object[2],2,0};
        void* retained=query(a.real,interfaces[versionIndex],phase,version);
        // Retained interface identity is distinct from wrapper ownership. Lock
        // descriptor addresses are runtime observations, not allocation IDs.
        lock_surface(retained,version,desc,phase);
        for(u32 y=0;y<6;++y)for(u32 x=0;x<8;++x)emit(76,phase,0,*((u16*)((u8*)desc[9]+y*desc[4])+x),y*8+x,0);
        unlock_surface(retained,phase,version);fill(a.real,phase,1,0x1234);
        lock_surface(retained,version,desc,phase);pixels(desc,phase,0,lease);
        void* dc=0;
        if(lease!=1)unlock_surface(retained,phase,version);
        if(lease==2){u32 hr=((u32(WIN*)(void*,void**))vt(retained)[17])(retained,&dc);emit(77,phase,version,hr,dc!=0,0);original_require(!hr&&dc,222);}
        object[1]=(u32)&a;object[2]=(u32)&b;
        typedef void(__attribute__((thiscall))*Retire)(void*);((Retire)0x58b3e0)(object);
        u32 dirty=object[1]|object[2]|object[4]|object[5]|object[6]|object[10];emit(78,phase,version,0,dirty,*(u32*)0x6f8154);original_require(!dirty&&!*(u32*)0x6f8154,223);
        if(lease==1){*((u16*)((u8*)desc[9]+2*desc[4])+2)=0x6a35;pixels(desc,phase,1,lease);unlock_surface(retained,phase,version);}
        if(lease==2){original_require(set_pixel(dc,2,2,0xff),224);u32 hr=((u32(WIN*)(void*,void*))vt(retained)[26])(retained,dc);emit(79,phase,version,hr,0,0);original_require(!hr,225);}
        lock_surface(retained,version,desc,phase);pixels(desc,phase,1,lease);unlock_surface(retained,phase,version);
        // A new QI credit on the surviving object is safe. Its numerical
        // address may equal a retired wrapper interface; it is a new credit.
        static const u8 unknown[16]={0,0,0,0,0,0,0,0,0xc0,0,0,0,0,0,0,0x46};
        void* identity=query(retained,unknown,phase,0);release(identity);
        u32 refs=((u32(WIN*)(void*))vt(retained)[2])(retained);emit(80,phase,version,refs,0,0);original_require(!refs,226);
        ++phase;
    }
    original_require(((u32(WIN*)(void*))vt(dd)[19])(dd)==0,227);original_require(((u32(WIN*)(void*,void*,u32))vt(dd)[20])(dd,hwnd,8)==0,228);
    release(dd2);release(dd);typedef u32(WIN*Destroy)(void*);Destroy destroy=(Destroy)GetProcAddress(user_dll,"DestroyWindow");original_require(destroy&&destroy(hwnd),229);
    emit(99,99,0,0,phase,0);CloseHandle(output);ExitProcess(0);
}
