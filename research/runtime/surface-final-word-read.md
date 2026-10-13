# Battle pause and original distortion tail read (2026-10-12)

Two separate failures occurred in `menu-observer/run-_k21dqtv`. The retained
[diagnostic record](battle-pause-distortion-20261012.json) embeds raw samples,
disassembly, probe provenance, and the isolated original reproducer. Raw files
are pinned under `working/experiments/battle-progress/run-f96qgva2`.

## Confirmed pause, then resumed simulation

Read-only Wine `ReadProcessMemory` sampling found mode DWORD `0x6db8d2=1`,
world tick `0x6c4830=894` unchanged, and accepted sequence `0x6db8da`
advancing 11825–11880. Scheduler phases also stayed fixed. The next sample
window had mode zero, world ticks 1348–1403 and sequence 12741–12796.
Thus the earlier stopped battle was paused and subsequently resumed. These
are separate non-atomic DWORD reads, not a complete simulation snapshot.

Original update `0x46afc0` gates the world counter at `0x46b496` and creature
passes at `0x46b711` on the mode DWORD. Keyboard handling `0x46c070` maps
Space to scan code `0x39`; branch `0x46d125` chooses pause/resume commands
`0x1a/0x1b`. Dispatcher `0x539080` routes them to `0x46ee90` (mode one) and
`0x46eec0` (mode zero). This explains responsive orders/markers with stationary
creatures. How the first pause was triggered was not captured.

## Subsequent crash

The final sample window had mode zero, tick 3071 and sequence 14595 fixed.
Wine reported a read fault at `0x05316000`, instruction `0x005965f7`, thread
`0x130`. The later native rendering recovery timeout is consistent with the
producer having crashed; it is not evidence of an unsupported render command.

For original build `40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`,
distortion leaf `0x596490` contains `mov edx,[edi+eax*2]` at `0x5965f7`, followed
by a WORD store. Its last-pixel RGB565 sample reads two unused bytes beyond
the logical payload. The isolated fixture supplies a one-pixel mask at
(638,479), offset one, on a 640x480 canvas. A protected page immediately
after the payload produces **the same fault instruction**, with the fault
address equal to the payload end. Providing a readable following page returns
success and copies the expected low WORD. Sandbox runs first failed with
SIGSYS; only the separately retained host runs establish these results.

Confidence is high for this selected original boundary read. The live crash's
source coordinates and allocation base were not captured, so its exact cause
remains a hypothesis supported by the matching instruction and page boundary.
No claim covers arbitrary out-of-range distortion offsets.

## Native allocation policy and validation

`runtime/native-draw/surfaces.h` now requests `pitch*height+2` zeroed bytes.
Pitch, dimensions, logical memory accounting and valid pixel coordinates stay
unchanged. VirtualAlloc rounds the allocation to pages; page-aligned payloads
therefore require another page. The change does not authorize a source pixel
beyond the surface and does not change GPU read-envelope guards.

The surface probe adds four final-pixel volatile DWORD reads: DirectDraw1/4
at 640x480 and 800x640. Both payload sizes end on page boundaries. It checks
the last pixel and zero upper WORD, alongside the existing 64 caller/pixel
comparisons and native surface policy tests. The
[retained result](native-surface-tail-20261012.json) passes all four tail reads,
64 comparison cases with zero mismatched words, and the existing native policy
tests. Original manifests verified all 2927 files before and after; source/input
hashes stayed stable. An earlier attempt refused while another Wine session was
active (`working/tests/native-draw-surfaces/run-mzylzsrq/report.json`).
Live battle validation remains
pending; historical rendering evidence is not refreshed by this change.

To reproduce the isolated original boundary result, extract
`distortion.fixture_source` from the diagnostic JSON into a scratch `.cpp`
file and use its retained `distortion.original_run.build_command`, replacing
only the scratch source/output paths. The include root must be this checkout.
Verify the original manifest and the pinned executable SHA-256 before running
the resulting binary with `working/game-nocd/Chaos.exe tight` (expected exit
42, EIP `005965f7`) and `padded` (expected exit zero and `pixel_matches=1`).
Verify the original manifest afterward. Use the storage retention wrapper;
the Linux harness needs 32-bit execution support and does not patch game files.
