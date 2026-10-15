/* Selected CPU readers must use this before native work AND original fallback.
 * Resolve at adapter installation; hot calls do not query the loader. */
typedef i32 (WIN *CpuHistory)(const void*,u32,u32,u32,u32,u32);
static CpuHistory cpu_history;
static int cpu_history_install(void){
 void* module=GetModuleHandleA("MnmDraw.dll");
 if(!module)return 1; /* Independent original-reference splice fixtures. */
 cpu_history=(CpuHistory)GetProcAddress(module,"NativeDrawCpuHistoryV1");
 return cpu_history!=0;
}
static void cpu_history_require_route(const void* pixels,u32 route){if(cpu_history)cpu_history(pixels,0,MNM_HISTORY_FAMILY,route,(u32)__builtin_return_address(0),0);}
#define cpu_history_require(pixels) cpu_history_require_route(pixels,0)
