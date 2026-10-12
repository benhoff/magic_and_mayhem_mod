# Native startup without an output device

Intentional native policy: missing output at launch allows voice control and
movie/file-sound/music decoding to progress silently. Original no-device
behavior is not asserted. Voice PCM advances on a bounded native discard clock;
completed one-shots expire before an endpoint appears, and looping voices retain
current position and controls. Logical consumption and physical output availability
are reported separately. A compatible default endpoint attaches automatically.
Existing admitted-device loss/Restore behavior and explicit device ownership
remain separate.

MNMAUD01 version5 retains its layout. Optional initial-silence capability16 is
published before readiness, authorized only by explicit staging metadata and
retained as an initial-policy marker after attachment. Unknown capabilities reject.
The media and music paths retain their QAudioOutput renderer while initially
silent: attaching a renderer halfway through playback duplicated PCM and dropped
video frames in the first fixture. Changing only its device preserves the source.
MusicBroker initializes Qt device discovery on its surviving facade thread before
starting the disposable worker; otherwise stopping a startup broker could retire
the global notification provider and prevent later endpoint attachment.

## Execution evidence

[Current endpoint evidence](native-audio-initial-silence-endpoints-20261012.json)
uses four separate owned, actually empty PipeWire/Pulse graphs. Voices expire a
one-shot, advance a loop, acknowledge logical Restore and attach without replay.
Movie and file-sound tests retain all30 frames and66150 PCM frames each. Music
retains132300 PCM frames and completes after startup MusicBroker teardown and
automatic device arrival. Graphs return to empty and all owned daemons stop.
These are native host integration tests with software endpoints, not physical
speaker output or original timing/waveform comparisons.

[Healthy full-game evidence](native-audio-initial-silence-healthy-public-20261012.json)
passes complete native intros, menus, rendering, WAVs, voices and CD music through
normal Quit, with Wine DirectSound disabled. It records12 plays/116 voice ACKs,
zero unsupported/failed/fallback audio calls, and1777/1004 intro frames.
Source/input fingerprints, original manifests and private-process cleanup pass.
This run begins with a physical backend endpoint; its initial-silence markers
remain false. It does not validate an absent-device whole-game launch.

The final post-provider audio regression suite passes67 CTests and11 admission
tests; logs are retained under working/tests/native-initial-silence-*-post-provider-20261012.log.

## Preserved failures and remaining boundaries

The initial late-renderer fixture at native-initial-silence/run-2moo0u42 reported
67174 instead of66150 PCM frames and25 instead of30 video frames. Runs
run-qfi1x5of, run-valk2sk6 and run-4fmjemkh exposed the retired Qt device provider
following startup-broker teardown; decoder progress followed real elapsed time,
but an arriving endpoint never attached. Run-u53z1wpe exposed a private fixture
startup race; the helper now waits for the PipeWire socket before launching its
policy daemon. Earlier passing source versions and all failed raw records remain
unchanged. Final accepted endpoint run is run-e1u8mnxo.

Whole-game empty-graph attempts native-campaign/run-itq_p4ie and
run-d17u76qv time out before adapter initialization: no menu events, native draw
status or installed audio producer flags appear. Disabling legacy Wine audio
drivers in the second diagnostic did not resolve the stall. Neither run establishes
a native-audio decoding failure or successful whole-game silent startup. A basic
Wine command succeeds with the same owned empty server and private prefix
(private-audio-backend/run-iuulvc3c); a later owned-process inspection finds wineboot/rundll32 blocked on control.exe appwiz.cpl install_mono before Chaos.exe starts. A bounded retry suppressing optional installers is pending.
The healthy software-rendered run-av7582v1 failed a rendering-recovery refusal;
run-m9xmddpw failed source-stability checks because sources changed during it.
Neither is promoted. Retained raw campaign reports record cleanup and original
manifest verification.

Physical waveform/tail/listening, migration to an unsupported new output rate,
original no-device equivalence and whole-game empty-output launch remain pending.
Native manager/config/cache/event replacement remains a separate milestone.

## Complete empty-output game validation

[Final reproducible empty-output campaign](native-audio-initial-silence-empty-public-20261012.json)
passes the full native game through normal Quit:1777/1004 intro frames,
2476804/1393283 decoded soundtrack frames,7 WAV uploads/349862 bytes,
12 plays/116 ACK,10701888 generated and logically consumed discard frames,
7 CD plays and120 music requests. All voice failure/fallback/unsupported
counters remain zero. Physical availability stays false and initial-silence
markers stay true. Private graphs are empty before and after; owned daemons and
Wine stop, source/input fingerprints stay stable and original manifests pass.

The fixture disables optional Wine CLR/HTML installer modules before creating
its own prefix. Earlier empty-cache runs stopped at an optional Mono installer,
not game/audio initialization. The successful diagnostic run-kid0ywfu and its
private graph report remain unchanged; accepted reproducible wrapper run-pjdh2q98
uses game run-psxe_2la. Fresh four-case endpoint run-wjyx3hy_ binds the final fixture
source. This supersedes the whole-game pending milestone above, preserving the
earlier failure evidence. It does not promote original/physical equivalence.
