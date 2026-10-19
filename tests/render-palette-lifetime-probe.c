/* Two scheduled threads through the real Release/CreatePalette/SetPalette hooks.
 * Synthetic COM results/palette bytes are independently authored inputs. */
#include "../runtime/shadow/win32_min.h"
API void WIN RenderInstallForTest(void*,u32);
API u32 WIN RenderPaletteStateForTest(void*,void*,u32*);
API HANDLE WIN CreateThread(void*,u32,u32(WIN*)(void*),void*,u32,u32*);
API u32 WIN WaitForSingleObject(HANDLE,u32);
API u32 WIN GetTickCount(void);
API void WIN Sleep(u32);
static void** palette,**surface,**draw;
static u8 colors[1024];static volatile u32 phase,done;static u32 collision,releases,creates,binds;
static void check(int value,u32 code){if(!value)ExitProcess(code);}
static void entry(void){check(GetLastError()==0x77,71);SetLastError(0x88);}
static u32 WIN release(void* p){check(p==&palette,72);entry();++releases;return 0;}
static i32 WIN read_colors(void* p,u32 flags,u32 first,u32 count,void* out){check(p==&palette&&!flags&&first+count<=256,91);for(u32 i=0;i<count*4;++i)((u8*)out)[i]=colors[first*4+i];SetLastError(0x88);return 23;}
static i32 WIN factory(void* d,u32 caps,u8* entries,void** out,void* outer){check(d==&draw&&caps==0x44&&!outer,73);entry();for(u32 i=0;i<1024;++i)colors[i]=entries[i];++creates;*out=&palette;return 23;}
static i32 WIN bind(void* s,void* p){check(s==&surface&&p==&palette,74);entry();++binds;
 if(collision){__atomic_store_n(&phase,1,__ATOMIC_RELEASE);u32 start=GetTickCount();while(!__atomic_load_n(&done,__ATOMIC_ACQUIRE)){check(GetTickCount()-start<3000,75);Sleep(1);}SetLastError(0x88);}return 23;}
static u32 WIN worker(void* unused){(void)unused;if(collision){u32 start=GetTickCount();while(!__atomic_load_n(&phase,__ATOMIC_ACQUIRE)){check(GetTickCount()-start<3000,76);Sleep(1);}}
 SetLastError(0x77);check(!((u32(WIN*)(void*))palette[2])(&palette)&&GetLastError()==0x88,77);
 u8 next[1024];for(u32 i=0;i<1024;++i)next[i]=(u8)(i*13+37);void* made=0;
 SetLastError(0x77);check(((i32(WIN*)(void*,u32,u8*,void**,void*))draw[5])(&draw,0x44,next,&made,0)==23&&made==&palette&&GetLastError()==0x88,78);
 __atomic_store_n(&done,1,__ATOMIC_RELEASE);return 0;}
void start(void){
 static void* p[7],*s[33],*d[7];p[2]=(void*)release;p[4]=(void*)read_colors;s[31]=(void*)bind;d[5]=(void*)factory;palette=p;surface=s;draw=d;
 RenderInstallForTest(&palette,20);RenderInstallForTest(&surface,14);RenderInstallForTest(&draw,4);
 u8 first[1024];for(u32 i=0;i<1024;++i)first[i]=(u8)(i*7+11);void* made=0;
 SetLastError(0x77);check(((i32(WIN*)(void*,u32,u8*,void**,void*))draw[5])(&draw,0x44,first,&made,0)==23&&GetLastError()==0x88,79);
 static u32 before[259],after[259];check(RenderPaletteStateForTest(&surface,&palette,before)&&before[0]&&before[1],80);
 char mode[2];collision=GetEnvironmentVariableA("MNM_PALETTE_RACE",mode,2)==1&&mode[0]=='1';
 HANDLE thread=CreateThread(0,0,worker,0,0,0);check(thread!=0,81);if(!collision)check(!WaitForSingleObject(thread,4000),82);
 SetLastError(0x77);check(((i32(WIN*)(void*,void*))surface[31])(&surface,&palette)==23&&GetLastError()==0x88,83);
 check(!WaitForSingleObject(thread,4000)&&CloseHandle(thread),84);check(RenderPaletteStateForTest(&surface,&palette,after),85);
 check(releases==1&&creates==2&&binds==1,86);
 if(collision)check(!after[1],87);else{check(after[0]>before[0]&&after[1]&&after[2],88);for(u32 i=0;i<1024;++i)check(((u8*)(after+3))[i]==(u8)(i*13+37),89);}
 u32 result[7]={collision,before[0],after[0],after[1],after[2],releases,creates},written;HANDLE f=CreateFileA("palette-race.bin",0x40000000,0,0,1,0x80,0);
 check(f!=(HANDLE)-1&&WriteFile(f,result,sizeof(result),&written,0)&&written==sizeof(result)&&CloseHandle(f),90);ExitProcess(0);
}
