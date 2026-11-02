/* A registered callback's return value never substitutes for physical authority. */
#define start resource_probe_unused
#include "native-draw-resource-probe.c"
#undef start
typedef i32 (WIN *History)(const void*,u32,u32,u32,u32,u32);
static void* expected_object;
static void* expected_pixels;
static i32 reader_result;
static i32 WIN reader(void* object,const void* pixels,u32 family,u32 route,u32 caller){
 require(object==expected_object && pixels==expected_pixels && !family && !route && !caller,11);
 return reader_result;
}
void start(void){
 void* module=LoadLibraryA("MnmDraw.dll");require(module!=0,12);
 Create create=(Create)GetProcAddress(module,"NativeDrawCreate");
 History history=(History)GetProcAddress(module,"NativeDrawCpuHistoryV1");
 i32 (WIN *register_reader)(void*)=(void*)GetProcAddress(module,"NativeDrawCpuReaderV1");
 void* draw=0;u32 d[31];require(create && history && register_reader && !create(0,&draw,0),13);
 desc(d,2);require(!((Surface)table(draw)[6])(draw,d,&expected_object,0),14);
 desc(d,2);require(!((Lock)table(expected_object)[25])(expected_object,0,d,1,0),15);
 expected_pixels=(void*)d[9];require(!history(expected_pixels,1,2,0,5,6),16);
 require(!((i32(WIN*)(void*,void*))table(expected_object)[32])(expected_object,expected_pixels),17);
 char mode[2];require(GetEnvironmentVariableA("MNM_READER_RESULT",mode,2)==1,18);
 if(mode[0]!='n'){reader_result=mode[0]=='1';require(!register_reader((void*)reader),19);}
 history(expected_pixels,0,0,0,0,0);
 ExitProcess(20); /* The stale reader must terminate first, even after return1. */
}
