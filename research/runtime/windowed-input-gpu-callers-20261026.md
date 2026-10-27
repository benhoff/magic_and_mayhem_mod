# Windowed input and natural GPU callers

The current windowed path can receive Qt input, return through native menus and
Quit normally with all owned graphics resources retired. Natural GPU self-copy
and keyed GPU-source callers are still unproved. Whole-application copy output
and reliable shutdown across all timings remain open; no engine status is promoted.

## Input and menu return

The old selector required a game client matching the whole native frame. Actual
windowed Wine geometry is an 800×600 desktop, a 646×512 outer game window at
(77,44), and a 640×480 Win32 client at (80,73). The read-only production probe
records old target zero and the new desktop selector's nonzero game target.

The new selector requires exactly one newly observed desktop matching the frame
and one mapped named game input window with visible overlap. Ambiguity,
incomplete trees and incompatible geometry refuse. Events translate desktop
pixels to the current X11 outer window on each delivery; no border dimensions or
client offsets are guessed. Input wire v2 explicitly carries desktop coordinates,
so GetCursorPos returns them without adding ClientToScreen again. Client-relative
v1 remains supported. See the [wire contract](../formats/render-input-state.md)
and [input policy](qt-input-forwarding.md).

A separate Results Quit timing bug could trap the host if its successful ACK
already contained the ready Main/Quick Battle root: the host stored that root and
waited for a further generation change. The fix accepts that already-ready root
only after the existing Results owner/thread/ACK/status guards, with zero handoff.
Other actions and unready roots retain the guarded transition.

`controls03.json` passes normal/high-DPI Qt event delivery, 17 Results timing and
identity cases, v1/v2 wire admission and actual synthetic PE32 polling in both
coordinate modes. The new ready-root test failed against the previous host;
`root-regression-before.json` preserves that failure. The initial selector
traversal failure and its exact source preimages are also retained. No original
game executes in these controls.

## Live findings, high confidence within the recorded calls

| Capture | Outcome |
| --- | --- |
| `quick01` | Refuses missing installation manifest before game execution. |
| `quick02` | Both Qt Right/focus-release/Escape sequences work; stage 8 menu-return deadline, missing retirement receipt and shell signal 11 remain negative. |
| `quick03` | Pre-final timing-fix code: two World visits, native menus and normal Quit pass; strict census has 128 checked original GPU calls, 57 geometrically reduced calls and zero surviving graphics owners. |
| `quick04` | Final code, ordinary window: reaches Main Quit and retires all graphics owners, but native history GAP reason 2 makes the journey fail. |
| `quick05` | Final code, declared Win32 window movement: both World visits, Qt keys/focus release/resize, Results return and Main Quit pass; shell and original process exit 0, with no fallback or recovery. |

The final capture has 1,380 closed application rows, no active missed callback,
143 checked original GPU calls and 28 actual geometrically reduced calls from
original return PC `0x0058c03d` in wrapper `0x0058bf40`. The ordinary helper moves
the window from `[77,44,723,556]` to `[-323,44,323,556]`; the client origin moves
from `[80,73]` to `[-320,73]`. Requested output `[-320,73,320,553]` reduces to
`[0,73,320,553]`, from 307,200 to 153,600 pixels. This is surface-bounds clipping;
additional clipping within those bounds is unproved. The shorter final run has
no retained clipped-command Python pixel witness; its complete selected GPU
intervals receive the existing independent native destination comparisons.
Original-driver pixel equivalence is not claimed.

The final diagnostics reach END. The graphics retirement receipt verifies two
factory creations, 65 surface creations, and zero remaining devices, surfaces,
DCs, clippers, palettes or pixel bytes. DirectDraw is not loaded. The resident
report confirms normal-Quit retirement. These are bounded ownership results;
the ordinary-run GAP is not erased by this successful capture.

## Remaining work

`remaining-callers02.json` classifies the final immutable census: all 678 keyed
calls are CPU-source prepared calls (`0x58c255`: whole keyed copy; `0x58c98b`:
keyed rectangle/restore). None has GPU source history. No observed row has the
same tracked or raw source/destination identity. This is a bounded observation,
not an all-game absence claim. Retained static wrapper/caller candidates do not
prove that a reachable caller supplies a GPU source or aliases its destination;
forcing those conditions would not establish a natural game path.

The strict whole-output validator still refuses 129 successful original calls
without native preparation: 108 CPU-source copies reject an attached clipper,
20 reject target rectangles, and one fill is unsupported. Its admission rules
remain unchanged. These paths, natural self-copy/keyed GPU sources, arbitrary
aliases/readers and original-driver comparison remain separate work.

In `quick04`, the final whole-copy commit is invalidated by changed epoch/source
or destination generations; the adapter publishes GAP and the host correctly
reports failure. Deterministic teardown/copy synchronization remains open. The
ready-root ACK fix addresses the separate host transition, not that native race.

## Reproduction and preservation

Use `tools/test-windowed-input.py` with a prospective declaration, fresh output,
SQLite report and stopped admitted prefix for the combined controls/build. Use
`tools/test-natural-gpu-copy-callers.py --mode quick` with that exact source-bound
shell, the checked installation manifest and isolated source fixture for the live
journey. `--win32-exposure` declares the ordinary window-movement stimulus.
The overall acquisition success condition still requires all requested natural
branches and full output closure; both missing branches remain explicit, so
`quick05.success` is false while its lifecycle/retirement milestones are true.

Every live acquisition has immutable-original manifest bookends. All reports,
unique run inputs, raw captures, binaries, setup refusals, source preimages and
negative evidence are retained through SQLite references. The first controls'
preimages and the successful pre-final source snapshot preserve their original
hashes. The prior installation/prefix templates and exact unchanged staged inputs
remain pinned; private RAM staging is temporary and reproducible from them.
No source push, publication or unrelated cleanup is performed.

`history04.json` reviews 19 intervening commits and 11,218 exact first-parent
transitions from `d5e08213` through base `6cdbbacd`. One missing ancillary log
receipt is appended retrospectively; original payload hashes and failed history
reader attempts remain intact. This accounting does not assert a past gate pass
or new engine validation. Unrelated pending master edits are outside this owned
worktree.
