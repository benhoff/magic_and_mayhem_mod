# CPU-context debugger proof, version 1

Test-owned PE32 observation protocol, not an original game format. The debugger
creates `raster-cpu-debug.bin` with CREATE_NEW in its owned game/fixture directory.
The file is 256 bytes: `MNMCPU01`, followed by 62 little-endian DWORDs. Its C
writer asserts the extent; the host decodes explicit scalar words. Guest
addresses are observations, not host pointers or shared object layouts.

| Physical DWORD | Meaning |
| --- | --- |
| 0..1 | `MNMCPU01` |
| 2..3 | Version 1, extent 256 |
| 4..6 | Complete, error, actual child exit code |
| 7..10 | Observed PID/TID/image base, authored raw CPU family |
| 11..13 | Initializer entry / post-CPUID / initializer return hit counts |
| 14..15 | Observed original caller return PC / object address |
| 16..19 | Actual CPUID.1 EAX/EBX/ECX/EDX |
| 20..23 | Authored CPUID.1 EAX/EBX/ECX/EDX |
| 24..27 | Original selected targets at `6c4e3c`, `689b7c`, `689b78`, `6903dc` |
| 28..29 | Original override at `6e1f38`, original object raw family |
| 30..32 | Pinned-code checks, loader breakpoints, other first-chance exceptions |
| 33..63 | Reserved zero |

Current admission requires complete=1/error=0/child-exit=0, base `0x400000`, one
hit per observation, three exact checks of `[0x484470,0x4846a1)`, and the one
ordinary loader breakpoint. Only EAX bits 8..11 may differ in the four CPUID
registers; authored families are 6 or 15. Original target values must match its
family/override selection and observed object family. The original entry and
return must expose the same caller. Hardware breakpoints are cleared after
their observations. The helper never calls WriteProcessMemory.

The writer updates/flushes the bounded file at observations and actual child
exit. Readers consume it after owned process completion/private Wine shutdown;
this is not a concurrent publication channel. Missing, partial, reserved-data,
unfinished or failed proofs refuse success. Host SHA-256 binds the final artifact.
Failed native-presentation acquisitions may leave a valid partial observation
without the final child-exit proof; they do not satisfy complete admission.

See [the caller experiment](../runtime/native-pixel-cpu-callers.md) for authored
platform, frozen-source and original-pixel boundaries.
