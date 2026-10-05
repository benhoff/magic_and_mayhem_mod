# Installed ground-creature profile and native scene (NS09)

Scope: selected installed Redcap movement configuration, ground ANI samples,
normal-memory ANI binding and bounded ordinary MAP presentation. This advances
NS08's synthetic profile to one configured creature. It does not initialize a
complete original creature, replace live simulation, load runtime entities,
implement flying/swimming actions, or establish original whole-scene equivalence.

## Recovered configuration and samples

Pinned No-CD SHA-256 is
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
The original fastcall loader `0x00502b20` receives creature/HTH profile paths in
ECX/EDX and the 28-record output table as its stack argument. Each type record is
`0x5c9` bytes. The isolated test supplies seven selected profile values; all
other creature fields and HTH matrix reads are absent. This tests the original
selected-field conversions, not complete installed type initialization.

| Installed `CREATURE_10` field | Runtime offset | Conversion/value |
| --- | --- | --- |
| TileHeight | +0x0c | Clamp 1..5; 3 |
| TileSizeXY | +0x08 | Clamp 1..2; 1 |
| Acceleration | +0x10 | Nonnegative; 10 |
| CanFly | +0x3c | Case-insensitive TRUE; false |
| SwimmingAbility | +0x44 | Signed integer; 1 |
| GroundSpeed | +0x5bd | Clamp 0..1000; 4 |
| FlyingSpeed | +0x5c1 | Clamp 0..1000; 0 |

The native asset reader retains raw values and requires every selected field, complete signed decimal
integers and TRUE/FALSE Boolean text. These are deliberate refusal policies;
the recovered clamps live in the reconstruction layer, and original missing/partial-token/overflow behavior is not generalized from the
admitted fixture domain. Original clamp branches are compared independently.
GroundSpeed/FlyingSpeed are retained as configuration values. They do not replace
the sample-driven scalar calculation in this native movement slice.

The unmodified original `0x00505220` derives the two ground parity banks from
ANI sequences 0 and 1. It finds the first sprite and the last sprite before the
stop, including the repeated first sprite after event 2. For normalized record
DWORD +8 values `first` and `last`, take their absolute difference, scale by six
for facing 0 or three for facing 1, form twelve cumulative integer twelfths, then
subtract preceding cumulative values. It does not copy each frame's displacement.
The native bounded reader verifies all eight ground programs contain twelve
sprites, event 2, one repeated first sprite and a terminal stop; unsupported
controls, zero displacement totals and displacement above 4096 reject.

Installed Redcap yields twelve **8** values (sum 96), then alternating **7,8**
(sum 90). Non-flying records leave both flying banks zero. `0x00505160` takes the
maximum target over admitted category/delta combinations. Its ground target
includes descending `3/2` and ascending `2/3` scaling; the installed maximum is
**235**, not the maximum unadjusted base scalar 157. That maximum is copied to
frozen generator +0x589; unused runtime fields are not invented as recovered data.

The original ordinary segment branches in `0x00510e80` select ground sequence
`facing` (base 0), while category-specific and configured attachment paths remain
separate. `0x005057b0` with type 10 and the normal-memory flag returns
`creatures\redcap.ani`; this selector executes unchanged in the reference test.
The ANI header names `redcap.spr`. Native admission selects only this normal
Redcap binding, with case-insensitive asset resolution and exact input hashes.
Camera-relative action production and the low-memory asset path remain outside
this milestone; four native views retain the existing diagnostic facing policy.

## Native integration and zero samples

`mnm-map-navigation-export` accepts an optional creature type **10**, then optional
start XYZ, target XYZ and tick count. It decodes the encrypted installed creature
CFG through the existing owned codec, decodes the selected ANI, builds the exact
profile and optionally publishes a v6 `.mnms` checkpoint with owned ANI bytes.
Callers no longer provide a numeric sequence base for this configured path.

Configured admission is one cell wide, height 1..5, no flying, SwimmingAbility 1,
acceleration 0..1000000 and sample values at most 192. Standing positions and
original predicate tests use the selected height, width, swimming and type index.
All source-world geometry is still prepared before cropping. Projected runtime
references/objects, the sealed perimeter, navigation/visual identity checks and
bounded ordinary categories 0/4 remain NS08 policies. Default scalar 720, slope
0.969, crop boundary and budget remain explicit inputs; their live provenance is
not claimed. Native simulation consumes navigation and owned animation bindings,
without knowing CFG keys, original type offsets or Qt.

The installed diagonal sequence consumes a **zero sample at the event boundary**
before event 2 resets its cursor. The former positive-only sample admission
rejected this real program. The motion model now allows 0..192 per sample,
retaining finite iteration/cursor bounds and transactional state updates. Zero
advances the controller while leaving displacement unchanged. Negative samples
still refuse before committing state. No unsupported animation event is looped
or reinterpreted. Installed movement/controller comparisons include these zeros.

## Validation and remaining boundaries

The reproducible runner is `tools/test-creature-navigation.py BUILD_DIRECTORY`.
It pins the original PE, verifies original manifests before/after, executes the
original selected-field loader and sample builder in private memory, and compares
installed inputs, terrain predicates, routes, independent complete pixels and
fresh-process restoration. The original loader/sample/selector instructions are
not redirected. Only the movement completion dependency at `0x00512460` is
byte-checked and redirected in disposable private memory; complete route
consumption/world-loop side effects are not part of that comparison.

The reference covers six selected-field profiles, 28 output rows and **1,176
field comparisons**, plus ground sample/maximum agreement for all five non-flying
profiles. It compares **7,968 installed ANI/fine-motion/controller transitions**
across eight facings, categories 0/4, authored planar/vertical inputs, height
deltas -32/0/32 and three rates. Installed configuration bytes are independently
decoded by the Python oracle. Focused native tests cover missing/malformed fields,
unsupported Boolean/ANI controls, sample construction, ownership and bounds.
Only accepted final run records advance the milestone; diagnostic failures are
retained in disposable working output and do not count as validation.

Both accepted normal and ASan/UBSan runs compare 16,896 original support/validity
predicates and 5,434 movement-helper results across six crops. Each run also
passes 672 independent complete-frame comparisons and 24 fresh-process
continuations. All 100 normal CTests and five focused sanitizer CTests pass.
All 2,927 immutable original files and the input executable remain unchanged.

Commands are linked from the app README; accepted run paths, hashes and bounded
results are preserved in the [NS09 machine-readable record](native-creature-profile.json). Dynamic occupancy, multiple creatures, original orders,
complete action selection, original creature construction, camera convention,
thread/pause/cadence, flying/swimming profiles and live replacement remain open.
