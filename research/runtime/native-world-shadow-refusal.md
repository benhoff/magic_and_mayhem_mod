# Native World shadow capability refusals

## Observed failure

The retained user launch `working/experiments/scene-observer/run-hrgdte6c`
exited with viewer code 2 before any native presentation. Its native report
records producer state FAILED (3), reason 8 (unknown queue kind), zero readbacks
and zero original pixels used as native inputs. The channel retains a refused
80-byte World header with sequence 1 and no raster requests. This confirms a
capture capability refusal; it does not identify the particular draw kind or
the user's menu/battle stage. The build's optional Vulkan-header message is
independent of this producer failure. Confidence: high within these observations.

The previous producer made every capture refusal fatal, which closed the viewer
and caused launcher supervision to end the isolated original game as well.

## Native policy

`NR.world-shadow-refusal` applies only to normal shadow presentation. Reasons
7 (unsupported/uninitialized displacement state) and 8 (unknown queue kind)
publish a diagnostic-only header with zero requests and no partial payload.
The producer remains ACTIVE; it always forwards original drawing. The host
validates envelope identity, size, sequence, dimensions and mode, releases its
displayed GPU lease and shows a waiting/unsupported message. A later complete
owned frame can resume normal presentation.

Verification remains fatal on every capture refusal. Malformed envelopes,
unbound native resources, unknown native operations, structural failures,
changed channel identity and pixel differences retain fatal admission. No
refused packet supplies raster work or an original pixel oracle to the renderer.
Wire layouts remain v1; older clients refuse these diagnostics.

The native report counts consumed refusals by reason and retains at most 64
sequence/reason records. Publication drops and superseding still apply, so these
counts cannot reconstruct every queue or support live retained canvas history.

## Reproduction and evidence boundaries

Run `xvfb-run -a python3 tools/test-world-shadow-refusals.py`. The script builds
and executes the actual freestanding PE32 producer in isolated Wine for normal
and verification channels. It checks removal of a partial record, continued
publication after reasons 8/7 and fatal structural failure. An unreadable oracle
address ensures normal publication cannot depend on original output. Its success
packet tests transport only; the placeholder record is not a native draw fixture.
Qt tests separately exercise valid drawing, repeated refusal, lease removal,
recovery, malformed partial packets, bounded diagnostics and strict verification.

The bounded original startup scenario uses
`xvfb-run -a -s '-screen 0 1600x1024x24' python3 tools/capture-scene-game.py --world-live normal --skip-queues 0 --interval 1 --live-frames 24`.
It verifies immutable original media before/after and retains original drawing.
Passing it establishes native shadow refusal/recovery only for the recorded
selected Quick Battle requests, without original pixel equivalence. The user's
manual launch, other campaign routes, unknown draw implementations, initial
canvas ownership, complete delivery, HUD, native input and whole-scene bypass
remain separate boundaries. Immutable source-bound reports are registered after
execution; historical records and their hashes remain unchanged.
