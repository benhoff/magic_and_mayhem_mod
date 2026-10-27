# Original copy argument lifetimes across retries

Confirmed for No-CD SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`,
opaque copy `0x58c4a0` and keyed copy `0x58c8a0`. Confidence: high within the
scripted argument-forwarding domain below. No game instruction bytes were
patched; only private fixture import/global data and controlled COM endpoints
were supplied. Original manifests verify all 2,927 immutable files before/after.

The destination rectangle is calculated before the draw loop. Window translation
is queried once if the initial mode is windowed and the destination equals the
primary-wrapper global. A missing window provider supplies a null HWND to the
ClientToScreen import; this fixture returns failure without changing the initial
zero point. No later retry repeats that query or recomputes destination extent.

Each draw re-reads the fast-mode global `0x6f68d8`, no-WAIT global `0x6f98d8`,
source interface and source RECT memory. Thus a changed source RECT reaches the
next COM draw while the cached Blt destination retains its initial dimensions.
Fast calls always use the original by-value x/y arguments. A windowed-to-fast
switch discards the cached translation for the fast call. A fast-to-windowed
switch uses the initially calculated, untranslated Blt rectangle; it does not
perform a new window query. Changed primary/provider globals after the first
failed draw also do not cause another query.

The loop heads are `0x58c56a` and `0x58c958`. Both non-lost error paths and the
source/destination recovery tails jump there, after the geometry/window setup.
Successful Restore reapplies low-WORD keys when enabled; key failure is ignored.
Source/destination Restore failures do not change cached destination coordinates.
The key ranges occupy separate stack storage after accounting for pushed
arguments; the key preparation does not overwrite destination RECT fields.

`SurfaceCopyRetryGeometry` expresses those argument lifetimes separately from
native surface storage and the existing snapshot-context copy API. It uses
defined 32-bit wrapping arithmetic and the existing scoped geometry planner.
The native snapshot service is intentionally distinct: these findings do not
retroactively validate dynamic original behavior in that service.

The [SQLite comparison](native-surface-copy-recovery-results-20261026/context03.json)
matches 18,432 original cases against native 32/64-bit and ASan/UBSan 32/64-bit
models. Inputs cross both entries, initial Blt/BltFast and WAIT states, no
primary/primary-provider/missing-provider contexts, key enabled/disabled, direct
success/lost-success/lost-busy-success, all four Restore success/failure pairs,
key success/failure, eight mode/WAIT/window/source-RECT mutation combinations and
ordinary/overflow coordinates. Every ordered window, draw, Restore, key and error
endpoint event and final source RECT is compared. The independent original
events never seed model state. The separate existing 4,480-schedule comparison
remains its own evidence.

Two preliminary fixture attempts remain failed. `context01.json` records GCC's
strict ISO rejection of the required PE32 thiscall pointer extension, before any
entry ran. `context02.json` and `context-debug01.log` record an SSE stack-alignment
fault in the C++ fast-call recording endpoint. Original Win32 callers provide
four-byte alignment; the corrected endpoints use `force_align_arg_pointer`.
This is fixture diagnosis, not an original engine fault or a model mismatch.
Their source variants, partial outputs and manifest bookends remain retained.

This closes the selected retry/window/argument lifetime gap, not complete surface
recovery. Null/replaced COM interfaces, callbacks changing keys/routes/ownership,
GetHWnd failures that mutate outputs, real driver loss and pixel effects,
unbounded original error loops, arbitrary reentrancy, Windows/hardware behavior
and live replacement remain pending. The original fixture caps draws at three;
all admitted original cases return within that bound.

Reproduce under the storage guard with new output identities:

```sh
python3 tools/test-original-surface-copy-context.py --main-root /path/to/main \
  --output working/tests/original-copy-context-next \
  --report research/runtime/original-copy-context-next.json
```
