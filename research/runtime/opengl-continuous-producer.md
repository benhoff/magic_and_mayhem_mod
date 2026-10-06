# Continuous owned producer and bounded command archive

2026-10-06. Native policy `NR.continuous-producer` separates producer liveness
from diagnostic archive ownership. This increment extends the existing admitted
operations; resource identity/release, broader updates, backpressure policy,
automatic application shutdown and recovery remain separate chunks.

## Contract

`MNM_RENDER_CONTINUOUS=1` opts into a successfully bound v2 command queue. It
implicitly enables owned sessions and requires the existing lock-tracking
directory. A v1 or absent channel cannot become an archive-only continuous
session. Default bounded sessions remain unchanged.

Live state has its own active flag, sequence and byte counters. The ordinary
16-operation sample, first-PRESENT-by-64 rule, finite 1..32 presentation target,
256-operation ceiling and cumulative 64MiB/4096 archive limits do not end
continuous publication. CHECK output remains excluded from continuous rendering
traffic. Current snapshots, formats, ownership and admission checks still apply.

The command archive is disabled by default in continuous mode. Set
`MNM_RENDER_SESSION_ARCHIVE=1` to retain an independently bounded prefix using
the existing command format. Its separate sequence/byte counters reserve space
for cleanup. Reaching 4062 operational records or the 64MiB allowance attempts
archive GAP reason 2 and closes only the file. Live command sequence remains
contiguous; it never receives that diagnostic GAP. An optional archive creation
or write failure is logged and disables only that archive. An exhausted archive
is explicitly incomplete and cannot be replayed as a complete session. A short
archive that survives until orderly shutdown retains DELETE/END. Existing
per-operation lock/blit evidence files and lifecycle logs retain their separate
finite capture budgets.

The producer still bounds the session table to 32 surfaces and 16,777,216
pixels; the owned snapshot tracker keeps its 64MiB budget. The private queue
remains 32MiB and the mapped ring 1MiB. Queue overflow, consumer cancellation,
ownership uncertainty or unsupported drawing refuses the live stream. Records
and cumulative bytes reserve cleanup before their finite UINT32 lifetimes
exhaust; there is no cursor wrapping or automatic session restart. Operation
and presentation diagnostic counters saturate instead of wrapping.

`RenderShutdown` closes admission and emits current resource DELETEs and END
through the existing bounded drain/join path. Finishing with no PRESENT or with
borrowed ownership still refuses. Original process exit retains its existing
best-effort detach boundary; this increment does not install an orchestrator
shutdown call or recover after resource retirement/stream failure. Original
drawing stays active.

## Launch

```sh
MNM_RENDER_CONTINUOUS=1 ./tools/run-qt-shell.sh --native-commands
# Optional bounded command prefix:
MNM_RENDER_CONTINUOUS=1 MNM_RENDER_SESSION_ARCHIVE=1 \
  ./tools/run-qt-shell.sh --native-commands
```

The Qt launcher selects v2 for this opt-in. Staging rejects a continuous request
without a fresh v2 channel before reading/staging game media, and records the
continuous/archive configuration. Opening the shell does not launch the game.
Full original startup/menu/gameplay validation remains pending.

## Validation

```sh
xvfb-run -a -s '-screen 0 1280x1024x24' \
  python3 tools/test-render-continuous-producer.py working/build/render-ring
```

The independent PE32 fake engine owns separate packed pixels and padded borrowed
rows, then poisons all exposed bytes at successful Unlock. Actual hooks feed the
v2 producer, idle queue, native streaming decoder and GPU consumer. The test
compares every displayed pixel through complete RGBA hashes, verifies original
call counts/HRESULT/LastError, and checks command/byte counts, archive terminal
records, consumer acknowledgement and native resource/readback/upload cleanup.

Cases cover no archive, record and byte exhaustion, archive creation failure,
a completed short archive, v1 rejection, missing presentation and the retained
surface-table capacity. Long complete cases exceed 64MiB, 4096 commands, 256
operations and 32 presentations, including first presentation after more than
64 operations. These are synthetic contract tests; they do not establish
original-driver equivalence or sustained original gameplay. Execution evidence is recorded separately after the final sources finish these
checks. Continuous mixed-operation sessions, injected mid-file write failure,
full UINT32 lifetime exhaustion and real-game startup/gameplay remain pending.


## Recorded synthetic result

[Final-source execution](opengl-continuous-producer.json) passes all eight cases:
five complete native sessions, three intentional refusals and 352 independent
full-frame comparisons. Long sessions publish 73,723,756 bytes / 5,143 decoded
commands, retain 70 changing frames and complete 5,070 original Lock/Unlock pairs.
The byte-archive case adds later small updates for 73,728,876 bytes / 5,223 commands.

The record-limited archive stops at 4,063 records including GAP / 260,008 bytes;
the byte-limited archive stops at 207 records / 66,068,480 bytes. Both live
streams still complete 70 native frames and END. Archive creation failure also
leaves live production complete; the short archive ends normally with DELETE/END.
Consumer acknowledgement reaches every published byte in long complete cases,
and ordinary native/RGBA readbacks, viewport uploads and terminal GPU surfaces
are zero. The first sandbox attempt could not start Qt/Xvfb; an early fixture
reset its shared hooked vtable and was corrected. The consumer count field was
renamed to avoid collisions with bounded test reports. Those attempts remain in
`working/tests/render-continuous/`; the retained record uses final sources.

Both missing-channel and v1 staging guards reject before game media access.
Production/selftest PE32 builds and the Qt shell build pass. The existing queue
regression passes native and ASan/UBSan runs, including leak checking; its
sandbox attempt hit LeakSanitizer's tracing restriction and its unrestricted
rerun passed. No original media or game executable was consumed by these tests.
Historical shared-source evidence keeps its old hashes and is not refreshed by
this scoped result. Default bounded/idle regressions are recorded separately.

The default bounded [v1](opengl-continuous-producer-bounded-v1-regression.json)
and [v2](opengl-continuous-producer-bounded-v2-regression.json) sequences each
pass eight cases and 44 complete-frame comparisons. The [idle lifecycle](opengl-continuous-producer-idle-regression.json)
passes five cases; the [queue](opengl-continuous-producer-queue-regression.json)
passes native and sanitized checks. The [bounded DC suite](opengl-continuous-producer-dc-regression.json)
passes eighteen cases and 38 complete-frame comparisons. These new records cover
default-policy compatibility within synthetic fixtures, preserving historical
original-comparison and replacement statuses.

The existing owned-session suite also passes all ten mixed, indexed, failure
and limit cases (`working/tests/render-owned-session/run-ev0q0z89/report.json`).
That legacy report lacks source fingerprints and is retained as a regression
observation, without promoting a current-source equivalence claim.

## Subsequent resource-lifetime increment

[Continuous resource identities and final Release](opengl-continuous-resource-lifetime.md)
adds monotonic IDs, admitted DELETE and slot/pixel reclamation. The earlier result
above remains historical and keeps its original fingerprints. The resource
increment records fresh lifetime and continuity evidence separately; implicit
unobserved destruction, original COM/driver equivalence and recovery remain gaps.
