# Original surface Restore and retry scheduling

2026-10-06. High confidence within scripted fault scheduling; driver loss and
restored pixels are separate pending evidence.

The [capture](original-surface-retry-capture-20261006.json) executes unchanged
no-CD instructions in a private i386 PE mapping with hash-checked input and
original manifests verified before/after. Four wrappers receive explicit COM
result schedules: success, lost→success, busy→success, generic error→success,
two losses, busy→lost→success and lost→busy→success. Both Restore results, key
success/failure, key-enabled global, optional source/destination reload callbacks,
Blt/BltFast and WAIT branches produce 4,480 retained input/ordered-call traces.
No Wine or DirectDraw driver is launched; fault injection is evidence of original
wrapper branching, not physical lost allocations or driver Restore behavior.

Confirmed rules:

- Partial fill `0x58bac0` draws once. Lost invokes destination Restore; successful
  Restore optionally reapplies the low16-bit saved source key and calls a nonnull
  reload callback with zero. It returns without repeating the fill.
- Full fill `0x58bc10` draws once, then after a lost result attempts destination
  Restore and exactly one additional full fill even if Restore failed. A second
  lost result does not cause another Restore or retry.
- Opaque rectangle `0x58c4a0` retries every nonzero draw result until success.
  Lost results restore source then destination; keyed rectangle `0x58c8a0`
  uses the same source-before-destination order. Either Restore failure still permits the
  other surface's Restore and the next draw.
- Only a successful Restore reapplies the saved source key and invokes that
  surface's optional reload callback. SetColorKey failure is ignored; the reload
  callback and subsequent work continue. Key values are truncated to16 bits.
- Error dispatcher `0x484c00` suppresses modal UI for the captured lost result;
  busy and generic error reach the bounded MessageBox/DestroyWindow observers.
  UI imports are rebound in private fixture data; original text is unmodified.

[Offline input-derived comparison](original-surface-retry-cpu-20261006.json)
checks every ordered event and consumed-result count. Eight regression/refusal
tests cover provenance, order, consumed counts, key failure continuation and a
finite schedule refusing an endless-error loop.

```sh
python3 tests/test-original-surface-retry.py
python3 tools/check-original-surface-retry.py --report working/tests/retry-new.json
```

The [portable C++ and native validity comparison](original-surface-retry-native-20261006.json)
now matches all4,480 traces using `reconstruction/rendering/surface_retry.hpp`.
Adapters own actual draw, Restore, key/reload and error-report actions; the model
has no Qt, COM pointer or renderer dependency. Explicit draw budgets refuse zero/
over65536 and distinguish original return from budget exhaustion.1,536 cases at
a two-draw budget retain the exact original event prefix without claiming success.
This bounded stop is intentional native policy.

`GlBlitter::invalidateContents` marks native pixel contents undefined. It neither
clears storage nor uploads zeros or asserts driver Restore. Explicit updates and
opaque copies establish only the written regions. Source and mask sampling,
keyed/masked destinations and whole-surface read/CPU or GPU presentation refuse
unknown bytes. Validity follows swapped storage; native clipper/palette metadata
stays with its existing identity. Such metadata retention is a native policy,
not original driver palette/key evidence. Fully known surfaces allocate no map;
invalidated surfaces allocate one byte per pixel within the existing surface/
pixel limits. Prior valid presentation snapshots remain historical valid frames.

Thirty GL checks include partial reloads, guards, swaps, clipped writes and
a test-only adapter composing recovered fill callbacks with explicit native
invalidation/reload input bytes. Partial fill without reload leaves contents
unknown; full fill's second successful UPDATE establishes complete constant
contents; a supplied reload establishes its own bytes even when key setting
failed. No live adapter or new wire opcode is enabled.

```sh
cmake --build working/build/renderer
xvfb-run -a python3 tools/check-native-surface-retry.py --report working/tests/retry-native-new.json
```

Eight renderer regressions pass. Unbounded original
copy loops require an explicit native execution budget; terminating on that
budget is a native policy, not an original success. Unknown restored pixels must
remain unknown until an actual reload or overwrite establishes their contents.
Calling the reload callback alone does not establish its success or pixel bytes.
Real driver loss/Restore, palette retention, asset reloader internals, the `-1`
reload sentinel route `0x4a2f90`, null-interface mutation during recovery, window
translation during retries and live adapter/replacement remain pending.

## Subsequent standalone driver evidence

[Real Wine Surface2 observations](surface-driver-restore.md) now cover mode-change
loss, successful/repeated Restore, primary wrong-dimension refusal and source-key
retention in two display formats. This API-only probe does not load original
wrappers; its COLORFILL returns success even while IsLost/Lock report loss.
Original wrapper composition with real lost draw results, palettes/reloaders
and live recovery remain pending. The native validity policy is unchanged.
