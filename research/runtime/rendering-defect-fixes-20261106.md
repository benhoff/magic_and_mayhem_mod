# Native rendering defect fixes — 2026-11-06

This batch changes native presentation and menu submission. It addresses Sharp
texel-boundary sampling and a reproducible lost-action race affecting Main, Map
and Save. Original game artifacts and old evidence remain unchanged.

## Confirmed Sharp defect

An isolated Qt/Mesa test using the retained 800x600 source and 1100x795 viewport
geometry fails against the existing independent pixel-centre reference: output
pixel (46,0) selects source x=19 instead of x=20. No original game is required to
reproduce it. The previous shader used interpolated floating-point coordinates
with nearest texture sampling. Both the CPU-image and native integer-texture
presentation paths now calculate the source coordinate using an exact integer
quotient of the physical destination pixel centre. Smooth filtering is unchanged.
The original reference and its zero-tolerance comparison are unchanged.

Regression coverage includes 800x600, 640x480 and odd-sized source images, three
viewport sizes, both texture paths and normal/high-DPI output. Historical Sharp
captures remain negative; a current implementation test does not rewrite them.

## Confirmed menu publication race

The native bridge takes a seqlock snapshot before publishing a request. If an
original update overlaps that read, it returns false. Main/Map callbacks discard
that failure; Save also lacked a request/refusal diagnostic. This reproducibly
loses an otherwise valid button action without advancing the host request or
calling the original. It matches the observed failure boundary in CM06/CS05;
their sparse records do not prove that every historical miss had this cause.

The bridge now distinguishes an unstable read from stable invalid admission.
The session retains one owned, unpublished action and retries it on subsequent
polls for at most 500ms of active menu time. It preserves the original generation,
screen, thread, acknowledgement, arguments, filename and copied setup rules.
Every existing bridge guard is rechecked. A published request is never resent;
duplicate input cannot overtake a retained action. Stable ownership/readiness
changes, retirement and deadline expiry refuse the intent explicitly. Media
playback pauses dispatch and the deadline; stop/restart discards the intent.

Save submission now reports accepted/refused requests. The campaign driver
records these and fails immediately on refusal instead of waiting for its overall
deadline. Wire layouts, original serializer behavior and gameplay are unchanged.

## Validation boundary

The bounded regression runner records sources, binary identities, full output and
failures through SQLite. Synthetic tests force publication across the callback,
then require exactly one request after stability, owned filename preservation,
and refusal after generation/thread/readiness changes or retirement. A separate
XTest/AT-SPI run delivers actual mouse press/release events to Qt controls while
the original lane is deliberately held unstable. It launches no original game.

These tests establish the native defects and their bounded corrections. Current
original-game camera/Save repetition, every historical failure cause, arbitrary
providers and the full acceptance decisions remain separate. Existing milestones
and historical fingerprints are preserved; native-source drift requires targeted
current execution before older whole-player results can be claimed current.

`RF06.native-regressions01` passes 286 complete framebuffer comparisons and 34
on-screen checks at DPR1/1.5, the 21 forced publication cases plus existing storage
guards, and three real XTest/AT-SPI button actions. All three physical callbacks
retain the intent while the synthetic engine lane is odd and publish exactly one
request after it becomes stable. Eight existing Main, battle, Results, Mini,
Preferences and storage suites also pass. The exact tested executables are
retained separately in `native-regressions-retention-01.json`.

## Current original-game diagnostic

`software640-storage-binding-01.json` binds one original-game execution after
the corrections. The explicit software, skipped-intro route passes original Save,
Mini Load, both display rebuilds, Main Load, restored gameplay and normal Quit.
The original event journal has one Save and two Load callbacks; all three native
storage submissions are accepted.846source imports return,12,254native calls
forward zero pixels, all graphics owners retire and the original process exits0.
The saved file hash remains exact across reload. Every World-tagged captured frame is640x480. The two display
changes return to the initial mode before Main Load. No independent new
World-pixel comparison is claimed.

Four earlier preparation refusals launched no original game: private NVIDIA EGL
startup, an unsupported already-modified working executable, its now-inapplicable
input manifest, and a missing worktree music catalog. The repaired attempt uses
explicit Mesa and the existing canonical pristine No-CD/assets/prefix inputs,
with a task-private low-resolution profile and the canonical inputs preserved.
Every refusal, source phase, exact input repair and the successful execution is
retained in SQLite. The old stopped camera collector remains stopped.

One successful diagnostic and forced publication controls do not establish the
precise cause of every old failure or repeated-run reliability. The historical
42-row pack, enabled-intro nominal acceptance and both final decisions are not
renewed or promoted. The new native implementation/preview scopes have fresh
execution support; other source-affected claims retain historical limitations.
