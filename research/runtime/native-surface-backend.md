# Integrated owned surface backend

## First integration step — 2026-10-06

`SurfaceBackend` owns a GlBlitter and a private entry for each surface ID. Each
entry associates canonical indexed8/RGB565/RGB32 storage, SurfaceAccessState,
BitmapDcState, owned palette/default context, clipper and RGB565 source key.
GlBlitter IDs remain globally unique. Entry lookup and all operations enforce
the renderer GUI thread. No COM pointer, real HDC or wire alias is introduced.

The [base integration result](native-surface-backend-base-20261006.json) compares
2,630 draw HRESULTs and 299,808 native/DC pixel values against retained independent
outputs: 250 original fills, 436 original keyed calls, 408 driver clipping calls,
1,536 original opaque overlap calls, 192 DC cases/576 leases, and 96 access
sequences/1,182 transitions. It also compares 258,048 palette entries and 8,664
stable Lock descriptor fields. The 84 unstable failure caps words remain
excluded. All 105 failed fills and two failed keyed calls now have native result
and unchanged-output checks. Continued key/DC phases keep the same storage ID.
Thirty-nine native guards pass; twelve poisoned access cases explicitly destroy
the entire owner instead of presenting invented pixels or treating individual
borrowed destruction as successful. No new original execution is claimed by
this reuse of historical captures. New evidence preserves historical hashes.

## API and admission

Lock returns the captured-style descriptor with an opaque storage token and
stores a read-only CPU snapshot. Unknown bytes refuse before admission/output
mutation. `writeLocked` updates owned GPU storage and that snapshot together;
its pointer-free native policy is tested synthetically, separately from original
CPU pointer equivalence. Simultaneous CPU/DC writes and DC raster access during
a CPU lease refuse. GetDC may still acquire during a CPU lease, as observed.
Bad DC release tokens preserve the lease. Ordinary read/present/update/destroy
refuse borrowed or poisoned entries. Whole-owner teardown releases all storage.

Indexed DC acquire installs the private palette snapshot and resets GDI regions.
Object binding/updates during a lease defer; direct DC edits affect only that
lease. Successful release reinstalls the explicitly bound/default display table.
Shared COM palette identity and post-release Windows hardware display behavior
remain separate ownership milestones. Missing default context cannot acquire an
unbound indexed DC; default context is input, never a portable inferred formula.

RGB565 fill accepts an optional destination rectangle, a native 16-bit color and
COLORFILL optionally combined with WAIT. The original wrapper truncates its own
32-bit argument before this API. Geometry and clip-list errors precede native
borrowed admission; valid clipped pieces use constant argument-derived updates,
which define only those pixels. Expected after words are never rendering inputs.
Borrowed fill error precedence is initially a conservative native policy pending
the targeted driver comparison.

RGB565 copy derives busy state from owned admission, rejecting supplied busy
booleans. Source-key state is private per ID. Installed exact keys use bounded
geometry planning plus native keyed copies; missing-key Blt returns its recorded
error and missing-key BltFast uses the captured opaque behavior. Opaque same-ID
copies retain existing ordered per-piece GPU snapshots. Keyed overlap and
combined key/clip/borrowed branches require the next independent comparison;
unsupported geometry/flags/formats explicitly refuse.

## Validation and remaining boundary

The native fixture executable receives only operation inputs, initial bytes,
explicit environment palette context and original submitted BMP inputs. The
Python checker validates retained provenance and compares independent outputs
outside the native process. Full key mutation and DC reuse phases run on the
same owned IDs; error outputs are compared too. Synthetic checks cover unknown
pixels, partial fills defining bytes, lease writes/guards, palette deferral,
invalid regions, bad releases, foreign IDs, rejected arguments and owner cleanup.

This step does not complete shared palette/surface alias lifetime, flip/loss/
Restore/retry, additional formats/masks/pitch, live or wire integration, Windows
hardware behavior, or the whole surface milestone. Those remain explicit.

```sh
cmake -S renderer -B working/build/surface-backend -DBUILD_TESTING=OFF
cmake --build working/build/surface-backend --parallel 4
xvfb-run -a python3 tools/check-native-surface-backend.py --report working/tests/new-owned-backend.json
```

This reproduction requires Qt/OpenGL/Xvfb but no Wine, game installation or
original media. Choose a new report path; existing evidence is immutable.

## Combined operation completion — 2026-10-06

Two subsequent serialized Wine runs close the missing combined branches. The
unchanged original keyed wrapper `0x58ca90` provides 2,304 same-backing RGB565
calls: three keys, mixed/all-key pixels, same-pointer/Surface1 aliases, eight
geometries, six clip states, both APIs and WAIT pairs. The offline model matches
110,592 destination words, including 24 partial-error cases; six mutation tests
pass. Independently, 180 standalone driver calls compare fills and opaque/keyed
Blt/BltFast with source/destination Lock/DC or destination Lock+DC across all six
clip states. Eight mutation tests pass. Immutable manifests verify before/after
both runs, and no Wine sessions overlap.

These measurements overturn the provisional mapped-fill refusal: COLORFILL
succeeds with mapped destinations, preserving the active CPU/DC lease. The
backend updates its CPU snapshot along with GPU storage when a CPU lease exists;
twelve held-destination fill cases compare that snapshot with independent driver
bytes. Copy with an explicitly empty Blt clip list performs no draw and succeeds
even borrowed; BltFast still rejects the attached clipper first. Missing-list
errors are retained. Other mixed geometry/error precedence remains unvalidated.

Keyed same-backing reads are **forward row-major mutations**, unlike opaque
per-piece snapshots: a later source pixel can observe an earlier destination
write. A separate 1×1 GPU texture freezes each next source value before a keyed
single-pixel draw, avoiding framebuffer feedback. No pixel uploads/readbacks
occur during resident copies. The ordered path is intentionally capped at 4,096
texels per operation; larger requests refuse before any writes. This bounded
compatibility path is not a performance promise for large keyed overlap.
Opaque overlap and distinct keyed copies retain their existing GPU paths.

The [final current-source comparison](native-surface-backend-final-20261006.json)
matches 5,114 draw results and 538,272 native/DC pixel values through one owned
backend, plus 258,048 palette entries and 8,664 stable descriptors. Forty native
guards pass, including pre-write overlap-budget refusal. Twelve poisoned owners
still require explicit whole-owner teardown; 84 unstable failure caps words
remain excluded. All 14 renderer CTests pass in an isolated committed baseline
with the exact owned changes overlaid, preserving other contributors' Qt edits.
The suite includes the new `opengl-owned-surface-backend` CTest.

The first combined report predates checker/CTest wiring and remains historical;
its source hashes are retained. Base integration evidence also remains historical
where shared core/checker bytes changed. Fresh final evidence supplies current
validation; no old evidence hash is refreshed.

Chunk 1 is implemented within these recorded operation scopes. Shared palette
objects, live COM alias lifetime, flips and loss/Restore/retry belong to ownership
and recovery. Additional required formats/masks/pitch belong to format closure.
Uncaptured combinations, larger keyed overlap, CPU/DC raster interleaving after
extra Unlock, real HDCs, Windows hardware, live and wire replacement remain
explicitly outside this step. This does not complete the whole surface milestone.

```sh
xvfb-run -a python3 tools/check-native-surface-backend.py --combined --report working/tests/new-combined-backend.json
python3 tests/test-original-surface-keyed-overlap.py
python3 tests/test-surface-borrowed-draw.py
# Configure BUILD_TESTING=ON to include the combined suite in renderer CTest.
```

## Chunk 2: ownership and recovery — 2026-10-06

The backend now retains canonical aliases/shared palettes and two-buffer
attachments, swaps storage including CPU/DC lease state, and exposes explicit
loss/Restore over content validity. A recovered wrapper adapter routes bounded
retry/key/reload actions and the global cursor BMP/DC path with supplied bytes.
See [ownership and recovery](native-surface-ownership-recovery.md) for the measured
busy-after-swap cases, independent driver/original trace comparisons, and native
policies. This supersedes chunk 1's provisional ownership/recovery gap within the
listed offline scopes. Formats/pitch, uncaptured interleavings, actual asset and
physical original lost-draw composition, and live/wire replacement remain pending.

## Formats and row pitch — 2026-10-06

[Chunk 3](native-surface-formats-pitch.md) adds indexed8, RGB555, RGB565, RGB24 and RGB32 same-format fills/keys/copies with explicit signed owned byte rows. Independent standalone capture supplies 1,400 admitted draws and 2,100 actual rejected layouts; native replay also compares 2,800 signed-row variants. Active-mask key/fill rules and key descriptor output are measured. Imported negative pitches are native policy, not driver acceptance. Existing operation/ownership/retry comparisons freshly pass; required original format reachability and final milestone acceptance remain pending. Earlier sections describe historical scope.
