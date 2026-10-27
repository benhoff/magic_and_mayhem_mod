/* Exercise the production capture serializer with explicit fixture Win32
 * allocation/readability endpoints. No session, DLL hook or game is launched. */
#define __declspec(x)
#include "../runtime/lighting/observer.c"
#include <stdlib.h>

extern u32 singleCellQuery(u32 address,u32* information,u32 bytes);
HANDLE WIN GetProcessHeap(void){return (HANDLE)1;}
void* WIN HeapAlloc(HANDLE heap,u32 flags,u32 bytes){(void)heap;(void)flags;return malloc(bytes);}
int WIN HeapFree(HANDLE heap,u32 flags,void* p){(void)heap;(void)flags;free(p);return 1;}
u32 WIN VirtualQuery(const void* p,void* information,u32 bytes){
 return singleCellQuery((u32)p,(u32*)information,bytes);
}
u8* singleCellCapture(u32 map,u32 id,u32* length){
 sequence=id;nonce[0]=0x31415926;nonce[1]=0x27182818;
 return capture(map,0x4f27a0,length);
}
void singleCellResult(u8* bytes,u32 map,u32 count){readResult(bytes,map,count);}
void singleCellFree(u8* p){free(p);}
