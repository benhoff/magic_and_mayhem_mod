# Original surface rectangle forwarding and clipping boundary

2026-10-06, No-CD SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
This is the first bounded clipping investigation after the portable surface
fixture milestone. It establishes selected original wrapper argument behavior,
not DirectDraw driver pixels, native clipping or a live renderer replacement.

## Confirmed behavior

The five selected wrappers do not crop, reject or repair the tested rectangle
inputs against the fixture's 8x6 source/destination dimensions. Negative,
out-of-bounds, fully outside, empty and reversed source rectangles and destination
coordinates are forwarded to the recorded API endpoint. A mocked successful
endpoint is deliberately independent of the Python prediction and records the
arguments supplied by unchanged original instructions. Driver admission and
results are not implemented by this mock.

| Entry | Selected contract |
| --- | --- |
| `0x0058c360` | Opaque rectangle copy; source wrapper is `this`, followed by destination wrapper, source RECT, x, y. Null source/destination interface bypasses the call. |
| `0x0058c4a0` | Same successful geometry as the opaque wrapper, with separate error/retry/restore branches left unexecuted. This contains the previous fixture return PC `0x0058c5be`. |
| `0x0058c8a0` | Source-keyed copy with separate error/retry/restore branches left unexecuted. |
| `0x0058ca90` | Source-keyed rectangle copy. |
| `0x0058cbc0` | Explicit source/destination RECT Blt, always KEYSRC plus WAIT in the selected code. Dimensions are forwarded independently, so potentially stretched inputs are not normalized by this wrapper. |

For the first four, `0x006f68d8 != 0` selects BltFast with the original x/y.
Otherwise the wrapper builds destination `(x, y, x+source_width,
y+source_height)` and calls Blt. `0x006f98d8 == 0` adds WAIT: `0x10` for
BltFast, `0x01000000` for Blt. Keyed variants add `0x1` or `0x8000` respectively.
Signed coordinate arithmetic has the observed 32-bit wrap behavior, including
the extreme-coordinate fixture. These globals are preferred build addresses,
not portable native policy or process-stable pointers.

When windowed and the destination wrapper equals `0x006f98c0`, the code obtains
the window handle from the object at `0x006f98d0` and uses ClientToScreen through
IAT `0x005c51fc`. The fixture binds that import to a deterministic mock returning
offset `(-19, 33)`. Translation affects destination coordinates only. The source
RECT remains unchanged. The first four wrappers translate a temporary destination
RECT; `0x0058cbc0` translates the supplied destination RECT **in place**. A missing
window object still reaches the mocked ClientToScreen with a null HWND; its mock
failure leaves the original zero-initialized offset unchanged. This does not
claim real Win32 failure behavior or thread/window lifetime validation.

## Execution and portable traces

[Fresh original execution](surface-rectangle-forwarding-original-20261006.json)
matches 966 cases: five entries, sixteen geometry categories, fullscreen/windowed
selection, WAIT on/off, nonprimary/primary/no-window-object routing, plus six
null-interface bypasses for the two guarded opaque wrappers. Four offline tests
pass against the retained compressed original argument corpus, including
deliberately incorrect cropping/flags. The existing five pixel-fixture tests
also pass. Confidence is high within the stated successful-call forwarding scope.

The i386 Linux fixture maps the hash-checked PE privately at its preferred base,
checks original entry bytes, initializes owned wrapper/COM objects and globals,
and substitutes the private ClientToScreen import binding. Original text bytes
remain unchanged. No game, Wine, window or driver is launched, and no original
file or working executable is patched. Original manifests verify before/after.
The sandbox blocks the required 32-bit system calls; original execution was run
outside it. Offline trace regression needs only Python and the retained corpus.

```sh
python3 -B tests/test-surface-rectangles.py
```

For a new original comparison, use fresh output and fixture paths:

```sh
python3 tools/compare-surface-rectangles.py \
  --report working/tests/surface-rectangles-new.json \
  --fixture-dir working/tests/surface-rectangles-new-corpus
```

Requires g++ with i386 development support. The tool refuses a different game
hash, existing reports and existing fixture files. Generated cases supply only
original inputs; expected call arguments never seed COM output or original memory.
Reports pin current tool/test and portable trace hashes. Historical pixel evidence
and its source hashes remain separate and unchanged.

## Remaining clipping work

Neither this argument corpus nor the prior in-bounds pixel corpus establishes
driver cropping, clipper regions, invalid-rectangle HRESULTs, failure/retry
termination or actual original loss/restoration. Real-driver boundary calls need
independent before/after native pixels, actual API results, clipper state and
input/output validity. In particular, do not infer a memmove or clamp policy
from the successful fake endpoints. The previous one-/three-row game samples may
have been bounded upstream; these wrappers themselves do not explain that
upstream producer behavior.

The renderer and wire protocol continue to require in-bounds copies and distinct
surfaces. Keep that admission boundary until a separate driver-output experiment
establishes the selected baseline. Record upstream rectangle producers and
unresolved driver COM flows rather than assigning this wrapper's forwarding
coverage to the entire drawing function or marking native clipping complete.
