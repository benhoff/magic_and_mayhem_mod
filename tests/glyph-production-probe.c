#include "../runtime/shadow/win32_min.h"
API void* WIN LoadLibraryA(const char*);
API void* WIN GetProcAddress(void*,const char*);
API int WIN SetEnvironmentVariableA(const char*,const char*);
/* Reserve synthetic game-address pages in this test executable's writable BSS,
 * before Wine creates heaps/DLL allocations. No original bytes are included. */
static volatile u8 reserved_game_globals[4*1024*1024];
void start(void){
 reserved_game_globals[sizeof(reserved_game_globals)-1]=1;
 void* module=LoadLibraryA("MnmScene.dll");if(!module)ExitProcess(160);
 char directory[1024];const u32 n=GetEnvironmentVariableA("MNM_TEST_GLYPH_DIR",directory,sizeof(directory));
 if(!n || n>=sizeof(directory) || !SetEnvironmentVariableA("MNM_GLYPH_BACKEND","takeover") ||
  !SetEnvironmentVariableA("MNM_GLYPH_CAPTURE","0") || !SetEnvironmentVariableA("MNM_GLYPH_DIR",directory))ExitProcess(161);
 typedef u32 (WIN *Test)(void);Test test=(Test)GetProcAddress(module,"GlyphProductionForTest");if(!test)ExitProcess(162);
 u32 result=test();if(result)ExitProcess(163+result);ExitProcess(0);
}
