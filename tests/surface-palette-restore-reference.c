/* Indexed Surface2 palette retention and explicit reload probe. No game image. */
#include "../runtime/shadow/win32_min.h"
#ifndef PROBE_BPP
#define PROBE_BPP 8
#endif
API void WIN Sleep(u32);
API HANDLE WIN LoadLibraryA(const char*);
API void* WIN GetProcAddress(HANDLE,const char*);
static void zero(void* p,u32 n){u8* q=p;while(n--)*q++=0;}
static void** vt(void* p){return *(void***)p;}
static void check(int ok,u32 stage){if(!ok)ExitProcess(stage);}
static void release(void* p){if(p)((u32(WIN*)(void*))vt(p)[2])(p);}
static HANDLE output;
static HANDLE user_dll;
static void* palettes[2];

static void emit(u32 kind,u32 phase,u32 id,u32 result,u32 a,u32 b){u32 row[6]={kind,phase,id,result,a,b},n;check(WriteFile(output,row,sizeof(row),&n,0)&&n==sizeof(row),90);}
static void display(void* dd,u32 phase){u32 d[27];zero(d,sizeof(d));d[0]=108;u32 h=((u32(WIN*)(void*,void*))vt(dd)[12])(dd,d);emit(18,phase,0,h,d[21],d[22]);emit(19,phase,0,d[23],d[3],d[2]);}

static i32(WIN*def_window)(void*,u32,u32,i32);
static i32 WIN window_proc(void* hwnd,u32 msg,u32 w,i32 l){return def_window(hwnd,msg,w,l);}
static void activate(void* hwnd,u32 phase){
 typedef int(WIN*Foreground)(void*);typedef void*(WIN*GetForeground)(void);typedef int(WIN*Peek)(void*,void*,u32,u32,u32);typedef i32(WIN*Message)(void*);
 Foreground set=(Foreground)GetProcAddress(user_dll,"SetForegroundWindow");GetForeground get=(GetForeground)GetProcAddress(user_dll,"GetForegroundWindow");Peek peek=(Peek)GetProcAddress(user_dll,"PeekMessageA");Message dispatch=(Message)GetProcAddress(user_dll,"DispatchMessageA"),translate=(Message)GetProcAddress(user_dll,"TranslateMessage");check(set&&get&&peek&&dispatch&&translate,19);
 typedef int(WIN*Show)(void*,i32);typedef void*(WIN*Active)(void*);Show show=(Show)GetProcAddress(user_dll,"ShowWindow");Active active=(Active)GetProcAddress(user_dll,"SetActiveWindow");check(show&&active,22);show(hwnd,5);active(hwnd);
 u32 h=0;for(u32 cycle=0;cycle<40;++cycle){show(hwnd,5);active(hwnd);h=set(hwnd);u32 msg[7];for(u32 n=0;n<128&&peek(msg,0,0,0,1);++n){translate(msg);dispatch(msg);}Sleep(50);}emit(14,phase,0,h,get()==hwnd,0);check(get()==hwnd,28);
}
static void* make(void* dd,u32 id,u32 caps){u32 d[27];void* p=0;zero(d,sizeof(d));d[0]=108;d[1]=1;d[26]=caps;
 if(id!=3){d[1]=0x1007;d[2]=6;d[3]=8;d[18]=32;d[19]=0x60;d[21]=8;}
 u32 h=((u32(WIN*)(void*,void*,void**,void*))vt(dd)[6])(dd,d,&p,0);emit(1,0,id,h,caps,0);if(h)return 0;
 static const u8 iid[16]={0x85,0x58,0x80,0x57,0xec,0x6e,0xcf,0x11,0x94,0x41,0xa8,0x23,3,0xc1,0x0e,0x27};void* second=0;
 check(((u32(WIN*)(void*,const void*,void**))vt(p)[0])(p,iid,&second)==0&&second,18);release(p);return second;
}
static u32 color(u32 i,u32 id,u32 generation){return ((i*17+id*37+generation*11)&255)|(((i*29+3+generation*19)&255)<<8)|(((i*43+9+generation*23)&255)<<16);}
static void palette_create(void* dd,u32 id){u32 entries[256];for(u32 i=0;i<256;++i)entries[i]=color(i,id,0);u32 h=((u32(WIN*)(void*,u32,void*,void**,void*))vt(dd)[5])(dd,0x44,entries,&palettes[id],0);emit(20,0,id,h,0x44,256);check(!h&&palettes[id],23);}
static void palette_update(u32 phase,u32 id,u32 first,u32 count,u32 generation){u32 entries[256];for(u32 i=0;i<count;++i)entries[i]=color(first+i,id,generation);u32 h=((u32(WIN*)(void*,u32,u32,u32,void*))vt(palettes[id])[6])(palettes[id],0,first,count,entries);emit(23,phase,id,h,first,count);}
static void bind(void* s,u32 phase,u32 id,u32 palette){emit(24,phase,id,((u32(WIN*)(void*,void*))vt(s)[31])(s,palettes[palette]),palette,0);}
static void palette_inspect(void* s,u32 phase,u32 id,u32 palette){void* got=0;u32 h=((u32(WIN*)(void*,void**))vt(s)[20])(s,&got);emit(21,phase,id,h,got==palettes[palette],palette);
 if(!h){u32 entries[256];zero(entries,sizeof(entries));h=((u32(WIN*)(void*,u32,u32,u32,void*))vt(got)[4])(got,0,0,256,entries);emit(25,phase,id,h,0,256);if(!h)for(u32 i=0;i<256;++i)emit(22,phase,id,0,i,entries[i]);release(got);}}
static void reload(void* s,u32 phase,u32 id){u32 d[27];zero(d,sizeof(d));d[0]=108;u32 h=((u32(WIN*)(void*,void*,void*,u32,HANDLE))vt(s)[25])(s,0,d,1,0);emit(26,phase,id,h,d[21],d[4]);if(!h){check(d[21]==8&&d[2]>=6&&d[3]>=8,24);for(u32 y=0;y<d[2];++y)for(u32 x=0;x<d[3];++x)*((u8*)d[9]+y*d[4]+x)=(u8)((y*8+x)*37+id*11+phase*13);emit(27,phase,id,((u32(WIN*)(void*,void*))vt(s)[32])(s,0),0,0);}}
static void inspect(void* s,u32 phase,u32 id){if(!s)return;palette_inspect(s,phase,id,id==2||phase==9?1:0);u32 k[2]={0,0},d[27];
 emit(2,phase,id,((u32(WIN*)(void*))vt(s)[24])(s),0,0);
 u32 h=((u32(WIN*)(void*,u32,void*))vt(s)[16])(s,8,k);emit(3,phase,id,h,k[0],k[1]);
 zero(d,sizeof(d));d[0]=108;h=((u32(WIN*)(void*,void*,void*,u32,HANDLE))vt(s)[25])(s,0,d,1,0);emit(4,phase,id,h,d[21],d[4]);
 if(!h){check(d[21]==8&&d[2]>=6&&d[3]>=8,21);emit(15,phase,id,d[21],d[3],d[2]);emit(16,phase,id,d[22],d[23],d[24]);u32 hash=2166136261u;for(u32 y=0;y<6;++y)for(u32 x=0;x<8;++x){u32 v=*((u8*)d[9]+y*d[4]+x);hash=(hash^v)*16777619u;emit(5,phase,id,0,y*8+x,v);}emit(6,phase,id,hash,d[22],d[23]);emit(7,phase,id,((u32(WIN*)(void*,void*))vt(s)[32])(s,0),0,0);}
}
static void fill(void* s,u32 phase,u32 id,u32 color){if(!s)return;u32 fx[25];zero(fx,sizeof(fx));fx[0]=100;fx[20]=color;emit(8,phase,id,((u32(WIN*)(void*,void*,void*,void*,u32,void*))vt(s)[5])(s,0,0,0,0x1000400,fx),color,0);}
void start(void){output=CreateFileA("outputs.bin",0x40000000,0,0,1,0,0);check(output!=(HANDLE)-1,10);
 HANDLE user=LoadLibraryA("user32.dll"),lib=LoadLibraryA("ddraw.dll");check(user&&lib,11);user_dll=user;
 typedef void*(WIN*Window)(u32,const char*,const char*,u32,i32,i32,i32,i32,void*,void*,void*,void*);
 Window window=(Window)GetProcAddress(user,"CreateWindowExA");typedef u16(WIN*Register)(void*);Register reg=(Register)GetProcAddress(user,"RegisterClassA");def_window=(i32(WIN*)(void*,u32,u32,i32))GetProcAddress(user,"DefWindowProcA");u32 cls[10];zero(cls,sizeof(cls));cls[1]=(u32)window_proc;cls[9]=(u32)"MnmRestoreProbe";check(window&&reg&&def_window&&reg(cls),12);void* hwnd=window(0,"MnmRestoreProbe","bounded Restore probe",0x10cf0000,0,0,640,480,0,0,0,0);check(hwnd!=0,13);activate(hwnd,8);
 void* dd=0;typedef u32(WIN*Create)(void*,void**,void*);Create create=(Create)GetProcAddress(lib,"DirectDrawCreate");check(create&&create(0,&dd,0)==0,14);
 u32 h=((u32(WIN*)(void*,void*,u32))vt(dd)[20])(dd,hwnd,0x11);emit(9,0,0,h,0x11,0);check(!h,15);
 typedef u32(WIN*Mode)(void*,u32,u32,u32);Mode mode=(Mode)vt(dd)[21];h=mode(dd,640,480,PROBE_BPP);emit(10,0,0,h,640,480);check(!h,16);display(dd,0);activate(hwnd,0);
 palette_create(dd,0);palette_create(dd,1);void* s[4];u32 caps[4]={0x840,0x4040,0x40,0x200};
 for(u32 i=0;i<4;++i){s[i]=make(dd,i,caps[i]);if(s[i]){emit(12,0,i,((u32(WIN*)(void*))vt(s[i])[27])(s[i]),0,0);bind(s[i],0,i,i==2?1:0);u32 key[2]={0x35,0x35};emit(11,0,i,((u32(WIN*)(void*,u32,void*))vt(s[i])[29])(s[i],8,key),key[0],key[1]);fill(s[i],0,i,0x12);inspect(s[i],0,i);}}
 h=mode(dd,800,600,PROBE_BPP);emit(10,1,0,h,800,600);display(dd,1);activate(hwnd,1);palette_update(1,0,37,7,1);for(u32 i=0;i<4;++i){inspect(s[i],1,i);fill(s[i],1,i,0xab);}
 for(u32 i=0;i<4;++i)if(s[i]){emit(12,2,i,((u32(WIN*)(void*))vt(s[i])[27])(s[i]),0,0);inspect(s[i],2,i);}
 h=mode(dd,640,480,PROBE_BPP);emit(10,3,0,h,640,480);display(dd,3);activate(hwnd,3);for(u32 i=0;i<4;++i)inspect(s[i],3,i);
 for(u32 i=0;i<4;++i)if(s[i]){emit(12,4,i,((u32(WIN*)(void*))vt(s[i])[27])(s[i]),0,0);inspect(s[i],4,i);emit(12,5,i,((u32(WIN*)(void*))vt(s[i])[27])(s[i]),0,0);inspect(s[i],5,i);reload(s[i],6,i);inspect(s[i],6,i);}
 palette_update(8,0,0,256,2);palette_update(8,1,0,256,2);for(u32 i=0;i<4;++i)inspect(s[i],8,i);bind(s[1],9,1,1);inspect(s[1],9,1);for(u32 i=0;i<4;++i)release(s[i]);release(palettes[0]);release(palettes[1]);
 emit(13,7,0,((u32(WIN*)(void*))vt(dd)[19])(dd),0,0);emit(9,7,0,((u32(WIN*)(void*,void*,u32))vt(dd)[20])(dd,hwnd,8),8,0);release(dd);
 typedef int(WIN*Destroy)(void*);Destroy destroy=(Destroy)GetProcAddress(user,"DestroyWindow");check(destroy!=0&&destroy(hwnd),17);emit(17,7,0,0,0,0);CloseHandle(output);ExitProcess(0);
}
