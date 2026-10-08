# Complete native World raster queues

The bounded experiment replaces every admitted outer raster in a selected
complete World queue, then expands to contiguous startup queues1..16. Original
World traversal and simulation remain active. Native destinations are reconstructed
from startup/HUD source operations and native completions only; captured original
pixels are external comparisons.

## Recovered caller contract

Confirmed static evidence in the hash-pinned nocd build:
`5003cc`/`500515` compareAX with1 for clipped fallback, `500848` storesAX into
queue node+0x1e, and wrapper `57e940` also selects fallback usingAX while
`57e95d` masks it to16bits. Other blend returns are masked/combined. The earlier
bounded incoming-EAX bypass remains historical and does not prove this contract.

| Backend | Admitted original entries | Low16AX | Argument cleanup |
| --- | --- | --- | --- |
|0 black|595677|1 if nonzero dimensions and left<clip.left or right>=clip.right, otherwise0|cdecl|
|1 indexed copy|5947b2,59521a|0; internal clipping|cdecl|
|2 indexed copy, four-byte palette stride|595b47,59603e|0; internal clipping|cdecl|
|3 clipped fallback|57de00|0 in sampled domain|cdecl|
|4 half blend|57ec90|0 in sampled domain|RET12|
|5 three-quarter blend|57f0f0|0 in sampled domain|RET12|
|6 quarter blend|57f5f0|0 in sampled domain|RET12|
|7 wave|5806f0|0 in sampled domain|RET8|
|9 destination displacement|596490,5968a4|same horizontal refusal rule as black|cdecl|
|10 shadow|57e540|0 in sampled domain|RET4|

The [first caller inventory](native-world-raster-callers-20261008.json) sampled
no backend9. The [additional inventory](native-world-raster-callers-wave9-20261008.json)
adds observed return5005d7 and its two backend9 entries. Generated
`runtime/scene/world_raster_callers.h` carries tag masks and ten-byte return
signatures; unknown or mismatched callers terminate the selected experiment.
Neither inventory establishes an entire function or every indirect route.
Backend8 and normal wrapper57e1b0 are not admitted for suppression inside
selected World consumers. HUD work outside those consumers forwards normally.

Wave5806f0 lazily prepares table5f14d0 before its dimension test. The adapter
retains that original **non-drawing** preparation through the relocated routine
on a zero-size owned frame when needed. The preparation count is separate from
native raster count. Effective wave/displacement offsets are captured as owned
source tables. Destination displacement uses table656640 and horizontalAX1
refusal; it is distinct from the wave wrapper's clipped triangular offsets.

The adapter returns prescribed low16AX, restores incoming GPR/EFLAGS/FX and
LastError, and performs each original callee's stack cleanup. This is the narrow
reviewed caller contract, not equality of all volatile original output registers,
highEAX, flags, or raster scratch globals. The caller inventory records reviewed
mask/store/overwrite paths; unobserved consumers remain gaps.

## Validation boundaries

[ABI evidence](native-world-raster-abi-20261008.json) passes all49 hook forwards
and all13 admitted synthetic body skips. It checks AX, stack cleanup, original
body counters, prescribed saved state and malformed reply refusals. Synthetic
compatible prologues deliberately bypass the live caller whitelist; this is not
original raster pixel comparison.

The frozen preflight uses an original-active source capture, canonical entries
per backend, and the reconstructed native before-canvas. The exact comparator
maps the unmodified original executable privately, reconstructs its effective
palette/row tables, and executes each precise routine. Live validation must use
actual request entry addresses rather than canonical preflight entries. Every
native before-image must equal the previous native reply in its queue, with the
first before-image equal to the native queue entry. Each reply is then compared
over its entire canvas and low16AX, providing an intermediate-state comparison
chain. All16 final World canvases also receive independent original replay.

The [V2 wire contract](../formats/world-raster-bypass-v2.md) describes selected
queues, correlation, strict admission and incremental append-only stream reading.
Unknown entry/caller/backend, malformed or timed-out replies, wrong geometry and
incomplete queue ranges refuse. Post-bypass observations contain native work;
they alone cannot establish original equivalence. Native CPU/GPU checks and the
private original comparisons remain independent.

The [preflight](native-world-raster-preflight-20261008.json) matches all1817
canonical queue16 native canvases /872160000pixels and every low16AX result.
All16 original World returns /7680000pixels and1062 observed completions
/474236912pixels also match. Its125declared dependencies remained stable;
76normalized compiled dependencies are all declared. Four CTest cases pass.

Live queue16 completed1521actual native raster writebacks, with all1062
observed checkpoints matching. Its independent exact-entry comparison also
matched1521native canvases /730080000pixels andAX results, with stable sources
and2927original files unchanged. See the [queue16 replacement](native-world-raster-queue16-replacement-20261008.json).

The [first prefix attempt](native-world-raster-prefix-boundary-refusal-20261008.json)
completed1807rasters in queue1, then refused HUD tag36/596cb8 (raster backend8,
mode0) outside the World consumer. The selection incorrectly used the last queue
ordinal after its return. This backend number is not numeric World queue kind8;
the refusal did not emit its caller VA.

The adapter now opens suppression at actual World entry and closes it after
actual World return. [Boundary ABI evidence](native-world-raster-abi-boundary-20261008.json)
exercises the paired callbacks and following HUD raster: original-body count
increases, bypass count stays fixed. Its13body skips and49forwards pass. The
[current preflight](native-world-raster-preflight-boundary-20261008.json) replays
all1817canonical intermediates,AX results,1062checkpoints and16final World
canvases with zero differences after this fix. Earlier source fingerprints and
[fixture/source-drift diagnostics](native-world-raster-boundary-fixture-diagnostics-20261008.json)
remain unchanged. The current complete-prefix retry and independent comparison
both pass: queues 1–16 suppress 36,556 original raster bodies, and every
native intermediate canvas and low16AX matches its actual original entry over
17,546,880,000 pixels. All 16 independently replayed final World canvases
(7,680,000 pixels) and all 1,062 observed checkpoints (474,236,912 pixels) also
match. Every native before-image equals the preceding native reply in its queue;
the first equals the reconstructed native World entry. No original oracle pixels
serve as native inputs. There is no real-time performance claim. Original traversal,
menu/HUD raster work outside selected queues, palette preparation, lazy table preparation, driver/error
behavior, unsampled raster/global paths and native simulation remain separate
milestones. Historical evidence/source fingerprints remain unchanged.

## Complete contiguous prefix result

The immutable [live record](native-world-raster-prefix16-boundary-live-20261008.json),
[exact comparison](native-world-raster-prefix16-boundary-comparison-20261008.json)
and [replacement summary](native-world-raster-prefix16-boundary-replacement-20261008.json)
retain the prospective behavior/scenario claims, stable 125-source closure,
unmodified original executable hash, input fingerprints and queue returns.
All four focused native CTests pass. The live binary matches the reviewed frozen
preflight binary; its 76 normalized compiler dependencies are declared.
Original manifests verify all 2,927 files before and after execution/comparison.
The earlier [queue-1 comparison](native-world-raster-prefix-early-queue1-20261008.json)
remains separate historical evidence from the same capture.

| Actual original entry | Draw kind | Native body replacements |
| --- | --- | ---: |
| `0x595677` | Black | 5,415 |
| `0x57de00` | Clipped fallback | 1,862 |
| `0x57f5f0` | Quarter blend | 20 |
| `0x57f0f0` | Three-quarter blend | 20 |
| `0x57ec90` | Half blend | 20 |
| `0x5806f0` | Wave | 16 |
| `0x59603e` | Indexed copy, four-byte palette stride | 27,450 |
| `0x596490` | Destination displacement | 1,710 |
| `0x59521a` | Indexed copy | 35 |
| `0x57e540` | Shadow | 8 |

The exact comparisons include 1,937 AX=1 refusals
and 34,619 AX=0 returns. Every recorded raster inside
the selected queues has a corresponding native reply; the stream retains
13,741 observed raster requests outside World,
where suppression is closed and original work continues. Exactly one original
non-drawing lazy wave-table preparation occurred; it is counted separately.
The original World dispatcher and object traversal remain active.

The admitted entries `0x5947b2`, `0x595b47`, `0x5968a4`
were not exercised by live suppression in this prefix. Their synthetic ABI
admission and separate canonical preflight evidence do not establish actual-entry
live replacement here. All 1,710 destination-displacement
requests avoid vertical top clipping. The reconstructed table phase for vertical
top clipping remains unverified; no broader clipping claim follows from this
result. Unknown callers, formats, indirect routes, error paths, volatile output
registers/flags and original scratch-global effects remain gaps. Understanding
stays partial while implementation and live replacement are scoped to this
executed domain.

The committed range `63f32b1..e94f344` was reviewed separately in
[committed-history accounting](coverage/committed-history-world-raster-prefix-20261008.json):
436 file/link transitions and 138 behavior transitions have exact receipt chains;
prior receipt contents are preserved, with no unresolved exact transitions.
This historical accounting does not assert a past gate pass or new execution.
