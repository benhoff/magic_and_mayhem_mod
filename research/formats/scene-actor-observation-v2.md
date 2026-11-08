# Scene actor observation v2

Version 2 keeps the 128-byte generation header and 80-byte matched-draw rows
from [version 1](scene-actor-observation-v1.md), with stream/terminal magic
`MNMSAO02`/`MNMSAOE2` and version DWORD 2. Actor rows are 800 bytes. The
256-actor, 4096-draw, 8192-generation and 256 MiB limits remain unchanged.

The first original-runtime probe exposed a v1 collector bug: SPR index-table
entries are offsets added to SPR-object `+12`, not absolute pointers. The
original helpers at `0x5071de..0x5071eb` and `0x507352..0x50735f`, and selected
draw callers, perform that addition. v1's absent roles earn no attachment
placement credit. Their actual ANI/journal/generation observations remain
retained with the original source bytes. v2 checks addition overflow and
readability after resolving the relative entry; it does not call a helper.

Each actor's first 16 DWORDs retain the v1 state fields. Words 16–27 are twelve
frame-local role tokens, 28–159 twelve raw 44-byte ANI records, 160–171 SPR
inclusive bounds, 172–183 effective frame indices, and 184–195 observed gate
values. Words 196–199 are zero. The role-valid mask at word 12 has twelve bits.
At most 3072 tokens exist in one source generation. The original resource
pointer and frame-data base are never serialized.

| Role | Display / asset pointer fields | Gate observation |
| --- | --- | --- |
| 0 | Actor `+b4` / `+cc` | Body candidate |
| 1 | Actor `+7d9` / `+7f1` | Raw mode 1 or 3 |
| 2 | Actor `+829` / `+841` | Overlay candidate; caller admission separate |
| 3 | Actor `+130` / `+148` | Actor `+124`, nonzero health |
| 4 | Actor `+85d` / `+875` | Actor `+889`, nonzero health |
| 5 | Actor `+891` / `+8a9` | Actor `+8bd` |
| 6 | Globals `6df150` / `6df168` | Actor `+8d1`, nonzero health |
| 7 | Globals `6df060` / `6df078` | Actor `+780`, nonzero health |
| 8 | Globals `6df090` / `6df0a8` | Actor `+8d5`, nonzero health |
| 9 | Globals `6df0f0` / `6df108` | Actor `+963`, nonzero health |
| 10 | Globals `6df0c0` / `6df0d8` | Actor byte `+962`, nonzero health |
| 11 | Globals `6df120` / `6df138` | Actor `+78c`, nonzero health |

These are numeric producer roles, not inferred spell/equipment names. The
selected static caller range is `0x4fa9d0..0x4fb25f`. Shared global controllers
may match several actors; matches are candidates and cannot establish ownership
without the raw gate and composition checks. Other callers and role-2 overlay
admission remain separate. Mode 3 preserves the distinction between a
footprint-two second attachment point and the other caller's half-SPR-height
adjustment. Role 5's selected branch has no health gate.

The header's actor-size DWORD distinguishes layouts. The reader retains v1
support and refuses unknown versions, role bits, malformed counts or terminal
loss. New v2 observations never reinterpret historical v1 bytes.


Generation header words20/21 preserve queue-object `+16/+4` as opaque raw
32-bit auxiliary metadata. Retained live values are address-like, including
36-byte increments; their precise meanings are not needed or admitted here.
They are never converted to native pointers, dereferenced, used as actor
ownership, or assumed stable across launches. The v2 header's “No pointer
values cross this contract” comment is too broad. Frame-local role tokens and
normalized frame identities exclude their original pointers; the complete
private observation file also retains these opaque original queue words. This
document and the corrected behavior scope state that boundary explicitly
without changing or refreshing the captured native/protocol source hashes.
