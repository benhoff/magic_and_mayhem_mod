# Bounded native audio request handoff

The synchronous voice channel added a millisecond-scale wait to each sound
status/control request. Native dispatch used a 2 ms timer; the PE32 caller slept
for 1 ms while awaiting each reply. Creature maintenance can issue chains of
these requests. Reducing those transport waits is a native performance policy,
not a recovered sound-manager contract or a change to the original game clock.

The native audio worker now polls every 1 ms. After servicing a request it yields
while looking for following requests, bounded by 1.25 ms and 64 commands including
the first. Idle polls return immediately. The budget is checked between commands;
a file read or device operation can itself take longer. The producer checks for
a reply for at most 4096 x86 PAUSE iterations before falling back to Sleep(1).
The one-second timeout, permanent retirement, contention refusal, serial IDs,
payload lifetime, HRESULT/value-before-ACK ordering and version5 wire stay intact.
All Device, output, command and lifecycle work remains on the same audio thread.

The component regression uses an unending legal stream of alternating successes
and refusals. It checks every reply and requires a queued owner snapshot to run
while that stream is alive. Its peer has a hard deadline to avoid a hanging test.
The existing GUI-stall, PCM/WAV, retirement/restart and malformed-input controls
remain. Production PE32 COM fixtures exercise live-peer behavior and a stopped
peer's timeout/permanent retirement. Five focused native tests pass, with the
real silent Qt endpoint recovery test run on the host after the sandbox refused
PulseAudio wakeups. The sandbox failure is retained separately.

Private probes retained under `working/slowdown-20261112/` compare balanced
same-session windows. Polling alone changed source cadence from17.30 to18.28Hz
in the eight-Zombie probe15 and reduced the sampled publication-to-ACK span
from1.33 to0.45ms. Probe16 reached19.96Hz with1ms polling and a250us followup,
versus19.47Hz with2ms polling. These evolving scenes do not establish a universal
speedup or the isolated benefit of followup over1ms polling alone.

Probe17 observed twelve casts, eleven living creatures and six movers in its
first movement interval, then the unchanged original-raster guard refused an
unknown route (exit95). Its partial1ms-only versus250us-followup windows are
diagnostic only. Probe18 stopped after only eleven accepted casts within twenty
attempts. Production probe19 created twelve Zombies and averaged19.94Hz, but
only one moved; its motion gate fails. None of these failures is converted into
moving-crowd acceptance. Private signal controls and private staging overrides
are retained diagnostics and are absent from production.

Production probe20 completes eight ordinary casts and verifies eight Zombies
changing world positions. Its two periods with at least three movers measure
**19.98Hz (six movers)** and **19.89Hz (five movers)**. Four ground-order periods
measure19.62–19.98Hz overall; the latter two contain no observed Zombie movement.
The largest source interval is174ms in the last, idle-crowd period, so this does
not establish perfect frame pacing. The exact production Qt ELF is
`d91146e94040430a491fc5edf005f2f7eab2ae9e05332b41c44122291e8a2be0`.
The diagnostic intentionally stops after its input/motion proof; it is not a
normal-Quit or whole-player acceptance run.

Earlier recording-free probe11 observed five movers at12.20–13.67Hz. Its map
placement and trajectories differ from probe20; this is historical context,
not a matched causal speedup measurement. Sound transport is a demonstrated
contributor, not an attribution of every creature-maintenance stall to sound.

Production moving-crowd results are recorded in the linked SQLite reports. Source
cadence uses consecutive completed source generations, never native paint repeats.
Actual creature position changes are checked independently. Passive request/ACK
sampling misses sub-poll replies and begins at first observed publication; its
spans are not exact client blocking durations. Different map placements and
trajectories, finite windows and scheduling remain comparison limitations.

Retain exact tested binaries, source state, build and input recipes, fixture
results, raw headers, actor journals, failed attempts and verified SQLite backups.
The existing graphics admission guards, immutable original manifest and user
save/prefix remain protected. This work does not establish full original pixels,
audible waveform/timing equivalence, normal Quit or19–20FPS on every map and
combat load. Prior historical validation hashes and milestones remain separate.

Reports: [component controls](native-audio-handoff-results-20261112/controls01.json), [bounded live observations](native-audio-handoff-results-20261112/live01.json).

Current generated [coverage summary](coverage/summary-audio-handoff-20261112.md); the earlier default summary remains historical.
