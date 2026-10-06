# Original opaque RGB565 overlap

2026-10-06. Confirmed within the captured Wine driver scope; this is not Windows
hardware or live replacement evidence.

The unchanged no-CD wrapper `0x58c360` (build SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`) executes
in a private PE mapping. A forwarding destination observer records its real
Blt/BltFast call and HRESULT. Both interfaces reference one actual 8×6 RGB565
system-memory backing. Source Surface2 pointer equality and a separately queried
Surface1 interface are checked independently using IUnknown identity. Full native
pixels, descriptors, absent key state and clip lists are captured before/after.
No original instruction bytes are changed and no native renderer supplies outputs.

[Capture](original-surface-overlap-capture-20261006.json) retains 1,536 calls:
632 successes, 224 invalid rectangles, 120 missing clip lists and 560
BltFast-cannot-clip results. There are 768 distinct-interface alias cases.
WAIT/no-WAIT, opaque patterned/unique pixels, nonoverlap/touching, identical/full
rectangles, four directions, diagonals, one-pixel, invalid/empty rectangles and
six clip states are covered. All source/destination snapshots agree for shared
storage; alias and direct-pointer cases obey the same observed rules.

[Offline CPU comparison](original-surface-overlap-cpu-20261006.json) derives
admission, ordered clipped pieces and HRESULTs from inputs. Each admitted piece
reads a frozen snapshot taken after preceding pieces have completed. Sixteen
errors retain preceding writes. Ordered vertically aligned regions deliberately
make earlier writes affect a later source: a single snapshot for the whole call
fails eight distinguishing cases. The existing distinct-surface clipping/error
planner agrees with this captured self-copy geometry and error order.

Run entirely offline:

```sh
python3 tests/test-original-surface-overlap.py
python3 tools/check-original-surface-overlap.py --report working/tests/overlap-new.json
```

Capture reproduction requires supported working Chaos.exe, Wine and Xvfb:

```sh
xvfb-run -a python3 tools/capture-original-surface-overlap.py --report working/tests/overlap-capture-new.json --fixture-dir working/tests/overlap-fixtures-new
```

The collector refuses retained-output overwrite and concurrent Wine sessions,
uses a copied prefix, and verifies the original manifest before and after. The
initial exploratory runs remains under `working/tests`; final evidence
uses the expanded distinguishing cases. These are independent driver-backed
wrapper observations, not a full game launch.

The [native GPU comparison](original-surface-overlap-native-20261006.json)
checks all 1,536 backend HRESULTs and 3,072 source/destination full pixel arrays,
then all 1,536 v3 replay HRESULTs and shared pixel arrays in six bounded batches.
Ordinary backend operations perform zero CPU uploads/native or RGBA readbacks.
Twenty lifetime/admission checks and six poisoned/unsupported streams pass.

`GlBlitter::surfaceCopy` accepts opaque RGB565 same-ID calls and snapshots each
ordered source piece with `glCopyTexSubImage2D` into a transient R32UI texture.
Drawing samples that texture, then releases it on success or exceptions. Maximum
scratch size is 2048×2048 words (16MiB), independent of the64 user surface handle
budget. Errors retain earlier writes and return the input-derived planner result.
The v3 opcode17 admits identical logical IDs only with zero/API WAIT flags.
Expected outputs/results appear only in diagnostics. Legacy opcode3 and keyed/
masked self-copy remain refused. Separate verified COM aliases normalize to one
logical backing ID for replay; this does not implement live COM pointer tracking.

Reproduce the native comparison offline after building the renderer:

```sh
cmake --build working/build/renderer
xvfb-run -a python3 tools/check-native-surface-overlap.py --report working/tests/overlap-native-new.json
```

Native support is scoped to these captured opaque RGB565 operations. Keyed/masked overlap,
other formats, COM pointer tracking in a live adapter, lost surfaces, Restore and
retry wrappers remain outstanding. Native logical IDs will represent verified
shared backing; COM pointers are not part of the offline wire contract.
