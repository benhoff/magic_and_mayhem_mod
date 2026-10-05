# Bounded installed MAP navigation and creature presentation (NS08)

Reviewed 2026-10-05. This is an offline ordinary-terrain integration milestone.
It does not implement complete original map loading, configured creature/entity
admission, original whole-search equivalence, live observation or replacement.

## Admission and projection

`apps/world-scene/map_navigation.*` consumes the native version-6 MAP loader and
version-4 TTD catalog. It applies existing `prepareTerrainGeometry` to the entire
source before cropping, preserving source-neighbor geometry at the crop boundary.
The geometry projection removes all three reference WORDs and object/flag-path
bits as previously documented; it deliberately omits their runtime entities.

The crop includes all source layers. XY extents must be 4..16, layers 2..32, with
at most 4,096 cells. Its outer XY ring receives flags10 `0x4000` on every layer.
This is a native sealed-boundary policy: a cropped region must not gain the
original full map's toroidal connections. The ring is also omitted from drawing.
Coordinates are crop-local; the report retains source origin and realm. Retained
header metadata is zeroed rather than guessed or rebound to this smaller world.

The exporter writes a native frozen `MNMWLD01` input using the exact projected
12-byte cells and complete 356-byte TTD records. Row/layer offsets are canonical
cell offsets, with inert pointer tokens for existing reconstruction adapters.
An **explicit synthetic one-cell, one-layer ordinary creature profile** is used:
width/height 1, scalar acceleration 500, alternating direction banks 60/40,
generator/default scalar 720, boundary 1 and slope global 0.969. This profile is
controlled test input; it is not derived from installed creature configuration.
Object state and special movement controls are zeroed. The standing-coordinate
list uses recovered support and validity predicates and ordinary height bounds
-16..16; this list is not a claim of reachability between every pair.

The `.geometry` output is a version-6 decoded MAP **payload**, not an original
packed/encrypted MAP file. The `.frozen` file feeds the existing headless movement
adapter. A JSON report records source hashes, crop, omitted source references
and object flags, sealed cell count, standing coordinates and output hashes.
Output paths must be new. Each file is independently published; interruption
can leave an incomplete bundle, and consumers must validate admission.

## Matched presentation

`mnm-world-scene-preview --terrain-map FILE.geometry` loads the owned visual
payload and refuses unless dimensions, canonical row/layer tables, every cell
byte and every TTD record agree with its checkpoint's frozen navigation.
The frozen bytes are checked against the already-admitted navigation fingerprint,
so a subsequent file change is refused. No movement tick or frame output is
performed before these checks. A TTD from another realm is not silently accepted.

The scene now submits actual admitted definitions/flags from all crop layers,
including the existing terrain producer's body and optional additional frames,
and interleaves the owned ANI body display in the recovered signed queue.
Clipping, SPR origins and body offsets retain existing contracts. Diagnostic
cell-centred projection and fixed camera remain native policies. No visibility,
lighting, camera-relative action selection or entity-linked attachments are
introduced. The old diagnostic fixture remains available with its existing
options. Simulation remains independent of Qt and asset/reconstruction types.

## Validation and scope

`tests/map-navigation-test.cpp` checks expected supported cells on a known
synthetic floor, sealed-ring accounting, source immutability, canonical frozen
tables/tokens, valid MAP-payload serialization, and refusal of wrong dimensions,
cell flags, TTD records, changed frozen fingerprints, invalid definitions and
out-of-bounds crops.

`tools/test-map-navigation.py BUILD_DIRECTORY` is manifest-guarded and uses
installed Plains/Forest/Village section 01, with 8x8 crops at (0,0) and (4,4).
It independently transcribes ordinary source geometry and cropping, compares the
complete geometry/frozen cell payloads and TTD bytes, then executes isolated
original support `004f3320`, validity `004f3440` and complete movement helper
`004f3990` with its preserved lower terrain dependencies. The PE is SHA-256-pinned
to `40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Functions are unchanged in a private mapping; controlled map/global receiver
bindings and the explicit profile are supplied by the harness. References and
terrain bytes must remain unchanged. This establishes predicate agreement on
these projected inputs, not full game load/search or original scene agreement.

Each crop selects an ordinary category-0/4 route, reaches arrival headlessly,
and keeps all positions inside the sealed perimeter. Four views compare full
RGB565 output using an independent SPR decoder/compositor and headless fine
positions. Split/restart processes must reproduce queue reports, RGB565/PNG
frames and final v6 checkpoint bytes. A changed visual cell must refuse before
creating any frame/checkpoint output. ANI is a deterministic owned test program
with explicit frame IDs paired with installed RedCap SPR; complete installed
action configuration remains outside the scope.

Confidence is high for these bounded native policies and isolated predicate
comparisons. Static-object collision/admission, actual creature configuration,
full-world topology, unsupported movement/action categories, dynamic occupancy,
multi-creature scheduling, lighting integration and live agreement remain open.

## Recorded evidence

Both normal and ASan/UBSan builds pass six crops: 8,448 cells, **16,896 original
support/validity comparisons** and **5,434 complete movement-helper comparisons**
(1,050 accepted and 4,384 rejected). No move into the sealed perimeter is
accepted. Geometry/frozen bytes match independent projection/cropping, and all
selected native routes arrive with positions inside the crop. Each build passes
**288 full-frame pixel comparisons** and **24 fresh-process continuations**,
including visual-cell mismatch refusal before output. All 98 normal CTests
pass; the new map-navigation, scene and terrain-motion continuation tests pass
under ASan/UBSan (three selected CTests). Before/after checks preserve all 2,927
original files, and report hashes confirm the source PE is unchanged.

- Normal report: `working/tests/map-navigation/run-r432173z/report.json`.
- Sanitizer report and build/test logs: `working/tests/map-navigation/run-p0cltru4/`.
- [Machine-readable evidence and source/report hashes](native-map-navigation.json).

Build from `apps/world-scene` to `working/build/world-map-navigation`, then run
`ctest --test-dir working/build/world-map-navigation --output-on-failure` and
`python3 tools/test-map-navigation.py working/build/world-map-navigation`.
The sanitizer build is `working/build/world-map-navigation-sanitized`, with
`-fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie` and linker
`-fsanitize=address,undefined -no-pie`,
`ASAN_OPTIONS=detect_leaks=0:halt_on_error=1` and
`UBSAN_OPTIONS=halt_on_error=1`. Qt/OpenGL caches are excluded from leak checks.
Local Xvfb and isolated PE32 mapping need execution outside the restricted
sandbox. The initial reference compile included an unrelated replay CLI and
failed linking; the corrected model-only build produced the accepted reports.
No failed run is counted as evidence of predicate or scene agreement.

Runnable export and scene examples are in
[the app README](../../apps/world-scene/README.md#installed-map-crops).

## Installed ANI supplementary smoke check

Installed `Creatures/redcap.ani` numeric base **0**, explicitly paired with
`Creatures/RedCap.spr`, reaches (4,6,3) from (1,1,3) on the Plains crop at tick
27. A 100-tick headless trace split at tick 40 reproduces all 61 resumed states
and the final v6 checkpoint bytes. The exported arrival frame was manually
inspected and shows the creature on the actual terrain. This supplies no original
named-action/config mapping or original mixed-scene claim. The before/after
original manifests each preserve 2,927 files.

The prior example's numeric base 8 is a terminating program: a longer route
produced an unsupported animation-stop event. It was refused, not silently
looped or reinterpreted. The runnable examples now select base 0, whose event-2
program is compatible with this forward motion slice. Other stopping/gameplay
animation events remain outside the admitted driver.

Supplementary record: [installed ANI evidence](native-map-navigation-installed-ani.json).
Artifacts are `working/tests/map-navigation/run-r432173z/installed-base0-*`.
Reproduce with the app README's base-0 checkpoint, `trace START 100`,
`resume START TICK40 40`, `trace TICK40 60`, and direct/resumed `resume` commands
ending at tick 100; compare traces from tick 40 and complete checkpoint bytes.
The demo uses `--ticks 100 --frames 1 --output NEW_PREFIX` with the matched
`.geometry` payload. All output paths must be unused.
