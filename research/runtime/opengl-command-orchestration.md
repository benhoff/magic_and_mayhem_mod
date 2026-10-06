# Continuous producer startup and orderly exit

2026-10-06. Native policy `NR.command-orchestration` adds application exit
orchestration to the continuous producer. Original drawing remains active.
This is native policy and synthetic integration, not recovered exit behavior
or sustained original-game equivalence.

`RenderStartup()` returns readiness and preserves LastError. It requires an
initialized frame bridge, successful configured transport and, in continuous
mode, lock tracking and enabled capture admission. It starts the retry worker
outside DllMain. Startup and shutdown serialize worker handle publication;
startup after joined shutdown refuses without reopening the stream. Existing
DirectDrawCreate invokes this startup path and still forwards original arguments,
HRESULT and LastError. Explicit startup is available to other application hosts.

A configured channel that cannot open, map, validate, claim or allocate no longer
silently becomes optional. Continuous admission requires v2; v1 remains available
for default bounded sessions. Missing capture setup cannot report continuous
readiness. No wire version, opcode or failure reason changes.

Continuous mode installs a main-image ExitProcess import interceptor by default.
`MNM_RENDER_AUTO_SHUTDOWN=0` retains manual orchestration for controlled hosts.
The installer validates PE32 headers, image/import bounds, descriptor and thunk
termination, finite scan budgets, and exactly one resolved pointer matching
kernel32 ExitProcess before an atomic pointer replacement. It refuses malformed,
missing, duplicate or changed imports. It has no build-specific exit address and
accepts no unbound name as evidence of a resolved target. This initial lifecycle
installation occurs during attach; it neither creates nor joins a worker.
The supported exit invocation is an ordinary application call outside loader lock.

The exit callback calls `RenderShutdown(3000)` before forwarding the same exit
code and incoming LastError to the original ExitProcess. Shutdown closes capture
admission under the tracker, emits current DELETEs and END, then drains and joins
outside that tracker and loader lock. ACK means consumer-owned byte completion,
not GPU completion. The existing finite drain deadline and 1000ms worker join
remain; OS scheduling can exceed wall-clock targets. Join failure retains
worker-visible storage. Refusal never prevents original process exit.

Empty continuous sessions refuse GAP rather than waiting for nonexistent END.
Active locks/DCs, missing PRESENT, ownership loss and transport failure retain
refusal. Shutdown is idempotent; later original drawing callbacks remain callable
but cannot restart publication. After a normal process exit the existing Qt
consumer drains final owned commands and releases its resources. DllMain retains
its best-effort detach fallback and cannot join.

Forced termination, crashes, CRT paths bypassing the main-image ExitProcess
import, exit inside loader lock, dynamic hook uninstall, full original-game exit
coverage, startup allocation/thread fault injection and session recovery remain
pending. Installing an import does not establish that every original exit branch
uses it. No gameplay balance changes or original artifact modifications occur.

## Reproducible checks

```sh
xvfb-run -a -s '-screen 0 1280x1024x24' \
  python3 tools/test-render-orchestration.py working/build/render-ring
python3 tools/test-render-backpressure.py
python3 tools/test-render-command-idle.py
xvfb-run -a -s '-screen 0 1280x1024x24' \
  python3 tools/test-render-continuous-producer.py working/build/render-ring \
    --case short-archive --case no-archive
```

The actual PE32 executable uses its imported ExitProcess; the test does not call
the wrapper directly. Independently owned packed engine pixels and poisoned
borrowed rows feed real hooks, the ring, streaming decoder and GPU. Checks cover
automatic exit, explicit shutdown plus later original drawing, active locks,
missing presentation, empty sessions, v1, missing capture, missing/bad-sized/
overlong configured channels, malformed/duplicate/unresolved import guards,
manual installation and repeated startup/shutdown. Complete independent frames,
ACK totals, archive DELETE/END, native resource cleanup and ordinary zero
readback/upload counters are checked. No original media is consumed.

Execution records and exact coverage receipts are added only after final checks.
Earlier attempts remain under `working/tests/render-orchestration/`: initial
seven-case checks passed before stronger guards; a later harness expected a log
without capture configuration and was corrected to respect disabled diagnostics.
Historical evidence retains its fingerprints and independent statuses.

The prior backpressure commit `bc29824` was reviewed separately against its parent
`422be2a`: 20 committed files and nine affected behavior contracts match exact
receipts, with no unresolved historical accounting gaps. See
[committed history review](coverage/committed-history-backpressure-20261006.json).

## Recorded final-source result

[Eleven lifecycle cases](opengl-command-orchestration-pe32.json) pass, with eight
independent complete-frame comparisons. Automatic and explicit exit archives
end with two DELETEs and END; full published ACK and terminal GPU cleanup pass.
Configured startup failures also return failed shutdown consistently.
[Eight pressure cases](opengl-command-orchestration-pressure-v2.json),
[five idle cases](opengl-command-orchestration-idle-v2.json),
[two continuous GPU cases / 72 frames](opengl-command-orchestration-gpu-v2.json)
and [native/sanitized queue checks](opengl-command-orchestration-queue-regression.json)
pass. Production and selftest PE32 builds pass. The earlier pressure/idle/GPU
`*-regression.json` copies retain their pre-completion-fix fingerprints and are
historical; the v2 records above bind final sources. The first unbound-channel
harness waited for a consumer on an untouched unrelated channel; final unbound
cases correctly check startup without launching that consumer.

A subsequent [explicit fresh-session recovery increment](opengl-command-recovery.md)
adds a separate export after shutdown/join. `RenderStartup` itself still cannot
reopen a retired stream. Automatic host negotiation and original-game recovery
remain pending.
