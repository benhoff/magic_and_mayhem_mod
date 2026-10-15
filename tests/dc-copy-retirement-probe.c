/* Actual pinned copy wrappers with supplied COM/Win32 endpoints, no driver.
 * Check obsolete DLL refusal and two-argument ClientToScreen preservation. */
#include "../runtime/shadow/win32_min.h"
API void* WIN LoadLibraryA(const char*);
API i32 WIN FreeLibrary(void*);
API i32 WIN SetEnvironmentVariableA(const char*,const char*);
API i32 WIN ReadFile(HANDLE,void*,u32,u32*,void*);
API u32 WIN SetFilePointer(HANDLE,i32,i32*,u32);
static volatile u8 reserved_game_globals[4*1024*1024];
static const u32 entries[]={0x58bf40,0x58c140,0x58c360,0x58c4a0,0x58c6a0,0x58c8a0,0x58ca90,0x58cbc0,0x58cd50,0x58cf20};
static const u32 sites[]={0x58bfe2,0x58c1d6,0x58c3f5,0x58c534,0x58c6f8,0x58c922,0x58cb17,0x58cc16,0x58cdd1,0x58cfa1};
static u32 source[16],target[16],surface_table[33],window_table[5],surface[1],window[1];
static u32 clients,blts,fasts,bad_identity,results[30][16];
static i32 last_rect[4];
static i32 WIN get_window(void* self,u32* hwnd){if(self!=window)++bad_identity;*hwnd=0x1234;return 0;}
static i32 WIN client(u32 hwnd,i32* point){++clients;if(hwnd!=0x1234||point[0]||point[1])++bad_identity;point[0]=-19;point[1]=33;return 1;}
static i32 WIN blt(void* self,const i32* d,void* s,const i32* r,u32 flags,void* fx){
 (void)r;(void)flags;(void)fx;++blts;if(self!=surface||s!=surface)++bad_identity;
 for(u32 i=0;i<4;++i)last_rect[i]=d[i];return 0;
}
static i32 WIN fast(void* self,i32 x,i32 y,void* s,const i32* r,u32 flags){
 (void)self;(void)x;(void)y;(void)s;(void)r;(void)flags;++fasts;return 0;
}
static void unchanged(void){const u8 expected[]={0xff,0x15,0xfc,0x51,0x5c,0};
 for(u32 i=0;i<10;++i)for(u32 j=0;j<6;++j)if(*(const u8*)(sites[i]+j)!=expected[j])ExitProcess(181);
}
void start(void){
 /* Reserve original code/global addresses inside this test EXE's BSS, as in
  * the JPEG production fixture. Copy only the script-verified wrapper window;
  * the probe's own running code and imports stay intact. */
 reserved_game_globals[sizeof(reserved_game_globals)-1]=1;
 HANDLE input=CreateFileA("image.bin",0x80000000,1,0,3,0x80,0);u32 n=0;
 if(input==(HANDLE)-1)ExitProcess(182);
 u32 old;
 if(!VirtualProtect((void*)0x58b000,0x3000,0x40,&old)||SetFilePointer(input,0x18bf40,0,0)!=0x18bf40||
    !ReadFile(input,(void*)0x58bf40,0x11a0,&n,0)||n!=0x11a0){ExitProcess(184);}
 CloseHandle(input);unchanged();
 *(u32*)0x5c51fc=(u32)client;
 surface_table[5]=(u32)blt;surface_table[7]=(u32)fast;surface[0]=(u32)surface_table;
 window_table[4]=(u32)get_window;window[0]=(u32)window_table;
 source[1]=source[2]=target[1]=target[2]=(u32)surface;source[4]=target[4]=8;source[5]=target[5]=6;
 *(u32*)0x6f68d8=0;*(u32*)0x6f98d8=0;*(u32*)0x6f98c0=(u32)target;*(u32*)0x6f98d0=(u32)window;
 for(u32 phase=0;phase<3;++phase){
  if(!SetEnvironmentVariableA("MNM_DC_COPY_TAKEOVER",phase==0?0:phase==1?"0":"1"))ExitProcess(185);
  void* module=LoadLibraryA("MnmDcCopy.dll");if((phase<2&&!module)||(phase==2&&module))ExitProcess(186);unchanged();
  for(u32 i=0;i<10;++i){
   u32* row=results[phase*10+i];i32 rect[4]={1,1,4,3},dest[4]={1,2,6,4};clients=blts=fasts=bad_identity=0;
   for(u32 j=0;j<4;++j)last_rect[j]=0;
   typedef void (__attribute__((thiscall)) *Whole)(void*,void*,i32,i32);
   typedef void (__attribute__((thiscall)) *Rectangle)(void*,void*,void*,i32,i32);
   typedef void (__attribute__((thiscall)) *Explicit)(void*,void*,void*,void*);
   u32 before,after;__asm__ volatile("movl %%esp,%0":"=r"(before));
   if(i<2)((Whole)entries[i])(source,target,1,2);
   else if(i==7)((Explicit)entries[i])(source,target,rect,dest);
   else ((Rectangle)entries[i])(source,target,rect,1,2);
   __asm__ volatile("movl %%esp,%0":"=r"(after));
   row[0]=phase;row[1]=entries[i];row[2]=clients;row[3]=blts;row[4]=fasts;row[5]=bad_identity;row[6]=after-before;
   for(u32 j=0;j<4;++j)row[7+j]=(u32)last_rect[j];row[11]=*(u32*)0x5c51fc==(u32)client;
   for(u32 j=0;j<4;++j)row[12+j]=(u32)rect[j];
  }
  if(module&&!FreeLibrary(module))ExitProcess(187);unchanged();
 }
 HANDLE f=CreateFileA("retirement.bin",0x40000000,0,0,1,0x80,0);
 if(f==(HANDLE)-1||!WriteFile(f,results,sizeof(results),&n,0)||n!=sizeof(results))ExitProcess(188);CloseHandle(f);ExitProcess(0);
}
