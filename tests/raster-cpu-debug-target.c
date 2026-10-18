/* Executes the unchanged original initializer through an ordinary caller in a
 * PE32 process, to validate the debugger before any actual game acquisition. */
#include "../runtime/diagnostics/win32_debug_min.h"
#include "raster_cpu_pinned.h"
static DWORD object[32];
/* Reserve the fixed-address fixture region as part of this PE's own .bss,
 * before Wine can assign those addresses to its loader or heaps. */
static volatile unsigned char address_space[0x300000];
void mainCRTStartup(void) {
    address_space[sizeof(address_space)-1] = 1;
    DWORD old;
    if (!VirtualProtect((void*)0x484000, 4096, PAGE_EXECUTE_READWRITE, &old)) ExitProcess(61);
    unsigned char* code = (void*)0x484470;
    for (unsigned i = 0; i < sizeof(raster_cpu_pinned); ++i) code[i] = raster_cpu_pinned[i];
    char control[32];
    DWORD n = GetEnvironmentVariableA("MNM_RASTER_CPU_NEGATIVE", control, sizeof(control));
    if (n && control[0] == 's') ExitProcess(0);
    if (n && control[0] == 'b') code[0] ^= 1;
    for (unsigned i = 0; i < 32; ++i) object[i] = 0xa5a5a5a5;
    ((void (__attribute__((thiscall)) *)(void*))code)(object);
    char family[4];
    n = GetEnvironmentVariableA("MNM_RASTER_CPU_FAMILY", family, sizeof(family));
    const DWORD f = n == 1 && family[0] == '6' ? 6 : 15;
    if (object[11] != f) ExitProcess(62);
    for (unsigned i = 15; i < 32; ++i) if (object[i] != 0xa5a5a5a5) ExitProcess(63);
    /* Owned RET stub and prepared stack, not the original RGB routine. This
     * validates observer counts/geometry without manufacturing game callers. */
    if (!VirtualProtect((void*)0x545000, 4096, PAGE_EXECUTE_READWRITE, &old)) ExitProcess(64);
    *(volatile u8*)0x545ce9 = 0xc3;
    __asm__ volatile(
        "subl $0x38, %%esp\n"
        "movl $0, 0x24(%%esp)\n"
        "movl $1, 0x2c(%%esp)\n"
        "movl $3, %%eax\nmovl $1, %%ebx\nmovl $0x545ce9, %%edx\ncall *%%edx\n"
        "movl $0, 0x2c(%%esp)\ncall *%%edx\n"
        "addl $0x38, %%esp\n" ::: "eax", "ebx", "edx", "memory", "cc");
    ExitProcess(0);
}
