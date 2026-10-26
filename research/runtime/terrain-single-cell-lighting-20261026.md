# Size-one terrain lighting recovery and capture comparison

Size-one lighting already had an implementation and isolated original comparison
under `RE.terrain-single-cell-light`. This batch adds current-code validation of
its complete authored capture/update path and corrects the older
`TL.special-size-one` entry in the central and two historical focused registers.
The native implementation and wire bytes are unchanged. No original heap default
or natural live size-one caller is established.

The owned branch starts at `c5137b9c`. The [history review](size-one-lighting-results-20261026/history01.json)
checks 71 exact first-parent file transitions across that merge and its incoming
branch since the preceding `dfff4634` master review. All have prior exact
receipts. This accounting grants no historical gate pass or new engine evidence.

## Recovered behavior

In the pinned No-CD build, original initializer `004ecdd0` calls kernel builder
`004f1890`. The special kernel pointer at map+6381 points to map+6380; its extent
byte at map+6405 is one. Initialization and repeated initialization leave the
signed byte at map+6380 unchanged. It is inherited object storage, rather than
a distance falloff computed from AmbientLight or LightRamp.

Maximum stamping at `004f2140` uses this byte in work/base fields. Static stamping
at `004f1c60` applies signed saturated addition to prior/staging. Its two axial
diagonal writes alias the same cell for a size-one stamp; both additions occur.
This matters for inherited negative bytes, repeated sources and saturation.

The existing owned `TerrainLightingConfig.singleCell` explicitly defaults to
zero and admits -128..127. That deterministic default is native policy; it is
not an assertion that original allocation zeroed this location. The production
capture serializer reads map+6380 and sign-extends it into V1 header DWORD+56.
The native V1 decoder forwards that signed value to the owned field. No pointer
crosses the wire.

The unchanged original `0049cf60` lookup selects index one from an authored
effect-type table. The original outer update `004f27a0` then exercises admission,
`004f2ad0` static phases, additive stamping and publication, finally clearing the
shared changed flag. The native decoder executes the corresponding owned cycle.

## Current comparison

[SIZE1.original-20261026](size-one-lighting-results-20261026/compare04.json)
binds the final prospective contracts and every declared source:

| Boundary | Cases and result |
| --- | --- |
| Existing initializer/maximum/additive component | All 256 inherited bytes; 1,536 stamps; 2,122,295 complete buffer bytes per variant |
| Authored capture and original outer/static updater | 2,048 initializations; 8,192 requests; 19,978,240 buffer bytes |
| Production V1 decoder versus original reply | All 8,192 complete replies match, including 20,633,600 header/state/buffer bytes per native variant |
| Signed-byte admission | Six checksum-valid noncanonical values refused by every native variant |

The direct component compares original PE32, optimized native32/native64 and
ASan/UBSan native32/native64. The capture oracle independently executes unchanged
PE32 instructions with executable sections protected RX. Actual production C
capture runs with authored synchronous HeapAlloc/HeapFree/VirtualQuery endpoints
over exact fixture regions. No original instruction or supplied file is patched.
The oracle runs no native model to produce expected state or pixels. Native
variants receive only captured source bytes; original replies are comparison
data and never become decoder inputs.

Each inherited byte runs eight transitions in each paired dimension/configuration
case: (1,1,1,-127,-127), (2,3,4,-50,7), (17,19,5,0,-16) and
(19,17,6,127,127), where the last two values are ambient/ramp. This is a paired
matrix, not every dimension/configuration cross product. The direct component
retains its ambient=-50/ramp=7 contract. Fixtures include duplicate sources,
inactive and zero-light records, different recount/position coordinates,
disabled refresh with a pending changed flag, zero scan count, wrapped admission,
changed values 1/2/-1, edge layers and two-phase publication. All five buffers,
reply state words, source identity and buffer/object guards are checked.

The decoder refuses -129,128,255,256,INT_MIN and INT_MAX after checksums are
recomputed. Thus a malformed positive 255 cannot silently become inherited -1.
All 256 canonical signed values pass. Both original-manifest bookends verify
all 2,927 immutable files, and source/input fingerprints remain stable.

Two initial harness compiler failures remain in compare01/compare02 with exact
sources, diagnostics and partial outputs. The narrower successful compare03
retains its own fixed-config contract. Compare04 uses memcpy for unaligned
fixture metadata reads and records four paired configurations under a new
prospective domain. No earlier payload or hash is refreshed.

All generated reports, source variants, logs, input/oracle bundles and executed
binaries are SQLite-backed references. RAM build/input/output files are temporary
interchange; citable unique bytes are verified in SQLite before the mount ends.
The [retained local audit inputs](size-one-lighting-results-20261026/local-inputs01.json)
supply exact historical research text and bounded fixtures without republishing
or changing engine validation. Normal local completion retains verified SQLite
review/evidence backups.

## Remaining boundary and reproduction

`RE.terrain-single-cell-captured-cycle` is scoped implementation and original
comparison. Its integration and replacement remain none. `TL.special-size-one`
now records the remaining installed map allocation/construction/writer provenance
and naturally executed index-one source use; its statuses remain unchanged.
The two historical focused packages receive only that same gap correction.
Other package rows, source hashes and evidence are preserved. The new rendering
contract remains unrated in planning; the planning fixture acknowledges its ID
without changing importance weights.

Authored table entries, preallocated fields and readable-region callbacks do not
establish Windows API/allocator equivalence, arbitrary maps/saves, actual source
creation, a full scene, scheduling ownership or live replacement. Confidence is
high within the explicit compared domain; installed-state provenance remains
provisional. This batch does not introduce gameplay balance changes.

```sh
python3 /absolute/main/tools/working-retention.py run --reserve-gib 1 -- \
  python3 -B /absolute/task/tools/test-terrain-single-cell-lighting.py \
  --main-root /absolute/main --output /owned-ram/new-run \
  --report /absolute/task/research/runtime/size-one-lighting-NEW/compare.json
```

Use a bounded RAM execution directory and a new immutable report path. The
runner pins the executable, verifies original manifests before/after, declares
source/contract fingerprints before execution and persists failures in SQLite.
