/* Test-owned PE32 debugger. Hardware breakpoints leave game instructions intact.
 * Only CPUID.1 EAX bits 8..11 may change, before the original helper reads them.
 * Dispatch pointers, caller state, drawing inputs and route counters are read-only.
 */
#include "win32_debug_min.h"
#include "raster_cpu_pinned.h"

struct Proof {
    char magic[8];
    DWORD version, size, complete, error, child_exit, pid, tid, image_base;
    DWORD family, entry_hits, cpuid_hits, return_hits, caller, object;
    DWORD before[4], after[4], targets[4], override_value, object_family;
    DWORD code_checks, first_breakpoints, exceptions, reserved[31];
};
_Static_assert(sizeof(struct Proof) == 256, "CPU proof extent");
static struct Proof proof = {{'M','N','M','C','P','U','0','1'}, 1, sizeof(struct Proof)};
static HANDLE output, process, thread;
struct RgbSample { DWORD sequence, bottom, top, stack, flags, right, skip, tid; };
struct RgbProof {
    char magic[8];
    DWORD version, size, complete, error, child_exit, pid, tid, calls, empty_calls;
    DWORD samples, site, capacity, reserved[50];
    struct RgbSample sample[8];
};
_Static_assert(sizeof(struct RgbProof) == 512, "RGB proof extent");
static struct RgbProof rgb = {{'M','N','M','R','G','B','0','1'}, 1, sizeof(struct RgbProof)};
static HANDLE rgb_output;
static int rgb_enabled;

static void save_rgb(void) {
    if (!rgb_enabled) return;
    DWORD written;
    if (SetFilePointer(rgb_output, 0, 0, FILE_BEGIN) != 0 ||
        !WriteFile(rgb_output, &rgb, sizeof(rgb), &written, 0) || written != sizeof(rgb) ||
        !FlushFileBuffers(rgb_output)) ExitProcess(103);
}

static void save(void) {
    DWORD written;
    if (SetFilePointer(output, 0, 0, FILE_BEGIN) != 0 ||
        !WriteFile(output, &proof, sizeof(proof), &written, 0) || written != sizeof(proof) ||
        !FlushFileBuffers(output)) ExitProcess(91);
}
static void fail(DWORD code) {
    proof.error = code; save();
    if (rgb_output && rgb_output != INVALID_HANDLE_VALUE) { rgb.error = code; save_rgb(); }
    /* DEBUG_ONLY_THIS_PROCESS: debugger exit terminates only its owned child. */
    ExitProcess(code);
}
static void read_child(DWORD address, void* destination, DWORD bytes) {
    SIZE_T actual;
    if (!ReadProcessMemory(process, (void*)address, destination, bytes, &actual) || actual != bytes) fail(92);
}
static void check_code(void) {
    unsigned char code[sizeof(raster_cpu_pinned)];
    read_child(0x484470, code, sizeof(code));
    for (unsigned i = 0; i < sizeof(code); ++i) if (code[i] != raster_cpu_pinned[i]) fail(93);
    ++proof.code_checks;
}
static CONTEXT context(void) {
    CONTEXT c = {0};
    c.ContextFlags = CONTEXT_CONTROL | CONTEXT_INTEGER | CONTEXT_DEBUG_REGISTERS;
    if (!GetThreadContext(thread, &c)) fail(94);
    return c;
}
static void set_context(CONTEXT* c) {
    c->Dr6 = 0;
    /* Resume over an instruction hardware breakpoint without retriggering it. */
    c->EFlags |= 0x10000;
    if (!SetThreadContext(thread, c)) fail(95);
}

void mainCRTStartup(void) {
    char family[4];
    const DWORD n = GetEnvironmentVariableA("MNM_RASTER_CPU_FAMILY", family, sizeof(family));
    if (n == 1 && family[0] == '6') proof.family = 6;
    else if (n == 2 && family[0] == '1' && family[1] == '5') proof.family = 15;
    else ExitProcess(80);
    output = CreateFileA("raster-cpu-debug.bin", GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, 0);
    if (output == INVALID_HANDLE_VALUE) ExitProcess(81);
    save();
    char trace[2];
    rgb_enabled = GetEnvironmentVariableA("MNM_RASTER_RGB_TRACE", trace, sizeof(trace)) == 1 && trace[0] == '1';
    if (rgb_enabled) {
        rgb_output = CreateFileA("raster-rgb-callers.bin", GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, 0);
        if (rgb_output == INVALID_HANDLE_VALUE) fail(102);
        rgb.site = 0x545ce9; rgb.capacity = 8; save_rgb();
    }
    char* command = GetCommandLineA();
    if (*command == '"') { ++command; while (*command && *command != '"') ++command; if (*command) ++command; }
    else while (*command && *command != ' ' && *command != '\t') ++command;
    while (*command == ' ' || *command == '\t') ++command;
    if (!*command) fail(82);
    STARTUPINFOA startup = {0}; PROCESS_INFORMATION child = {0}; startup.cb = sizeof(startup);
    if (!CreateProcessA(0, command, 0, 0, FALSE, DEBUG_ONLY_THIS_PROCESS, 0, 0, &startup, &child)) fail(83);
    process = child.hProcess; thread = child.hThread;
    proof.pid = child.dwProcessId; proof.tid = child.dwThreadId;
    rgb.pid = proof.pid; rgb.tid = proof.tid;
    DWORD events = 0;
    for (;;) {
        DEBUG_EVENT event;
        if (!WaitForDebugEvent(&event, 600000) || ++events > 16384 || event.dwProcessId != proof.pid) fail(84);
        DWORD status = DBG_CONTINUE;
        if (event.dwDebugEventCode == CREATE_PROCESS_DEBUG_EVENT) {
            proof.image_base = (DWORD)event.u.CreateProcessInfo.lpBaseOfImage;
            if (proof.image_base != 0x400000 || event.dwThreadId != proof.tid) fail(85);
            if (event.u.CreateProcessInfo.hFile) CloseHandle(event.u.CreateProcessInfo.hFile);
            CONTEXT c = context();
            if (c.Dr0 || c.Dr1 || c.Dr2 || c.Dr3 || (c.Dr7 & 255)) fail(86);
            c.Dr0 = 0x484470; c.Dr1 = 0x484604; c.Dr2 = 0x48458a; c.Dr7 = 0x15;
            if (rgb_enabled) { c.Dr3 = rgb.site; c.Dr7 |= 0x40; }
            set_context(&c);
        } else if (event.dwDebugEventCode == LOAD_DLL_DEBUG_EVENT) {
            if (event.u.LoadDll.hFile) CloseHandle(event.u.LoadDll.hFile);
        } else if (event.dwDebugEventCode == CREATE_THREAD_DEBUG_EVENT) {
            CloseHandle(event.u.CreateThread.hThread);
        } else if (event.dwDebugEventCode == EXCEPTION_DEBUG_EVENT) {
            const DWORD code = event.u.Exception.ExceptionRecord.ExceptionCode;
            const DWORD at = (DWORD)event.u.Exception.ExceptionRecord.ExceptionAddress;
            if (rgb_enabled && code == EXCEPTION_SINGLE_STEP && event.dwThreadId == proof.tid && at == rgb.site) {
                CONTEXT c = context();
                if (c.Eip != at || !(c.Dr6 & 8)) fail(104);
                DWORD geometry[3];
                if (c.Esp > 0xffffffffu-0x34) fail(105);
                read_child(c.Esp+0x28, geometry, sizeof(geometry));
                ++rgb.calls;
                const i32 skip = (i32)geometry[0], right = (i32)geometry[2];
                if (c.Eax <= 2048 && c.Ebx <= 2048 && c.Eax > c.Ebx &&
                    skip >= 0 && skip <= 2048 && right <= skip && right >= -2048 && right <= 2048) {
                    ++rgb.empty_calls;
                    if (rgb.samples < 8) rgb.sample[rgb.samples++] = (struct RgbSample){
                        rgb.calls, c.Eax, c.Ebx, c.Esp, c.EFlags, (DWORD)right, (DWORD)skip, event.dwThreadId};
                }
                /* Read-only scalar observation; no game-side I/O, pixels,
                 * source descriptors, dispatch or prepared-input writes. */
                set_context(&c);
            } else if (code == EXCEPTION_SINGLE_STEP && event.dwThreadId == proof.tid &&
                (at == 0x484470 || at == 0x484604 || at == 0x48458a)) {
                CONTEXT c = context();
                if (c.Eip != at) fail(87);
                check_code();
                if (at == 0x484470) {
                    if (proof.entry_hits++ || proof.cpuid_hits || proof.return_hits) fail(96);
                    proof.object = c.Ecx; read_child(c.Esp, &proof.caller, 4);
                    c.Dr0 = 0; c.Dr7 &= ~3u;
                } else if (at == 0x484604) {
                    if (proof.entry_hits != 1 || proof.cpuid_hits++ || proof.return_hits) fail(97);
                    proof.before[0] = c.Eax; proof.before[1] = c.Ebx;
                    proof.before[2] = c.Ecx; proof.before[3] = c.Edx;
                    c.Eax = (c.Eax & ~0xf00u) | (proof.family << 8);
                    proof.after[0] = c.Eax; proof.after[1] = c.Ebx;
                    proof.after[2] = c.Ecx; proof.after[3] = c.Edx;
                    c.Dr1 = 0; c.Dr7 &= ~12u;
                } else {
                    if (proof.entry_hits != 1 || proof.cpuid_hits != 1 || proof.return_hits++) fail(98);
                    DWORD caller; read_child(c.Esp, &caller, 4);
                    if (caller != proof.caller) fail(99);
                    const DWORD slots[4] = {0x6c4e3c, 0x689b7c, 0x689b78, 0x6903dc};
                    for (unsigned i = 0; i < 4; ++i) read_child(slots[i], proof.targets + i, 4);
                    read_child(0x6e1f38, &proof.override_value, 4);
                    read_child(proof.object + 0x2c, &proof.object_family, 4);
                    if (proof.object_family != proof.family) fail(100);
                    c.Dr2 = 0; c.Dr7 &= ~48u;
                }
                set_context(&c); save();
            } else if (code == EXCEPTION_BREAKPOINT && event.u.Exception.dwFirstChance && !proof.first_breakpoints++) {
                /* The one ordinary loader debugger breakpoint. */
            } else {
                ++proof.exceptions; status = DBG_EXCEPTION_NOT_HANDLED;
                if (!event.u.Exception.dwFirstChance) fail(101);
            }
        } else if (event.dwDebugEventCode == EXIT_PROCESS_DEBUG_EVENT) {
            proof.child_exit = event.u.ExitProcess.dwExitCode;
            proof.complete = !proof.child_exit && proof.entry_hits == 1 && proof.cpuid_hits == 1 && proof.return_hits == 1;
            save();
            rgb.child_exit = proof.child_exit; rgb.complete = proof.complete; save_rgb();
            if (!ContinueDebugEvent(event.dwProcessId, event.dwThreadId, status)) fail(88);
            CloseHandle(thread); CloseHandle(process); CloseHandle(output);
            if (rgb_enabled) CloseHandle(rgb_output);
            ExitProcess(proof.complete ? 0 : 89);
        }
        if (!ContinueDebugEvent(event.dwProcessId, event.dwThreadId, status)) fail(88);
    }
}
