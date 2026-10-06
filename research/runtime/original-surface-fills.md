# Original RGB565 fill outputs

Build: No-CD `40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
The unchanged fill wrapper at `0x0058bac0` runs in the same loader-reserved private
PE mapping as the [keyed-copy fixture](original-surface-keys.md), against real
Wine built-in IDirectDrawSurface2 RGB565 system-memory surfaces. Its observed
Blt forwards NULL source/rectangle, flags and the original 100-byte effects
structure to the real driver and returns the actual HRESULT. Complete destination
storage is read before and after. No original instruction bytes/files are changed.

Scope is a controlled isolated original routine, not a full game session or
Windows-driver compatibility. Original error-reporting code executes, but its
private MessageBoxA and DestroyWindow IAT cells bind to bounded observers: dialogs
return1, destruction returns0 and both calls are counted. This avoids a modal UI
or host window side effect and does not validate actual UI behavior. Driver errors
are never replaced. No lost-surface result is admitted; original Restore/reload/
retry behavior and the other fill wrapper `0x58bc10` remain pending.

The retained corpus covers full NULL and explicit rectangles, partial/one-pixel/
bottom-row fills, negative/excess/outside/empty/reversed rectangles, and detached,
single/disjoint, attached-missing and attached-empty clip lists. Clipper GetClipList
state is captured before/after, including canonical regions and missing-list
HRESULT. Five argument colors include high32-bit values. The original wrapper's
observed effects color must equal the low16 bits; that is a recovered wrapper
contract, distinct from a universal DirectDraw color-conversion policy.

Evidence: [capture](original-surface-fills-capture-20261006.json),
[portable corpus](../../tests/fixtures/surfaces/original-rgb565-fills-corpus.json),
and [fresh offline comparison](original-surface-fills-replay-final-20261006.json).
Catalogs pin raw/compressed hashes, build, wrapper and oracle. The capture record
pins harness/source/probe/driver identities and verifies immutable inputs before
and after. The capture tools share a lock and refuse an already active Wine
session; runs must remain serialized with other Wine experiments.

The independent CPU model admits rectangles and clips against captured input
clipper state, comparing actual HRESULT and all destination words. Native replay
uses the existing constant UPDATE route, generating each region's bytes solely
from the argument's low16 bits and its admitted rectangle. Captured after pixels
occur only in CHECK diagnostics. Successful native pixels are compared in full;
failed native API HRESULTs are not claimed because UPDATE has no Surface2 fill
result contract. Initial unknown destination storage, indexed/masked formats,
key/fill combinations, native fill HRESULTs, lost/Restore/retry, live composition
and Windows-driver equivalence remain open.

Offline reproduction needs no game media, Wine or original executable:

```sh
python3 -B tests/test-original-surface-fills.py
xvfb-run -a python3 tools/check-original-surface-fills.py \
  --build working/build/renderer --report working/tests/original-fills-new.json
```

A new independent capture must use new paths, after other Wine runs have ended:

```sh
xvfb-run -a python3 tools/capture-original-surface-fills.py \
  --fixture-dir working/tests/original-fills-new \
  --report working/tests/original-fills-capture-new.json
```

Recorded result: all250 CPU HRESULT/output checks pass (145 successes,65 invalid
rectangles,40 missing clip lists), as do145 native full-destination checks and
eight refusal tests. Clipped empty/outside successes retain unchanged storage.

The earlier offline report is retained as preliminary evidence before the shared
key helper was tightened. The final report fingerprints current replay sources.
