# Original canvas driver observation journal

The opt-in scene observer writes `canvas-driver.bin` beside
`canvas-driver-clear.565`. This is diagnostic output, not a recovered game file.
All integers are little-endian DWORDs. Implementation:
`runtime/scene/canvas_driver_observe.h`; strict reader:
`tools/canvas_driver_observation.py`.

The 64-byte header has magic `MNMCDB01`, version 1, row size 128, pinned-build
tag 0x40209ca7 and row limit 4096; its remaining DWORDs are zero. Each 128-byte
row contains 32 DWORDs:

| Index | Meaning |
| --- | --- |
| 0–4 | Sequence, event, wrapper, original caller/return site, engine thread ID. |
| 5–10 | Surface1, Surface2, their canonical IUnknown identities and two QI HRESULTs. Identity queries apply to creation rows. |
| 11–14 | Actual forwarded initial Blt HRESULT, GetBltStatus result, added read Lock result and matching Unlock result. |
| 15–21 | Width, height, byte pitch, bit depth and red/green/blue masks. |
| 22–25 | Nonzero words, unexpected-colour words, FNV-1a of cropped pixel bytes, byte count. Terminal event uses index25 for read-lease count. |
| 26 | Lease/read/unlock flags 1/2/4; all three are required for the successful clear. |
| 27–31 | Observed wrapper creation counter, alternate global0x6e1f68, fault code, initial Blt count, reserved zero. |

Events are 10 creation, 20 actual initial Blt return, 30 initial clear read
lease, 40 selected wrapper release and 99 first-World terminal. The strict
successful cohort requires exactly one creation/Blt/lease on one engine thread,
stable aliases, zero HRESULTs, complete RGB565 dimensions/masks/pixels, zero
faults and terminal completion. A missing terminal, release before World,
incomplete read or malformed journal refuses the observation.

Pixel rows are cropped to `width * 2` bytes, omitting driver pitch padding. Reads
occur only inside the successful added read-only WAIT driver lease. Legacy
readable-pointer snapshots are disabled in this mode. The creation counter
identifies an observed wrapper creation; it does not identify a physical backing
allocation. Added QI and Lock/Unlock calls are explicit diagnostic intervention.
See [execution and remaining boundaries](../runtime/critical-original-recovery-20261020.md).
