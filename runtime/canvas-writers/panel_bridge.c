/* Optional post-Lock panel splice, independently admitted from fade/glyph/World.
 * No diagnostic file I/O, original output seeds or retained live pixel pointers. */
#include "../shadow/win32_min.h"
#include "producer_status.h"
#include "panel_route.h"
API u32 WIN GetCurrentThreadId(void);
API void* WIN GetProcAddress(void*,const char*);
#define MNM_HISTORY_FAMILY 1
#include "history.h"
extern void mnm_panel_bevel_enter(void), mnm_panel_halo_enter(void), mnm_panel_wash_enter(void);
static volatile u32 owner;
static int installed;
static int span(u32 at, u32 bytes, int writable) {
    if (!at || !bytes || at > 0xffffffffu - bytes) return 0;
 if(cpu_history_guard_span((void*)at,bytes))return 1;
    const u32 end = at + bytes;
    while (at < end) {
        u32 m[7];
        if (VirtualQuery((void *)at, m, 28) != 28 || m[4] != 0x1000 ||
            (m[5] & 0x101) || !(m[5] & 0xee) || (writable && !(m[5] & 0xcc)) ||
            m[0] > at || !m[3] || m[0] > 0xffffffffu - m[3] || m[0] + m[3] <= at)
            return 0;
        at = m[0] + m[3];
    }
    return 1;
}
static int resolve(u32 *r, unsigned kind, struct MnmPanelPlane *p, struct MnmPanelRequest *q) {
    u32 error = GetLastError();
    cpu_history_require_route((void*)r[7],kind);
    int ok = 0;
    u32 *stack = r + 9;
    if (kind >= 3 || !span((u32)stack, 0x64, 0)) goto done;
    const u32 object = kind == 0 ? stack[0x18 / 4] : kind == 1 ? stack[0x10 / 4] : r[0];
    if (!span(object, 40, 0)) goto done;
    const u32 *surface = (const u32 *)object;
    p->pixels = (u16 *)r[7]; p->width = surface[4]; p->height = surface[5];
    p->stride = surface[6] / 2; p->format = *(const u32 *)0x6e1f88;
    if (surface[6] & 1 || p->format > 1) goto done;
    q->mode = 15 + kind;
    if (kind == 0) {
        q->left = (i32)stack[0x30 / 4]; q->top = (i32)stack[0x34 / 4];
        /* Prepared wrapper endpoints already subtract the live origin. */
        if ((i32)stack[0x38 / 4] <= 0 || (i32)stack[0x3c / 4] <= 0) goto done;
        q->right = (i32)stack[0x38 / 4] - 1; q->bottom = (i32)stack[0x3c / 4] - 1;
        q->percent = stack[0x58 / 4]; q->pressed = stack[0x5c / 4]; q->reverse = stack[0x60 / 4];
        const u16 *masks = (const u16 *)0x6e1f7c;
        if (masks[0] != (p->format ? 0x7c00 : 0xf800) ||
            masks[1] != (p->format ? 0x3e0 : 0x7e0) || masks[2] != 31) goto done;
    } else {
        const u32 rectangle = stack[(kind == 1 ? 0x4c : 0x38) / 4];
        if (!span(rectangle, 16, 0)) goto done;
        const i32 *rect = (const i32 *)rectangle;
        q->left = rect[0]; q->top = rect[1]; q->right = rect[2]; q->bottom = rect[3];
        q->percent = q->pressed = q->reverse = 0;
    }
    if (!mnm_panel_admit(p, q) || !span((u32)p->pixels, p->stride * p->height * 2, 1)) goto done;
    const u32 thread = GetCurrentThreadId();
    if (!owner) __sync_bool_compare_and_swap(&owner, 0, thread);
    ok = owner == thread;
done:
    SetLastError(error);
    return ok;
}
static int same(const u8 *a, const u8 *b, u32 n) {
    for (u32 i = 0; i < n; ++i) if (a[i] != b[i]) return 0;
    return 1;
}
static void jump(u8 *at, void *target, u32 length) {
    at[0] = 0xe9; *(u32 *)(at + 1) = (u32)target - (u32)at - 5;
    for (u32 i = 5; i < length; ++i) at[i] = 0x90;
}
static const struct Site {
    u32 at, length, tail;
    u8 bytes[10], tail_bytes[8];
    void (*entry)(void);
} sites[3] = {
    {0x58de5a, 10, 0x58e28d, {0x8b,0x54,0x24,0x5c,0x89,0x44,0x24,0x28,0x8b,0xea},
     {0x8b,0x54,0x24,0x18,0x8b,0x42,0x04,0x8b}, mnm_panel_bevel_enter},
    {0x58d92c, 8, 0x58db5f, {0x8b,0x7c,0x24,0x4c,0x89,0x44,0x24,0x30},
     {0x8b,0x54,0x24,0x10,0x50,0x8b,0x4a,0x04}, mnm_panel_halo_enter},
    {0x58e32c, 10, 0x58e40b, {0x8b,0x4c,0x24,0x38,0x8b,0xd8,0x89,0x5c,0x24,0x30},
     {0x8b,0x47,0x04,0x53,0x50,0x8b,0x08,0xff}, mnm_panel_wash_enter}
};
static int install(void) {
    if (installed) return 1;
    char value[4]; const u32 n = GetEnvironmentVariableA("MNM_PANEL_TAKEOVER", value, sizeof(value));
    if (!n) return 1;
    if (n != 1 || value[0] != '1' || (u32)GetModuleHandleA(0) != 0x400000) return 0;
 if(!cpu_history_install())return 0;
 if(!producer_start(0,3))return 0;
    /* Validate every site before mutating any. These are interior hooks; they
     * compose with entry observers and preserve the original surface wrappers. */
    for (u32 k = 0; k < 3; ++k)
        if (!span(sites[k].at, sites[k].length, 0) || !span(sites[k].tail, 8, 0) ||
            !same((const u8 *)sites[k].at, sites[k].bytes, sites[k].length) ||
            !same((const u8 *)sites[k].tail, sites[k].tail_bytes, 8)) return 0;
    u8 *trampoline = VirtualAlloc(0, 4096, 0x3000, 0x40);
    if (!trampoline) return 0;
    for (u32 k = 0; k < 3; ++k) {
        u8 *at = trampoline + k * 32;
        for (u32 i = 0; i < sites[k].length; ++i) at[i] = sites[k].bytes[i];
        jump(at + sites[k].length, (void *)(sites[k].at + sites[k].length), 5);
        mnm_panel_original[k] = (uintptr_t)at; mnm_panel_resume[k] = sites[k].tail;
    }
    u32 old;
    if (!VirtualProtect(trampoline, 4096, 0x20, &old) ||
        !FlushInstructionCache(GetCurrentProcess(), trampoline, 4096)) return 0;
    if (!VirtualProtect((void *)0x58d000, 8192, 0x40, &old)) return 0;
    mnm_panel_resolve = resolve;mnm_panel_notice=producer_notice;
    for (u32 k = 0; k < 3; ++k) jump((u8 *)sites[k].at, (void *)sites[k].entry, sites[k].length);
    u32 ignored;
    if (!VirtualProtect((void *)0x58d000, 8192, old, &ignored) ||
        !FlushInstructionCache(GetCurrentProcess(), (void *)0x58d000, 8192)) ExitProcess(123);
    installed = 1;
    return 1;
}
__declspec(dllexport) void PanelWritersAnchor(void) {}
#ifdef MNM_PANEL_WRITER_SELFTEST
__declspec(dllexport) int WIN PanelWritersInstallForTest(void) { return install(); }
#endif
int WIN DllMain(HANDLE module, u32 reason, void *reserved) {
    (void)module; (void)reserved;
    if (reason != 1) return 1;
#ifdef MNM_PANEL_WRITER_SELFTEST
    return 1;
#else
    const u32 error = GetLastError(); const int result = install(); SetLastError(error); return result;
#endif
}
