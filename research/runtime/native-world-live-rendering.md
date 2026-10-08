# Continuous native World presentation

The opt-in `tools/run-native-world.py` launcher stages a hash-checked disposable
original installation with the selected World observation hooks. Original code
sections remain unchanged during import staging. The queue observer writes
owned effective raster requests to the bounded World v1 channel, rather than
creating unbounded scene files. It skips the selected startup queues and attempts
publication at a configurable queue interval. No consumer backpressure waits
are added. Copying inputs and instrumentation still add work to the original
thread; no original timing equivalence is claimed.

The live CPU worker builds a bounded `WorldCatalogue` from installed v4
SPR files, normalizing the original palette pointer out of each encoded frame.
It pins file hashes and prepares only requested native resources. The GUI adopts
the owned decode without reopening its file; indexed
aliases use each request's owned actual colour table. Non-v4 files remain outside
the catalogue; a requested missing identity refuses the complete live frame.
The existing native resource budgets still apply across the session.

`LiveWorldSession` owns worker orchestration, decoded resource adoption, the
renderer and session cancellation. Catalogue construction, packet decoding,
resource preparation, identity resolution and colour/coverage or projected-shadow
plane construction run on the CPU worker.
`GlViewport` receives an existing shared-context GPU lease.
The Qt application preflights the complete frame, then draws at most 32 requests
per timer poll with a 256 KiB colour/coverage row-transfer budget and cooperative
2 ms elapsed check, requesting one owned CPU payload at each GPU miss. It presents
only after completion. Each World begins
with an independently owned zero background; original pixels cannot seed it.
Verification mode separately compares every native RGB565 pixel with the owned
original post-consumer oracle before presenting. Ordinary mode omits both that
oracle and native readback. On failure, cancellation or resize the session clears
the old viewport lease before releasing native scene storage.

This is a standalone continuous World shadow viewer. Original menus and battle
input remain in the original window. World capture is restricted to the declared
selected dispatch and primitive backends. HUD/window composition, native input
ownership, campaign/camera/transition coverage, alternate dispatch, original
consumer side effects, resource retirement under long sessions, performance and
whole-scene raster bypass remain independent milestones. A complete World canvas
does not prove a complete game-window frame or authorize broad replacement.

Synthetic checks cover concurrent slot ownership and closure, superseding,
malformed packet extents, identity changes, terminal failure, GPU presentation,
ordinary readback absence, resize, unknown operations, mismatch refusal and
surface release. Live pixel evidence is recorded separately after execution.

The [asset preparation contract](native-world-asset-preparation.md) records
owned worker handoff, stale-result refusal, bounded completion/cancellation and
the remaining synchronous scene preflight and canvas creation. The
[upload preparation contract](native-world-upload-preparation.md) describes
worker planes, projected-shape caching and resumable GUI row uploads.

Normal shadow mode now distinguishes capability refusals from structural errors.
Capture reasons 7/8 deliver a closed diagnostic header instead of failing the
producer. The session clears its native viewport, displays a waiting/unsupported
message and resumes on the next complete admitted packet. The original window
continues drawing and accepting input. Verification, malformed inputs and other
capture errors remain fatal. Counters are bounded diagnostics of consumed
packets, not a census of every original queue. See the
[failure evidence and recovery scope](native-world-shadow-refusal.md).


## Opt-in finite startup history

`./tools/run-native-world.py --startup-history` retains all first16 source queues
and native completed canvases through asynchronous CPU/GPU preparation. Add
`--verify` to compare separate original post-consumer output. Source/reset metadata
and strict contiguous admission use [channel v2](../formats/world-channel-v2.md).
The fixed-canvas finite journal has its own [validation boundaries](native-world-live-history.md);
it does not extend the default newest-frame mode to sustained retained history.
