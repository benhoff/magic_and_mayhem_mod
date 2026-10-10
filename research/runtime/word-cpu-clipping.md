# Bounded native CPU direct-word clipping

This increment implements a portable CPU clipping path from the recovered
direct-word pixel and [caller-state contract](word-backend-state.md). It is a
separate native service; the existing live adapter retains strict unclipped
admission. Original binaries and entry assembly remain unchanged.

`word_clipped.c` intersects opaque runs with a half-open rectangular clip inside
the borrowed canvas. It validates every row and both contiguous streams before
any destination write, including fully hidden rows. Signed origins, exact edges,
corners, hidden and oversized frames are supported within the documented bounds.
Opaque zero overwrites destination bytes; transparent pixels and stride padding
remain untouched. Empty dimensions return a successful no-op and retain anchors.

Native policy bounds are frame bytes 40..4MiB, canvas/frame dimensions <=2048,
stride <=4096 WORDs, nonempty clip inside the canvas and positive-frame translated
anchors in [-4096,4096]. Indexed/auxiliary planes, reordered/padded source streams,
input/canvas overlap, invalid extents and malformed controls are refused before
pixels or result metadata change. These are native safety choices, not recovered
unsafe original handling. Borrowed inputs/descriptors remain stable throughout
the call; output/descriptors must not alias storage.

The comparison maps unchanged No-CD entries `0x596cb8` and `0x597086`. Native
drawing reads encoded input and initial pixels only. Original execution and an
independent expanded-mask oracle supply expected pixels. Existing recovered
workspace prediction supplies the future adapter's state, independently checked
against all 16 original words. The actual production entry assembly is linked
with a host-only clipped admission stand-in; it executes the new CPU core and
workspace model for positive frames, and forwards empty frames for original
floating-state handling. Actual handled/forward counters distinguish successful
native drawing from a comparison accidentally falling back to original.

The corpus retains 17 placement groups across 64 positive synthetic frames, both
backends, zero/(2,2) clip-left/top, selected smaller viewports, WORD alignments,
four masked floating/flag seeds and 12 empty templates. Register/stack/defined
flags, selected masked x87 state/three finite active values, XMM/MXCSR, complete
canvas/guards/source bytes and arguments are compared. Undefined AF, floating
instruction/data history, unused stack slots, unmasked/full-stack/exception FP,
other backends, installed-asset generality and original malformed acceptance are
excluded. Win32 access/LastError/reentry and live clipping remain separate work.

Reproduce the media-free native boundary tests:

```sh
cmake -S renderer/sprites/word-clipped -B working/build/word-clipped
cmake --build working/build/word-clipped
ctest --test-dir working/build/word-clipped --output-on-failure
```

Reproduce the original comparison with a new prospective declaration:

```sh
python3 tools/draft-coverage-claims.py --behavior RS.word-cpu-clipping --behavior NR.word-cpu-clipping-admission --scenario word-cpu-clipping-admission-host-20261010 --output working/tests/word-cpu-claims-new.json
python3 tools/test-word-clipped.py --claims working/tests/word-cpu-claims-new.json
```

The runner retains all input/result records and command logs, performs immutable
input checks before/after original execution, verifies pinned executable hash and
entry bytes, reviews compiler dependency closure and guards source stability.
It runs native CTest under AddressSanitizer/UndefinedBehaviorSanitizer, and
compiles the core for freestanding PE32 as well as host/i386. Cross-compilation
does not establish Win32 integration or authorize clipped live bypass.

## Recorded result — 2026-10-10

The [original comparison](word-cpu-clipping-original-20261010.json) passes all
4,448 cases, with actual native handling of every positive request (4,400 total) and
48 empty forwards. Both original/model and production-entry check masks pass
all ten categories for every input. Native drawing independently matches all
original canvas pixels and guards. Selected caller state matches at exact edges,
partial edges/corners, hidden frames and nonzero clip origins as well as interiors.
No positive request obtains comparison credit through original fallback.

Sanitized native tests pass 288 manual placements, 43 atomic refusals and 3 empty
no-ops. The [initial null-output failure](word-cpu-clipping-null-output-failure-20261010.json)
stopped during CTest before original execution: draw passed its local output to
admission but failed to check the caller output before committing. A direct guard
fix and new prospective declaration preceded the successful full rerun. Both
immutable reports and their source fingerprints remain; historical hashes were
not refreshed. The successful run's compiler dependency closure and source
stability checks pass, and both original manifest checks verify 2,927 files.

The source base is `3ff3fbe` plus this increment. Native live admission and engine
entry assembly remain unchanged. Next connect the core and recovered state to
a guarded opt-in clipped shadow adapter, validate Win32 extent/admission and
empty-frame forwarding, then compare captured clipped requests independently
before permitting live bypass.
