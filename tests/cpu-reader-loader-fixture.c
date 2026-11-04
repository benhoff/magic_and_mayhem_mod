#include "../runtime/shadow/win32_min.h"

static volatile u32 value=0x12345678;
static volatile u32* volatile relocated_pointer=&value;
u32 WIN HeaderProbe(void){return *relocated_pointer;}
int WIN DllMain(void* module,u32 reason,void* reserved){
 (void)module;(void)reason;(void)reserved;return 1;
}
