# DirectDraw surface contention and application-held capture

2026-10-07. Intentional native capture/launch policy, separate from recovered
game drawing and complete original replacement.

## Failure and confidence

The retained user run `run-a2iov6n2` logged four original `application_bltfast`
failures with HRESULT `0x887601ae` (`DDERR_SURFACEBUSY`), on two threads and two
Surface2 objects. Its manifest has `capture_locks=false`, `no_readback=false`,
no command/control channel and `continuous_commands=false`. The recent native
OpenGL batching changes were therefore not the presentation path in this run.

In that legacy path, `bridge.c::capture` takes an extra original DirectDraw Lock
while converting/copying primary pixels; draw snapshots also take observer
locks. Other original drawing threads can encounter a busy surface during that
interval. Observer-lock interference is strongly implicated, including the
earlier readback failure documented in `render-startup-black-screen.md`; the
exact held Lock or DC was not traced in this user's run. This is not a confirmed
leaked GL fence or a matched-workload before/after driver experiment.

The logged flags already include WAIT. Microsoft's
[BltFast contract](https://learn.microsoft.com/en-us/windows/win32/api/ddraw/nf-ddraw-idirectdrawsurface7-bltfast)
describes WAIT in terms of a busy bitblitter and permits other errors. It does
not authorize releasing another caller's surface ownership. Preserve the
original HRESULT and LastError rather than fabricating success or unconditionally
unlocking the surface.

## Presentation policy

Ordinary Qt OpenGL launches select continuous native commands and imply
application-held capture. `--frame-readback` now also implies `--capture-locks`:
it presents the owned primary mirror copied from application-held Lock/Unlock
buffers or DCs, without observer DirectDraw locks. It retains frame uploads
instead of native command presentation. The game still performs its original
rendering in both modes. Capture remains safe after native refusal because the
launch's application-held capture policy does not change during fallback.

`DllMain` disables observer readback when application-held capture is enabled.
The Lock/Unlock observer owns copied bytes before forwarding Unlock; it retains
a failed application Unlock for a later application retry. The held-DC observer
reads the application's existing bitmap rather than acquiring another DC or
replacing its selected bitmap. No original game surface is unlocked by Qt or GL.
Explicit legacy draw/history diagnostics can still request observer locks;
they are not ordinary presentation modes. Restart the game with the rebuilt
shell to apply the launch policy:

```sh
./tools/run-qt-shell.sh
# Optional original-renderer frame presentation, also without observer locks:
./tools/run-qt-shell.sh --frame-readback
```

Headless OpenGL replay consumes an already captured command stream. Its timings
cannot establish original-surface ownership safety or which live launch mode
was selected. Prior benchmark evidence retains its exact source fingerprints.

## Fresh validation

[Frozen evidence](render-surface-contention-20261007.json) retains the failed user
run, six actual Qt synthetic Launch cases, fourteen PE32 Lock/Unlock cases,
current input/queue/fallback regressions and an isolated original campaign run.
Both native commands and owned frame copies remain visible in windowed and
fullscreen/fractional-DPI tests; all modes request application-held capture.
The ownership fixtures prohibit extra Lock calls, poison storage during original
Unlock and check failed-unlock retry, HRESULT/LastError, aliases, negative pitch
and rejected inputs. These are synthetic callbacks, not driver equivalence.

The original campaign observation runs the idle World for 120 seconds without
injected reader failures. It completes 1,393 native presentations and 104,127
commands over 140.271 seconds including menu ingress, with no recovery requests,
no logged surface failures, no ordinary native readbacks or viewport uploads,
and zero terminal resources. The original remains alive, and all 2,927 immutable
input files verify before and after. Stable menu regions are independently
compared; animated World pixels are not compared. Phase labels inherited from
the recovery harness refer to injection points, but `surface_busy_observation`
disables those injections. The default recovery test keeps its existing strict
checks; the observation mode records native refusal separately and requires no
original application `DDERR_SURFACEBUSY`.

Reproduce from an optimized standalone renderer build:

```sh
xvfb-run -a -s '-screen 0 1800x1000x24' \
  python3 tools/test-live-render-routes.py working/build/renderer \
    --mode campaign --world-seconds 120 --observe-surface-busy
```

This bounded Xvfb/Mesa observation removes the observer-lock interference path
from normal presentation and did not reproduce the busy error. Hours-long
active battles, movies, the user's NVIDIA desktop and other game-owned Lock/DC
conflicts remain unvalidated. Recovery exhaustion is a separate producer/hosting
boundary. Existing evidence stays historical when shared launch/test files change.
