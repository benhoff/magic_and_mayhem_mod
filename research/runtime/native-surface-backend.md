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
