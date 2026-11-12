# Native offscreen-target loss diagnostic v1

This private host diagnostic is opt-in (`MNM_SURFACE_LOSS_TEST=1` and
`MNM_ACCEPTANCE_NORMAL=1`) and refuses isolated/device-recovery mode. It neither
changes original game memory nor writes command-ring state, cursors or payloads.
It is a compiled provider fault adapter, separate from compiled menu automation;
the game journey uses physical public controls for menus, restart and Quit.

All files live beside the admitted native command channel. Requests must be
published atomically as `surface-loss-request-v1.json`, after completely writing
a private temporary file. The consumer accepts at most4096bytes, one request
per owner and exactly these JSON fields:

| Field | Value |
| --- | --- |
| `schema` | Integer1 |
| `producer` | Actual nonzero uint32 command-channel session identity |
| `transport_version` | Integer3 |
| `request_id` | Exactly32lowercase hex characters, a fresh128-bit nonce |
| `action` | `destroy-render-target` |

The request is read by the render owner after a completed export, with no pending
CPU reply or World job. The primary ordinary provider is GUI-resident: it destroys
its owned target on the GUI thread, then checkpoints and terminally retires.
The optional dedicated render thread instead parks and queues a GUI action. Under
the same mutex the GUI checks that parking is still current and shutdown has not
cancelled it, then destroys its actual `QOffscreenSurface`. The worker waits at
most two seconds. Cancellation or timeout cannot require a blocked GUI callback
to finish the join. The GUI never destroys a target concurrently with GPU work.

`surface-loss-observation-v1.json` is a unique, non-overwriting observation.
`schema=1`, `kind=native_offscreen_target_loss`, producer/version/request nonce
and SHA256 of the exact request bytes bind it to one owner. Booleans record the
GUI destroy and render-owner probe, pre/post target validity, actual typed graphics
failure and explicit injected/physical-reset/old-continuation scope. Decimal
strings record monotonic host microsecond clocks, completed Worlds, pending CPU
reply and render turn. `live_surfaces_before` is an integer. Game admission
requires positive actual Worlds; synthetic command-only controls earn no game
credit. `provider_owner` distinguishes `gui-resident` from `dedicated-thread`;
the primary game admission requires the former. `render_owner_gui_thread` and
`worker_thread` must agree with that owner. The expected error is
`OpenGL offscreen target is unavailable`.

The probe invokes the existing native checkpoint's context acquisition. Invalid
context/target admission fails before any checkpoint/readback can commit. It
does not fabricate a draw HRESULT or invoke a legacy DirectDraw Restore.

Terminal handling first returns GUI frame leases. The GUI may recreate the
offscreen target solely so its actual render owner can delete its GL storage. It
does not resume the failed command stream or restore old game progress.
`surface-loss-retirement-v1.json` retains every initial observation field and
adds the monotonic retirement time, deletion-target restoration and independent
command/World/frame-owner retirement booleans. Both command and renderer surface
counts after cleanup must be decimal string zero. Normal-completion credit is
explicitly false. Remaining original-runner/input/timer retirement and the public
fresh-generation restart are independently checked by the existing native
terminal/session observations and action-bound source/output records.

For the primary GUI case, the diagnostic owner invokes the explicit renderer
target-restoration API before ordinary terminal cleanup. For the dedicated
worker, its GUI join supplies the deletion target after failure. This establishes
those bounded owner-supplied target lifetimes. It does not establish a general
automatic GUI hardware/display recovery path or old-session continuation.

Stopped command-ring state is a separate witness. A provider-loss generation
may retain active/none `(1,0)`, failed/cancelled `(3,3)` or failed/interrupted
`(3,4)`, always with cancellation set. The last case is a bridge close/deadline
outcome; it does not certify one particular emitter. Exact ring geometry, zero
reserved words and at most one capacity of unsigned unacknowledged bytes are
required. Only the independently compared visible prefix earns credit; a
queued or unpublished terminal tail earns none. Fresh normal END remains a
separate, stricter admission. Overflow, gap and invalid-stream failure codes
cannot substitute for the injected provider failure.

Real platform-target destruction is not physical GPU reset. Undefined native
contents and separately scoped isolated checkpoint recovery retain their
existing contracts. Arbitrary concurrent loss, a permanently unavailable
driver/display server and legacy hardware/DirectDraw parity are not established.
