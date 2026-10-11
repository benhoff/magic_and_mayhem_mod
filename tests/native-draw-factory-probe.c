/* The same unchanged caller probe runs against Wine DirectDraw and the native
 * factory. Pointer results are compared as identity relations, never addresses. */
#include "../runtime/shadow/win32_min.h"
API void* WIN LoadLibraryA(const char*);
API void* WIN GetProcAddress(void*,const char*);
typedef i32 (WIN *Create)(void*,void**,void*);
typedef i32 (WIN *Query)(void*,const u8*,void**);
typedef u32 (WIN *Ref)(void*);
static const u8 dd2[16]={0xe0,0xf3,0xa6,0xb3,0x43,0x2b,0xcf,0x11,0xa2,0xde,0,0xaa,0,0xb9,0x33,0x56};
static const u8 unknown[16]={0,0,0,0,0,0,0,0,0xc0,0,0,0,0,0,0,0x46};
static const u8 unsupported[16]={0x12,0x34,0x56,0x78};
static u32 records[128][18],enumerations;
static void** table(void* p){return *(void***)p;}
static i32 WIN enumerated(void* guid,const char* description,const char* name,void* context) {
  if(guid || !description || !name || context!=(void*)0x1234)ExitProcess(101);
  ++enumerations;return 1;
}
void start(void) {
  char value[2];u32 native=GetEnvironmentVariableA("MNM_FACTORY_NATIVE",value,2)!=0;
  void* module=LoadLibraryA(native?"MnmDraw.dll":"ddraw.dll");if(!module)ExitProcess(102);
  Create create=(Create)GetProcAddress(module,native?"NativeDrawCreate":"DirectDrawCreate");if(!create)ExitProcess(103);
  for(u32 i=0;i<128;++i){
    u32* r=records[i];void* draw=0;SetLastError(0x51);
    r[0]=(u32)create(0,&draw,0);r[1]=draw!=0;r[2]=GetLastError();if(r[0] || !draw)ExitProcess(104);
    void* second=0;r[3]=(u32)((Query)table(draw)[0])(draw,dd2,&second);r[4]=second!=0;if(r[3]||!second)ExitProcess(105);
    void* identity=0;r[5]=(u32)((Query)table(second)[0])(second,unknown,&identity);void* base_identity=0;if(((Query)table(draw)[0])(draw,unknown,&base_identity))ExitProcess(110);r[6]=identity==base_identity;
    void* denied=(void*)0x1234;r[7]=(u32)((Query)table(draw)[0])(draw,unsupported,&denied);r[8]=denied==0;
    void* aggregate=(void*)0x1234;r[16]=(u32)create(0,&aggregate,draw);r[17]=aggregate==0;
    r[9]=((Ref)table(draw)[1])(draw);
    r[10]=((Ref)table(identity)[2])(identity);((Ref)table(base_identity)[2])(base_identity);r[11]=((Ref)table(second)[2])(second);
    r[12]=((Ref)table(draw)[2])(draw);r[13]=((Ref)table(draw)[2])(draw);
    if(native){typedef u32 (WIN *Count)(void);Count count=(Count)GetProcAddress(module,"NativeDrawObjects");if(!count)ExitProcess(106);r[14]=count();}
  }
  if(native){
    if(create(0,0,0)!=(i32)0x80070057u)ExitProcess(111);
    if(GetModuleHandleA("ddraw.dll") || LoadLibraryA("ddraw.dll"))ExitProcess(107);
    typedef i32 (WIN *Enum)(void*,void*);
    Enum enumerate=(Enum)GetProcAddress(module,"NativeDrawEnumerateA");
    if(!enumerate || enumerate((void*)&enumerated,(void*)0x1234) || enumerations!=1)ExitProcess(108);
  }
  HANDLE out=CreateFileA("factory-results.bin",0x40000000,0,0,2,0x80,0);u32 n=0;
  if(out==(HANDLE)-1 || !WriteFile(out,records,sizeof(records),&n,0) || n!=sizeof(records))ExitProcess(109);
  CloseHandle(out);ExitProcess(0);
}
