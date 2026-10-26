/* Ordinary Win32 window management in the explicitly owned private Wine prefix.
 * No original address, injection, dispatch, clip-list API or drawing inputs. */
typedef unsigned int u32;typedef int i32;
#define API __declspec(dllimport)
#define WIN __stdcall
struct Rect {i32 left,top,right,bottom;};struct Point {i32 x,y;};
API void* WIN OpenDesktopA(const char*,u32,i32,u32);
API i32 WIN SetThreadDesktop(void*);
API void* WIN FindWindowA(const char*,const char*);
API i32 WIN IsWindowVisible(void*);
API i32 WIN GetWindowRect(void*,struct Rect*);
API i32 WIN GetClientRect(void*,struct Rect*);
API i32 WIN ClientToScreen(void*,struct Point*);
API u32 WIN GetWindowThreadProcessId(void*,u32*);
API i32 WIN SetWindowPos(void*,void*,i32,i32,i32,i32,u32);
API u32 WIN GetLastError(void);
API void* WIN GetStdHandle(u32);
API i32 WIN WriteFile(void*,const void*,u32,u32*,void*);
API void WIN ExitProcess(u32);
struct Report {char magic[8];u32 result,error,window,pid;struct Rect before,after;struct Point origin_before,origin_after;};
static struct Report report={{'M','N','M','E','X','P','0','1'},1};
typedef char report_size[sizeof(struct Report)==72?1:-1];
void mainCRTStartup(void) {
 void* game=0;void* desktop=OpenDesktopA("MagicMayhem",0,0,0x1ff);struct Rect client;u32 written=0;
 if(!desktop || !SetThreadDesktop(desktop)){report.result=6;goto emit;}
 game=FindWindowA(0,"Magic & Mayhem");
 report.window=(u32)game;
 if(!game || !IsWindowVisible(game) || !GetWindowRect(game,&report.before) || !GetClientRect(game,&client) || !ClientToScreen(game,&report.origin_before))goto emit;
 report.result=2;GetWindowThreadProcessId(game,&report.pid);
 if(!report.pid || client.left || client.top || client.right!=640 || client.bottom!=480 || report.before.right-report.before.left<640 || report.before.right-report.before.left>700)goto emit;
 report.result=3;i32 left=-(report.before.right-report.before.left)/2;
 if(!SetWindowPos(game,0,left,report.before.top,0,0,0x15) || !GetWindowRect(game,&report.after))goto emit;
 report.result=4;
 if(!ClientToScreen(game,&report.origin_after) || report.after.left!=left || report.origin_after.x>=0)goto emit;
 report.result=0;
 emit:report.error=GetLastError();
 if(!WriteFile(GetStdHandle((u32)-11),&report,sizeof(report),&written,0) || written!=sizeof(report))ExitProcess(5);
 ExitProcess(report.result);
}
