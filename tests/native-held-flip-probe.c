/* Same caller, independent native/real Wine driver processes. */
#include "../runtime/shadow/win32_min.h"
API void* WIN LoadLibraryA(const char*);
API void* WIN GetProcAddress(void*,const char*);
API void* WIN GetDesktopWindow(void);
API void WIN Sleep(u32);
typedef i32 (WIN *Create)(void*,void**,void*);
typedef i32 (WIN *Lock)(void*,const void*,u32*,u32,void*);
static void** table(void* p){return *(void***)p;}
static void require(int ok,u32 code){if(!ok)ExitProcess(code);}
static void desc(u32* d){for(u32 i=0;i<27;++i)d[i]=0;d[0]=108;}
static void release(void* o){((u32 (WIN *)(void*))table(o)[2])(o);}
static i32 unlock(void* o){return ((i32 (WIN *)(void*,void*))table(o)[32])(o,0);}
static i32 flip(void* o,u32 flags){return ((i32 (WIN *)(void*,void*,u32))table(o)[11])(o,0,flags);}
static i32 release_dc(void* o,void* dc){return ((i32 (WIN *)(void*,void*))table(o)[26])(o,dc);}
void start(void){
 char flag[4];int native=GetEnvironmentVariableA("MNM_FACTORY_NATIVE",flag,4)!=0;
 void* module=LoadLibraryA(native?"MnmDraw.dll":"ddraw.dll");require(module!=0,101);
 Create create=(Create)GetProcAddress(module,"DirectDrawCreate");u32 rows[4][12];
 void* user=LoadLibraryA("user32.dll");
 typedef void* (WIN *Window)(u32,const char*,const char*,u32,i32,i32,i32,i32,void*,void*,void*,void*);
 typedef int (WIN *Foreground)(void*);typedef void* (WIN *GetForeground)(void);
 typedef int (WIN *Position)(void*,void*,i32,i32,i32,i32,u32);
 typedef int (WIN *Cursor)(i32,i32);typedef void (WIN *Mouse)(u32,u32,u32,u32,u32);
 Window window=(Window)GetProcAddress(user,"CreateWindowExA");
 Foreground foreground=(Foreground)GetProcAddress(user,"SetForegroundWindow");
 GetForeground get_foreground=(GetForeground)GetProcAddress(user,"GetForegroundWindow");
 Position position=(Position)GetProcAddress(user,"SetWindowPos");Cursor cursor=(Cursor)GetProcAddress(user,"SetCursorPos");Mouse mouse=(Mouse)GetProcAddress(user,"mouse_event");
 void* hwnd=window(0,"STATIC","held lease comparison",0x10cf0000,0,0,640,480,0,0,0,0);require(hwnd!=0,114);
 for(u32 i=0;i<40&&get_foreground()!=hwnd;++i){position(hwnd,(void*)-1,0,0,640,480,0x40);cursor(320,240);mouse(2,0,0,0,0);mouse(4,0,0,0,0);foreground(hwnd);Sleep(50);}
 require(get_foreground()==hwnd,115);
 for(u32 held=0;held<4;++held){
  void *draw=0,*front=0,*back=0,*dc=0;u32 d[27],caps=4;
  require(create && !create(0,&draw,0),102);
  require(!((i32 (WIN *)(void*,void*,u32))table(draw)[20])(draw,hwnd,0x11),103);
  require(!((i32 (WIN *)(void*,u32,u32,u32))table(draw)[21])(draw,640,480,16),104);
  desc(d);d[1]=0x21;d[5]=1;d[26]=0x218;
  require(!((i32 (WIN *)(void*,u32*,void**,void*))table(draw)[6])(draw,d,&front,0),105);
  static const u8 surface2[16]={0x85,0x58,0x80,0x57,0xec,0x6e,0xcf,0x11,0x94,0x41,0xa8,0x23,3,0xc1,0x0e,0x27};
  void* alias=0;require(!((i32 (WIN *)(void*,const void*,void**))table(front)[0])(front,surface2,&alias),113);release(front);front=alias;
  require(!((i32 (WIN *)(void*,u32*,void**))table(front)[12])(front,&caps,&back),106);
  for(u32 side=0;side<2;++side){void* s=side?back:front;desc(d);require(!((Lock)table(s)[25])(s,0,d,1,0),107);
   for(u32 y=0;y<480;++y)for(u32 x=0;x<640;++x)((u16*)(d[9]+y*d[4]))[x]=(u16)(side?0x5678:0x1234);
   require(!unlock(s),108);}
  void* operand=held&1?back:front;void* other=held&1?front:back;
  for(u32 i=0;i<12;++i)rows[held][i]=0;
  desc(d);rows[held][0]=held<2?((Lock)table(operand)[25])(operand,0,d,1,0):((i32 (WIN *)(void*,void**))table(operand)[17])(operand,&dc);
  require(!rows[held][0],109);
  rows[held][1]=flip(front,0);
  rows[held][2]=held<2?unlock(operand):release_dc(operand,dc);
  rows[held][3]=held<2?unlock(other):release_dc(other,dc);
  if(held>=2){rows[held][4]=flip(front,1);rows[held][5]=release_dc(operand,dc);if(rows[held][5]){u32 done=0;HANDLE file=CreateFileA("driver-failed-rows.bin",0x40000000,0,0,1,0x80,0);WriteFile(file,rows,sizeof(rows),&done,0);CloseHandle(file);ExitProcess(110);}}
  for(u32 side=0;side<2;++side){void* s=side?back:front;desc(d);rows[held][6+side*3]=((Lock)table(s)[25])(s,0,d,1,0);require(!rows[held][6+side*3],111);
   rows[held][7+side*3]=*(u16*)d[9];rows[held][8+side*3]=unlock(s);}
  release(back);release(front);release(draw);
 }
 u32 done=0;HANDLE file=CreateFileA("held-flips.bin",0x40000000,0,0,1,0x80,0);
 require(file!=(HANDLE)-1 && WriteFile(file,rows,sizeof(rows),&done,0) && done==sizeof(rows),112);CloseHandle(file);ExitProcess(0);
}
