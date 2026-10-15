# Native menu startup admission

The historical native Initiate run `run-r0raru91` rendered Main but never
published the menu engine lane. Its final cancellation does not identify the
initial fault. No admission diagnostic was retained, so its exact cause remains
unknown.

Code inspection confirms that `menu_init` previously made one seqlock snapshot
at DLL installation and permanently unmapped the channel on either an odd
sequence or a crossed host publication. The host publishes every 50 ms. A valid
concurrent publication could therefore disable all later native commands.
This is a native adapter policy defect, not recovered original behavior.

The change under investigation retains a pending mapping and attempts admission
on subsequent original engine ticks. It never waits under the DLL loader lock.
Only transient snapshot instability can retry; malformed/dead/already-used lanes
and the bounded startup deadline refuse permanently. Commands and engine-lane
publication remain disabled until admission. Three repeated cold-start
storage journeys pass below. A changed-map/changed-World identity journey remains
a separate boundary.

## Admission and diagnostics

The mapping and version negotiation still happen during DLL installation. One
nonblocking admission attempt runs there; later attempts run on the original
menu engine thread. Pending admission has a 120,000 ms deadline, matching the
host's existing initial-publication allowance. It does not publish engine state
or acknowledge/dispatch commands. After admission, the existing heartbeat lease,
once-only request IDs and permanent retirement rules remain in force. A stable
snapshot with a dead host, pending request or nonzero engine lane is refused
permanently; repairing that memory does not reset the admission state. Header
failure and deadline expiry are also terminal. No protocol version/layout changes.

The optional diagnostic file is created beside the shared channel as
`channel.bin.admission`, using create-new semantics. Failure to open/write this
file does not change channel admission. Its 16-byte header is `MNMMADM1`, version1,
record size32, followed by at most64 reason-change records. Each contains eight
little-endian DWORDs: record ordinal, reason, tick count, observed host sequence,
observed engine sequence, LastError, channel version and thread ID. Sequence values
in diagnostic records are observational, not a second authorizing snapshot.
Reasons1..7 are admitted, unstable host snapshot, invalid header, dead host,
preexisting request, existing engine ownership and deadline expiry. Reasons8..11
are channel-file open, unsupported size, mapping creation and mapping view failure.
The trace is independent of the existing bounded original callback event stream.
Tick wrappers preserve the original return value and LastError.

## Focused execution

The [source-bound controls](menu-startup-admission-controls-20261015.json) pass
normal startup and startup held odd through installation plus300 original ticks.
The latter verifies no engine publication or command execution before releasing
the writer, then passes the existing once-only, stale generation, ownership,
unsupported action, unstable snapshot, lease retirement and non-revival guards.
Policy controls also cover invalid/dead/already-used lanes and the exact deadline.
The standalone policy test supplies stable/unstable reader results; it does not
reproduce a crossing writer at a particular instruction. Original callback bytes
are executed in the PE32 fixture with the common engine helper and tick stubbed.
This is separate from real-game execution.

```sh
cc -std=c11 -Wall -Wextra -Werror tests/menu-startup-admission-test.c -o /tmp/admission-test
/tmp/admission-test
python3 tools/working-retention.py run --reserve-gib 2 -- \
  python3 -B tools/test-menu-observer.py --startup-busy \
  --executable working/tests/pixel-staging/run-qeshupqr/source-game/Chaos.exe
```

Use `--channel` instead of `--startup-busy` for the stable-start control. The fixture
checks original manifests before/after, verifies the executable hash and builds
from the current observer sources. The first race fixture failed compilation on
an undeclared test-only UnmapViewOfFile import; after its declaration was added,
the retained source-bound controls passed. It was not a live admission failure.

Four additional [PE32 refusal controls](menu-startup-admission-refusals-20261015.json)
start with a corrupt header, a stable dead host, a prior request, or an already
owned engine lane. Each retains the existing engine sequence through300 original
ticks and records its distinct refusal reason. The existing successful-admission
assertion then exits42 as expected. These controls do not repair the memory and
try to revive the adapter; permanence after rejection is also a code-review
finding, while the older lease controls directly execute non-revival after
admitted-state retirement.

## Three independent cold launches

| Cold launch | Balanced native Worlds | Optional native calls | Admission trace |
| --- | ---: | ---: | --- |
| [First](menu-startup-cold-1-20261015.json) | 703 | 7,622 | admitted V17 |
| [Second](menu-startup-cold-2-20261015.json) | 736 | 7,963 | admitted V17 |
| [Third](menu-startup-cold-3-20261015.json) | 741 | 8,000 | admitted V17 |

Each creates a fresh private Wine prefix, completes the four Initiate opening
commands, Save/resume/Load/resume, original Quit Yes/report/Main Quit, and normal
exit0. All have matching completion/import/return counts, no pending World,
normal resident retirement, zero optional producer forwards/history refusals,
unchanged original save bytes and owned-store merge. Original manifests and
staged inputs remain unchanged; private Wine cleanup succeeds. The software GL
functional10FPS floor passes (12.92..13.55 measured engine FPS). The30 portable
campaign/storage controls also pass. This is not a hardware performance result.

All three admitted on the first stable snapshot. Therefore the live runs validate
normal cold startup after the fix; they do not reproduce or identify the initial
historical intermittent failure. Forced-overlap recovery is established by the
separate PE32 fixture. No random-write timing or post-cleanup terminal error is
used to infer the historical admission reason. All Loads retain World canvas2.

```sh
python3 tools/working-retention.py run --reserve-gib 5 -- \
  python3 -B tools/test-native-campaign.py --save-load --difficulty 0 \
  --resident-world --other-pixel-producers --legacy-audio --skip-movies \
  --min-fps 10 --timeout 420 \
  --source-game working/tests/pixel-staging/run-qeshupqr/source-game
```

The first two source-bound startup declarations predate the final scope wording
and scenario association; the third prospectively binds the final startup/storage
scenario. Their bytes and claims are retained. Shared menu changes leave older
unrelated evidence historical; these new journeys do not renew every menu,
reader, scene, raster comparison or replacement claim. The incoming history
review covers [6f069135..a5cd6514](coverage/menu-admission-incoming-history-20261015.json)
with28 exact file transitions and140 behavior fingerprints, no missing receipts.
