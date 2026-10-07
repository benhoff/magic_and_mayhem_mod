/* Standalone Wine Surface2 palette/alias lifetime and actual two-buffer flips. */
#define start retained_palette_probe
#include "surface-palette-restore-reference.c"
#undef start
static const u8 iid1[16]={0x81,0xdb,0x14,0x6c,0x33,0xa7,0xce,0x11,0xa5,0x21,0,0x20,0xaf,0x0b,0xe5,0x60};
static const u8 unknown[16]={0,0,0,0,0,0,0,0,0xc0,0,0,0,0,0,0,0x46};
static void* query(void* s,const u8* iid,u32 phase,u32 id){void* p=0;u32 h=((u32(WIN*)(void*,const void*,void**))vt(s)[0])(s,iid,&p);emit(30,phase,id,h,p!=0,0);check(!h&&p,31);return p;}
static void alias_identity(void* s,void* alias,u32 phase,u32 id){void* a=query(s,unknown,phase,id),*b=query(alias,unknown,phase,id);emit(31,phase,id,0,a==b,0);release(a);release(b);}
static void sample(void* s,u32 phase,u32 id){u32 d[27];zero(d,sizeof(d));d[0]=108;u32 h=((u32(WIN*)(void*,void*,void*,u32,HANDLE))vt(s)[25])(s,0,d,1,0);emit(32,phase,id,h,d[21],d[4]);check(!h&&d[21]==8&&d[3]>=8&&d[2]>=6,32);
 for(u32 y=0;y<6;++y)for(u32 x=0;x<8;++x)emit(33,phase,id,0,y*8+x,*((u8*)d[9]+y*d[4]+x));check(!((u32(WIN*)(void*,void*))vt(s)[32])(s,0),33);
 u32 k[2]={0,0};h=((u32(WIN*)(void*,u32,void*))vt(s)[16])(s,8,k);emit(34,phase,id,h,k[0],k[1]);
}
static void colors(void* s,u32 phase,u32 id,void* expected){void* got=0;u32 h=((u32(WIN*)(void*,void**))vt(s)[20])(s,&got);emit(35,phase,id,h,got==expected,got!=0);
 if(!h){u32 p[256];zero(p,sizeof(p));h=((u32(WIN*)(void*,u32,u32,u32,void*))vt(got)[4])(got,0,0,256,p);emit(36,phase,id,h,0,256);check(!h,34);for(u32 i=0;i<256;++i)emit(37,phase,id,0,i,p[i]);release(got);}}
static void setkey(void* s,u32 phase,u32 id,u32 key){u32 p[2]={key,key};emit(38,phase,id,((u32(WIN*)(void*,u32,void*))vt(s)[29])(s,8,p),key,0);}
static void drop(void* p,u32 phase,u32 id){emit(39,phase,id,((u32(WIN*)(void*))vt(p)[2])(p),0,0);}
static void flip(void* front,void* target,u32 phase,u32 flags){emit(40,phase,3,((u32(WIN*)(void*,void*,u32))vt(front)[11])(front,target,flags),target!=0,flags);}
static void focus(void* hwnd){
 typedef int(WIN*Position)(void*,void*,i32,i32,i32,i32,u32);typedef int(WIN*Point)(i32,i32);typedef void(WIN*Mouse)(u32,u32,u32,u32,u32);typedef int(WIN*Foreground)(void*);typedef void*(WIN*GetForeground)(void);
 Position pos=(Position)GetProcAddress(user_dll,"SetWindowPos");Point cursor=(Point)GetProcAddress(user_dll,"SetCursorPos");Mouse mouse=(Mouse)GetProcAddress(user_dll,"mouse_event");Foreground set=(Foreground)GetProcAddress(user_dll,"SetForegroundWindow");GetForeground get=(GetForeground)GetProcAddress(user_dll,"GetForegroundWindow");check(pos&&cursor&&mouse&&set&&get,45);
 for(u32 cycle=0;cycle<40&&get()!=hwnd;++cycle){pos(hwnd,(void*)-1,0,0,640,480,0x40);cursor(320,240);mouse(2,0,0,0,0);mouse(4,0,0,0,0);set(hwnd);u32 msg[7];typedef int(WIN*Peek)(void*,void*,u32,u32,u32);typedef i32(WIN*Message)(void*);Peek peek=(Peek)GetProcAddress(user_dll,"PeekMessageA");Message dispatch=(Message)GetProcAddress(user_dll,"DispatchMessageA"),translate=(Message)GetProcAddress(user_dll,"TranslateMessage");check(peek&&dispatch&&translate,46);for(u32 n=0;n<128&&peek(msg,0,0,0,1);++n){translate(msg);dispatch(msg);}Sleep(50);}
 check(get()==hwnd,47);
}
void start(void){output=CreateFileA("outputs.bin",0x40000000,0,0,1,0,0);check(output!=(HANDLE)-1,10);HANDLE user=LoadLibraryA("user32.dll"),lib=LoadLibraryA("ddraw.dll");check(user&&lib,11);user_dll=user;
 typedef void*(WIN*Window)(u32,const char*,const char*,u32,i32,i32,i32,i32,void*,void*,void*,void*);typedef u16(WIN*Register)(void*);
 Window window=(Window)GetProcAddress(user,"CreateWindowExA");Register reg=(Register)GetProcAddress(user,"RegisterClassA");def_window=(i32(WIN*)(void*,u32,u32,i32))GetProcAddress(user,"DefWindowProcA");u32 cls[10];zero(cls,sizeof(cls));cls[1]=(u32)window_proc;cls[9]=(u32)"MnmOwnershipProbe";check(window&&reg&&def_window&&reg(cls),12);void* hwnd=window(0,"MnmOwnershipProbe","surface ownership",0x10cf0000,0,0,640,480,0,0,0,0);check(hwnd!=0,13);
 typedef u32(WIN*Create)(void*,void**,void*);Create create=(Create)GetProcAddress(lib,"DirectDrawCreate");void* dd=0;check(create&&create(0,&dd,0)==0,14);check(!((u32(WIN*)(void*,void*,u32))vt(dd)[20])(dd,hwnd,0x11),15);check(!((u32(WIN*)(void*,u32,u32,u32))vt(dd)[21])(dd,640,480,8),16);focus(hwnd);check(!((u32(WIN*)(void*,u32,u32,u32))vt(dd)[21])(dd,640,480,8),16);display(dd,0);activate(hwnd,0);
 palette_create(dd,0);palette_create(dd,1);void* s0=make(dd,0,0x840),*s1=make(dd,1,0x840);check(s0&&s1,35);bind(s0,0,0,0);bind(s1,0,1,0);setkey(s0,0,0,0x35);setkey(s1,0,1,0x46);fill(s0,0,0,0x12);fill(s1,0,1,0x23);
 void* alias=query(s0,iid1,0,0);alias_identity(s0,alias,0,0);sample(alias,0,0);drop(s0,1,0);sample(alias,1,0);setkey(alias,1,0,0x57);sample(alias,2,0);
 drop(palettes[0],2,10);colors(alias,2,0,palettes[0]);colors(s1,2,1,palettes[0]);void* retained=0;check(!((u32(WIN*)(void*,void**))vt(s1)[20])(s1,&retained),36);palettes[0]=retained;palette_update(3,0,37,7,1);colors(alias,3,0,retained);colors(s1,3,1,retained);sample(alias,3,0);sample(s1,3,1);
 emit(41,4,0,((u32(WIN*)(void*,void*))vt(alias)[31])(alias,0),0,0);colors(alias,4,0,0);colors(s1,4,1,retained);drop(retained,4,10);drop(alias,4,0);
 colors(s1,5,1,palettes[0]);bind(s1,5,1,1);colors(s1,6,1,palettes[1]);sample(s1,6,1);drop(s1,6,1);
 u32 desc[27];zero(desc,sizeof(desc));desc[0]=108;desc[1]=0x21;desc[5]=1;desc[26]=0x218;void* front=0;u32 h=((u32(WIN*)(void*,void*,void**,void*))vt(dd)[6])(dd,desc,&front,0);emit(42,0,3,h,desc[26],1);check(!h&&front,37);
 static const u8 iid2[16]={0x85,0x58,0x80,0x57,0xec,0x6e,0xcf,0x11,0x94,0x41,0xa8,0x23,3,0xc1,0x0e,0x27};void* f2=query(front,iid2,0,3);release(front);front=f2;u32 caps=4;void* back=0;h=((u32(WIN*)(void*,void*,void**))vt(front)[12])(front,&caps,&back);emit(43,0,4,h,back!=0,0);check(!h&&back,38);void* fa=query(front,iid1,0,3),*ba=query(back,iid1,0,4);alias_identity(front,fa,0,3);alias_identity(back,ba,0,4);
 bind(front,0,3,1);bind(back,0,4,1);setkey(front,0,3,0x61);setkey(back,0,4,0x72);fill(front,0,3,0x31);fill(back,0,4,0xa5);
 for(u32 phase=10;phase<14;++phase){sample(fa,phase,3);sample(ba,phase,4);colors(front,phase,3,palettes[1]);colors(back,phase,4,palettes[1]);flip(front,phase==12?back:0,phase,1);sample(fa,phase+10,3);sample(ba,phase+10,4);}
 for(u32 held=0;held<4;++held){void* operand=held%2?back:front;void* dc=0;u32 d[27];zero(d,sizeof(d));d[0]=108;h=held<2?((u32(WIN*)(void*,void*,void*,u32,HANDLE))vt(operand)[25])(operand,0,d,1,0):((u32(WIN*)(void*,void**))vt(operand)[17])(operand,&dc);emit(44,30+held,held%2?4:3,h,held<2?0:1,0);check(!h,39);flip(front,0,30+held,0);h=held<2?((u32(WIN*)(void*,void*))vt(operand)[32])(operand,0):((u32(WIN*)(void*,void*))vt(operand)[26])(operand,dc);emit(45,30+held,held%2?4:3,h,0,0);void* other=held%2?front:back;h=held<2?((u32(WIN*)(void*,void*))vt(other)[32])(other,0):((u32(WIN*)(void*,void*))vt(other)[26])(other,dc);emit(47,30+held,held%2?3:4,h,0,0);if(held>=2){emit(48,30+held,3,((u32(WIN*)(void*,void*,u32))vt(front)[11])(front,0,1),0,1);h=((u32(WIN*)(void*,void*))vt(operand)[26])(operand,dc);emit(49,30+held,held%2?4:3,h,0,0);check(!h,40);}sample(front,30+held,3);sample(back,30+held,4);}
 drop(back,40,4);drop(ba,40,4);back=0;caps=4;h=((u32(WIN*)(void*,void*,void**))vt(front)[12])(front,&caps,&back);emit(43,40,4,h,back!=0,0);check(!h&&back,41);sample(back,40,4);drop(front,41,3);sample(fa,41,3);flip(fa,0,41,1);sample(fa,42,3);sample(back,42,4);drop(back,43,4);drop(fa,43,3);drop(palettes[1],43,11);
 check(!((u32(WIN*)(void*))vt(dd)[19])(dd),42);check(!((u32(WIN*)(void*,void*,u32))vt(dd)[20])(dd,hwnd,8),43);release(dd);typedef int(WIN*Destroy)(void*);Destroy destroy=(Destroy)GetProcAddress(user,"DestroyWindow");check(destroy&&destroy(hwnd),44);emit(46,99,0,0,0,0);CloseHandle(output);ExitProcess(0);
}
