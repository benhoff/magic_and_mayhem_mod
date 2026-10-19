# Native isolated graphics recovery (2026-10-18)

`tools/run-native-owned.py --device-recovery ...` now selects an isolated OpenGL
service and raster Qt viewport. The game, command consumer, World producer,
resource handles and unsaved simulation state stay in their existing host
processes. GPU calls execute only in `mnm-render-service`. This intentionally
adds process traffic and native GPU readbacks; the ordinary rendering profile
continues to use shared textures and the OpenGL viewport.

## Operation commit and recovery

The renderer serializes owned values over a private, versioned pipe protocol.
No game addresses, host pointers or C++ layouts cross it. After a complete
successful response, the host retains each mutating operation in order. After
256 operations or 8 MiB of commands, it replaces that journal with a full native
RAM checkpoint. Surface pixels, partial validity, palette knowledge, clipping
and coverage are preserved; undefined contents do not acquire validity.

A worker exit, graphics error or operation deadline retires the worker without
joining a potentially stuck driver thread. A new service uses software Mesa,
loads the last committed checkpoint and replays acknowledged renderer operations.
Failover overrides GLX/EGL vendor and Mesa driver overrides so a forced hardware
vendor cannot defeat the software selection. Final operation/error checks execute
with the service context current, and fence-allocation failures enter the same
graphics retry path.
It then retries the interrupted operation. This also covers a draw that changed
GPU state but failed before its acknowledgment: that uncommitted state is
abandoned, so non-idempotent operations apply once. Game commands, CPU-read
replies, producer callbacks and World import acknowledgments are not replayed.

Checkpoints are committed only after their complete response. A failure while
capturing one retains the earlier checkpoint and journal. Each operation permits
two replacement attempts. The default per-message deadline is 1500 ms;
`MNM_RENDER_PROCESS_TIMEOUT_MS` permits a bounded 100–10000 ms override. An
asynchronous child reaper never blocks the GUI or game on an uninterruptible
child, and a global eight-child budget (active plus retired) bounds unreaped failed workers.

Protocol/semantic errors remain explicit refusals, including undefined reads and
malformed producer input. Repeated backend failure after the bounded retries
also remains an error; recovery does not fabricate successful output.

## Presentation and lifecycle

Isolated mutable frames contain native-produced RGBA in host RAM. Immutable
snapshots retain their pixels across worker replacement. Cross-owner RGB565
imports use the same exact presentation expansion/import shader as shared-texture
imports. Recovery transport reads are counted as recovery readbacks, separately
from diagnostic native/RGBA readbacks.

`GlViewport` is a QWidget host. Its ordinary profile owns a QOpenGLWidget child;
the recovery profile creates no GUI OpenGL context and paints native frame bytes
with QPainter. Reparenting or recreation of the Qt native platform window retains
the frame, input coordinate mapping and render session. The application supplies semantic presentation intent, independently of Qt
visibility (Qt itself hides widgets while destroying a platform surface). A
surviving window still requested by that policy is recreated on the next event
turn; intentional suspension and close cancellation do not reopen it. On-screen pixel controls cover the
recreated window, separately from the retained frame buffer. A hung producer driver
therefore cannot strand the GUI inside a texture wait or GL cleanup call.

The dedicated command owner and asynchronous World queue accept this independent
process target without creating GUI-thread offscreen surfaces. Their existing
ordering, bounded immutable leases, command drain, media pause and normal quit
contracts remain in place.

## Evidence and limits

The prospective, source-bound run is registered as
`NATIVE.native-isolated-render-recovery.final-20261018` under
`NP.native-isolated-render-recovery`. It exercises real OpenGL operations with
worker death, a permanently stalled worker and a driver error injected after a
non-idempotent draw and before acknowledgment. Independent expected pixels check
exactly-once continuation, partial validity, palettes, clippers, journal
compaction, checkpoint failure and cross-owner imports. Separate Qt controls
recreate an actual platform window and continue after a hung producer.

The same run checks isolated and ordinary threaded command owners, ordinary GPU
presentation, window presentation, input and local quiescent checkpoint/queue
recovery. These are synthetic application/renderer controls, not an original-game
or physical hardware-reset comparison. Live-game fault acceptance and recovery
cost under sustained hardware workloads remain validation work. An OS/display
server that is itself unavailable cannot display frames until that system
service returns; no application can guarantee continuation through an OS failure.

The earlier `NATIVE.native-isolated-render-recovery-20261018` result is retained
unchanged as historical execution. The final result above supersedes it for the
current source bytes and includes forced-vendor refusal/failover controls.
