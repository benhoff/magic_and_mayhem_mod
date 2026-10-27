# Player lifetime and display-rebuild follow-up

The reviewed starting tree is `0357068df8474796946027f637932be878428003`.
This batch owns a separate worktree and does not include the main checkout's
pending surface-retry changes or the separate executed-CPU-plan work.

## Confirmed destructor failure

The unchanged starting implementation fails both staging and active-launcher
fixtures under AddressSanitizer and UndefinedBehaviorSanitizer. During implicit
`LiveMenuSession` destruction, `bridge_` and `timer_` are destroyed before
`GameProcess`. Its `QProcess` base destructor reaps the running child and emits
error/finished signals. The connected session handlers use an already-destroyed
timer and call `MenuBridge::retire()` on freed memory. Confidence is high for
this reproduced failure. `destruction-before02.json` retains both stacks and
the exact fixture/source/binary archive; `destruction-before01.json` retains
the initial compiler temporary-quota failure.

The session now disconnects process/timer delivery and retires its bridge while
all members are still alive. The process owner disconnects callbacks before
member teardown and kills/reaps its own launcher group and pending shutdown
helper. This terminal destruction does not report normal Quit or successful
asynchronous Wine-prefix shutdown. Four sanitizer fixtures cover staging,
active launch, a forked process tree and destruction during an in-flight stop.
The historical signal-11 runs lack a retained crash stack; this reproduction
does not establish their exact cause.

The full unmodified starting-master shell also receives SIGSEGV in
`InputState::publish`, through `Shell::finished` and the live-menu completion
callback while destroying `LiveMenuSession`. `shell-destruction02.json` retains
the GDB before/after results. Its preload fixture only requests application exit
after 500 ms while a fake preparer sleeps; it executes no game. The repaired shell
completes the same path with the requested exit1 and no signal. This confirms
the shell-owner consequence of late process callbacks. The earlier
`shell-destruction-before01` CLI combination was refused before staging and
receives no execution credit.

## Frame-size cursor continuity

The forwarder previously retained the last image-space point across a frame
resize. An 800x600 point `(799,599)` could be published beside new dimensions
640x480 until the next mouse event. The polling hook correctly refuses that
out-of-range snapshot and falls back to the original Win32 cursor. Before
heartbeat publication, inactive release and targeted events, the forwarder now
constrains the saved point to the current nonempty frame. Enlargement preserves
the constrained point. This is a native presentation policy; it neither warps
the desktop pointer nor changes the recovered engine cursor helpers.

The Qt input regression covers shrink without mouse movement, the next key's
targeted coordinates, inactive release and enlargement, alongside existing
client/desktop translation and high-DPI controls. Whether this caused either
historical Storage05 refusal requires separate live evidence.

`input-before02.json` links the new regression against the unchanged HEAD
forwarder and fails the heartbeat-shrink assertion. Current normal/high-DPI
tests pass. The initial standalone build omitted a renderer include directory;
`input-before01` retains that setup refusal without validation credit.

## Refused-copy geometry

The bridge formerly returned from source or Fast-position rejection before
constructing the destination rectangle. Its census then contained the full
target defaults instead of the request that actually failed. Both readable
argument rectangles are now captured before geometric checks. Fast extents use
defined unsigned PE32 arithmetic, retaining raw coordinate words even for
overflowing requests. Source-first refusal priority and admission remain.

The supplied PE32 probe uses the production bridge and owned graphics facade.
It checks complete source/target bytes, `NS_RECT`, zero COPY intervals, and
diagnostic rectangles for the known source-overrun shape, a separate target
overrun, extreme coordinates and combined source/target failures. These are
supplied controls, not reconstructed historical target coordinates or executed
original callers. Refused rows still receive no positive GPU-branch or whole
application output credit.

## Current bounded results

`controls03.json` prospectively binds the three independent implementation
contracts and 2,264 repository sources, including 391 actual Qt compiler inputs.
Seven normal Qt cases, four ASan/UBSan process/session cases, the full-shell
deadline fixture and five PE32 invalid-copy cases pass. The first unbound
refusal fixture omitted its observation-path initialization, then reused an
already-created output name; both refusals are retained. `controls02` executes
the valid PE32 cases but its supervisor rejects Wine's silent already-absent
server status1. The final supervisor admits exactly that status, still requires
the subsequent `-w` barrier, and rejects diagnosed stop errors. No engine guard
is relaxed by that bookkeeping correction.

The fresh actual public-player `storage01` passes Save/Load, both display
rebuilds and Main Load into a recreated World. All 18 cursor hooks and all 8
client categories execute;473 original Worlds balance and normal Quit leaves
zero active clients, pending jobs or graphics owners. The independent census
closes 4,474 application calls: 3,416 CPU copies, 984 GPU copies and 74 fills. All 4,400
COPY destinations match, with 12 retained Python checks over 3,102,160 pixels.
There are no failed or unprepared calls in this interval. All 2,457 keyed copies
use CPU sources; natural self-copy and keyed GPU-source calls remain zero.
The eight retired tail calls are explicit exclusions. This is a fresh bounded
whole-COPY pass, not retroactive identification of either Storage05 error.

`storage01-acceptance.json` joins those unchanged records. The public launch
uses the user's approved task-only 84 GiB storage floor via a private mount-view
configuration; the standing 90 GiB source policy remains unchanged. Its policy
bytes are separately declared and retained, rather than silently changing a
source fingerprint. No cold-store write is part of this batch.

Retention restores and verifies 6,910 regular files and 1,304,465,133 logical bytes.
Its new unique archive is 23,052,720 bytes and reuses the retained earlier blob
catalog. Execution sources use 2,251 verified Git versions at incoming master
`432d8b36` plus 13 unique overlays (44,404 compressed bytes), avoiding another
complete source snapshot. The incoming committed-history review covers 72 exact
first-parent file transitions with no missing receipts; it grants no past gate
pass or new engine validation. New scenario integration promotions are separate
from the recorded current implementation proofs.

The existing supplied GPU suite also passes on this bridge: `gpu-plan04` covers
all 23 key/self/alias/clipping/empty/partial-error cases plus command guards,
legacy baseline/restore and both renderer compatibility programs. It grants no
natural caller credit. The first cloned prefix was automatically returned by
the temporary environment manager; its empty successor times out in Wine setup
and later cannot load kernel32. A subsequent invocation used the test executable
where the replay CLI was required and correctly refused its missing output.
All three setup failures remain retained; the final run uses this task's admitted
prefix under both ownership leases and the actual `mnm-render-commands` CLI.

All 38 planning and 26 coverage-UI tests pass after preserving the three new
unrated IDs and the incoming surface-recovery unrated ID. Initial time limits,
the missing incoming expectation and the private network's disabled loopback
remain separate setup failures. No weights or historical source reviews change.

## Retained boundaries

Natural self-copy and keyed GPU-source callers require actual caller-bound
execution and independent output. Existing supplied branch controls cannot
establish their reachability. No key/history/dispatch mutation is introduced to
manufacture a natural positive. Original-driver pixels, unknown aliases/readers,
deferred installed-game paths and arbitrary duration/hardware remain separate.
Generated results and exact source preimages use SQLite references under
`player-open-items-results-20261027/`; failed attempts remain immutable.


## Current-master integration, 2026-10-27

Repair commit `6695eb356c420678e5bdc676be32d5188524713b` is integrated
separately onto current master `1efa145cff326fc3dea617843e4bec10997f3182`.
The incoming first-parent history review retains all 65 exact transitions. One
missing camera-archive receipt received retrospective accounting with its
original payload unchanged; that receipt asserts no past gate pass or new
engine validation. Main checkout rendering drafts remain outside this batch.

Fresh `PLAYERMERGE27.controls02` passes seven normal Qt tests, four sanitizer
fixtures, the full-shell staging deadline and five actual PE32 refusal/no-write
cases. Its prospective claims bind the three new behaviors and their exact
scenarios. Their integration milestone is now headless. The initial integration
attempt hit the 180-second cold-build deadline; its failed report is retained.
Completing that same source build and rerunning passes without engine edits.

Fresh combined-master `PLAYERMERGE27.storage01` uses the actual public player:
Save/Load, two display rebuilds, Main Load and World recreation complete with
447 balanced Worlds, normal original exit0 and zero cursor or graphics
owners. All 4188 emitted COPY destinations match independent transforms;
3264 CPU copies, 924 GPU copies and 74 fills are separately counted.
Two out-of-bounds cursor calls return NS_RECT without COPY intervals, so whole-application COPY closure is refused. Their exact source/target rectangles are retained in storage01-refusals.json: a 640x480 source to a 40x40 target requests source (640,512)-(680,552), then a keyed 40x40 source requests that same out-of-range target on a 640x480 surface. This is current negative evidence, not reconstruction of the earlier Storage05 arguments. The already-passing supplied refusal controls require unchanged pixels and the same strict result. Inactive/expired Qt snapshots deliberately retain original OS cursor fallback; no unsafe geometric admission or original cursor policy is changed to make this journey pass. 8 retired tail calls
remain explicit exclusions. The bounded source inventory contains 4,578 exact
Git-resolved inputs; the storage-only runtime policy is separately preserved.
Actual raw restoration verifies 6909 regular files and
1303254055 logical bytes. The task-only 84 GiB storage exception does
not change the standing source policy or write to the cold archive.

Natural self-copy and keyed GPU-source calls remain unobserved in this journey
(0 and 0). Supplied controls do not establish natural reachability or
all-game absence. Historical unstacked crashes and missing old refusal target
coordinates remain independently unattributed. Original-driver pixels, arbitrary
aliases/readers, hardware and duration combinations, ARM and installed-game
deferred-path execution retain their separate boundaries. No gameplay balance
change or complete engine replacement is claimed.
