# Bounded RGB-add refusal inputs, version 1

Native diagnostic policy, not an original game file format. Opt-in environment
`MNM_PIXEL_REFUSAL_DIAGNOSTICS=1` retains at most eight CREATE_NEW files named
`pixel-rgb-refusal-0.bin` through `7.bin` in the isolated game directory.
Disabled normally and compiled out of the production-splice self-test fixture.
No pixel bytes, route-counter writes, dispatch changes or admission changes.

Each file is exactly 48 little-endian DWORDs (192 bytes):

| DWORD | Meaning |
| --- | --- |
| 0..2 | `PXR1`, version 1, extent 192 |
| 3 | Refusal: 1 row bounds; 2 plane/extent admission; 3 object span; 4 destination bounds |
| 4..7 | RGB-add kind 1, bounded sample ordinal, observed thread ID, reserved zero |
| 8..30 | Entry saved PUSHAD/flags and 14 already span-checked stack words (`r[0..22]`) |
| 31..35 | Destination pointer, logical width/height, WORD stride, format globals |
| 36..40 | Five source descriptor DWORDs when readable; otherwise zero |
| 41..47 | Reserved zero |

Saved-register order is EDI, ESI, EBP, saved ESP, EBX, EDX, ECX, EAX, EFLAGS;
stack words follow at slot 9. Addresses belong only to the observed process.
The adapter restores its entry LastError normally. Logging failure does not
change admission: fallback still increments the existing forward counter and
fails ordinary zero-forward campaign acceptance. Partial/missing records cannot
establish an input contract. Live acquisition and interpretation remain separate.
Only the four listed geometry/object guards are sampled; initial thread/DF/stack
guards and later kernel refusal can lack a record. Production counters remain
the authority for zero-forward admission.
