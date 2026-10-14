# Native Qt Quick Battle app journey

This opt-in application test follows the passing323-frame offline replay. It
uses actual native Qt menus/command presentation and the default owned graphics
adapter. Qt viewport input, focus-loss key release, resize, original Results
callbacks and normal Quit are separate operational checks. Original CPU surface
writers remain active; source-owned live drawing takeover and new independent
pixel parity are outside this test. Outcomes and limitations are recorded below.

## Current execution

[App05](draw02-qt-quick-battle-app-05.json) passes the actual compiled Qt app:
Main → Quick Battle → setup/map2 → default spell selection → World → Results →
Quick Battle → Main → original normal Quit. All nine original semantic callbacks
are observed on one engine thread. The default owned graphics adapter runs with
real ddraw disabled/absent, native command frames and actual Qt paints, no native
fallback/recovery/refusal, unchanged staged binaries and current source claims.

The test sends keys through the real Qt viewport event filter and mapped input
channel. Xvfb has no window manager, so it explicitly focuses the **Qt host**,
then allows its focus event to arrive before posting the first key. Right is
observed active/down; clearing viewport focus clears that key and deactivates
input. The viewport resizes before Escape reaches original Results. Normal
original Quit retires the final menu acknowledgement and clears all mapped keys.
The existing Results controller/bridge, resident Quit and normal/HiDPI input
checks pass5/5. This is an operational app integration test; the prior323-frame
source-owned offline comparison remains independent and unchanged.

## Preserved failures and boundaries

App01 was an argument-parser rejection because this worktree's launcher had not
yet exposed the existing native-command switch. Its original-executed flag was
initialized prospectively and does not prove execution. App02 actually reached
native spell selection, despite the old failed-wrapper false execution flag;
the experiment event journal supplies the execution fact. The current wrapper
derives failed-run execution from retained actual menu events.

App03 reached World and Results but its first synthetic key preceded actual Qt
host activation. The input word showed Right down while active was false.
App04 corrects that boundary but captures an immediate Results action rejected
as unavailable during the held-Escape/fade transition; no original Quit callback
was dispatched. App05 uses a120ms Escape hold and750ms settled Results delay.
No consumed request is retried. The test no longer blocks on a screenshot between
reading a fresh menu state and clicking its action. These fixes are confined to
the opt-in validation harness; rapid-action rejection remains an explicit app
boundary rather than being relabeled complete UI/driver equivalence.

The source helper is selected only with `--live-menu-test` and its explicit
validation environment. Ordinary session rendering, input and menu behavior are
unchanged. Original CPU surface writers remain active in the public native
command mode. Independent live pixel equivalence, full native producer takeover,
hardware timing, semantic camera/picking and universal resource teardown remain
open. The7314333ecommit was reviewed against45exact file receipts with no
unresolved history gaps before this coverage update.

## User-requested storage cleanup

The main checkout's scoped cleanup archived and verified51841payloads from17
completed unpinned drawing-api-session runs older than24hours under the two
DRAW-02 worktrees, releasing35.830GiB. It preserves source trees, metadata,
all pins/recent runs and the current prefix/EGL library inputs. The main
`working/retention/draw02-milestone-cleanup-result-20261014-01.json` is storage
provenance only. Every pruned payload has durable local and remote receipts in
`/var/local/magic-and-mayhem-archive`; use `tools/working-retention.py restore
<catalogued working path>` before reading archived inputs. Historical engine
hashes remain unchanged, and this storage operation does not grant validation.
