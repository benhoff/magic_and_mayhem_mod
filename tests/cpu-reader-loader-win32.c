#include "../runtime/shadow/win32_min.h"
API void* WIN LoadLibraryA(const char*);
API void* WIN GetProcAddress(void*,const char*);
API int WIN FreeLibrary(void*);
API int WIN VirtualFree(void*,u32,u32);
typedef u32 (WIN *Probe)(void);

void start(void){
 void* reserved=VirtualAlloc((void*)0x10000000,65536,0x2000,1);
 if(reserved!=(void*)0x10000000)ExitProcess(10);
 u8* module=LoadLibraryA("HeaderFixture.dll");
 if(!module || module==(void*)0x10000000)ExitProcess(11);
 Probe probe=(Probe)GetProcAddress(module,"HeaderProbe");
 if(!probe || probe()!=0x12345678)ExitProcess(12);
 u32 pe=*(u32*)(module+0x3c),bytes=*(u32*)(module+pe+84);
 if(pe>4096 || !bytes || bytes>4096)ExitProcess(13);
 u32 header[8]={0x4c4d4e4d,0x31305244,1,(u32)module,pe,bytes,0x10000000,0};
 HANDLE file=CreateFileA("mapped-header.bin",0x40000000,0,0,1,0x80,0);
 if(file==(void*)-1)ExitProcess(14);
 u32 wrote;
 if(!WriteFile(file,header,sizeof(header),&wrote,0) || wrote!=sizeof(header) ||
    !WriteFile(file,module,bytes,&wrote,0) || wrote!=bytes)ExitProcess(15);
 if(!CloseHandle(file) || !FreeLibrary(module) || !VirtualFree(reserved,0,0x8000))ExitProcess(16);
 ExitProcess(0);
}
