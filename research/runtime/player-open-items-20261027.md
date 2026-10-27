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
