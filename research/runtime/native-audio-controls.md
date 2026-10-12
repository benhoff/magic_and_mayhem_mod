# Extended native static voice controls

NA.extended_voice_controls is seeded before implementation. Native static PCM
voices gain pitch changes and frame-aligned byte seeks without changing source
format. Frequency0 restores the source clock; explicit rates are100..200000Hz,
matching constants in the installed Wine dsound.h. Duplicates retain independent
controls while sharing sample storage. Generated cursors wrap0 after completion;
these are not physical output-consumed cursors. Static write cursor equals the
generated play cursor because no streaming writer/ring contract is implemented.

Offline ramp/phase/duplicate/bounds/control tests and wire/PE integration remain
pending. Original driver cursor/frequency/seek equivalence, consumed timing and
primary streaming rings are separate milestones.

## Version4 voice transport and validation

MNMAUD01 version4 adds frequency/set/read, generated-cursor and aligned seek
operations18..21. The header/payload remain128bytes/16MiB; capability2 advertises
extended controls before readiness (with WAV capability1, the mask is3). Fresh
menu staging binds that policy; old channel versions or missing capabilities
refuse. Static PE cursors validate output memory and preserve outputs on failure.
Primary streaming rings remain unsupported. Source format metadata is unchanged
by pitch; frequency0 restores the source clock. Seek rejects unaligned/outside
bytes, resets fractional phase and preserves active looping playback.

Independent native ramp tests validate pitch/phase/seek/duplicate/bounds and
released/primary refusal. The real PE32 COM fixture validates frequency0/44000,
invalid99, source format44100 restoration, byte2 seek, read/write cursor2,
optional/invalid pointers and zero reset, alongside existing native WAV/primary/
lifetime/stale-host behavior.42 requests/ACKs balance; one deliberately
unsupported play flag is counted. All63 audio/assets CTests pass. Evidence:
native-audio-controls-core-synthetic-20261012.json and
native-voice-controls-wire-synthetic-20261012.json.

Renewed installed WAV transport matches362 authored requests and12002990 PCM
bytes with independent Python wave decoding. A version4 native menu/campaign
run passes default voices/guarded WAV/factory/music and normal Quit with intros
skipped,0 unsupported live COM calls/fallback/failed requests. This observes
ordinary game control dispatch, not every newly added pitch/cursor branch or
original physical timing. See native-wave-v4-installed-20261012.json and
native-voice-v4-public-pass-20261012.json. Earlier version3 results retain hashes.

Generated PCM status/cursors remain ahead of the physical sink queue. Static
write cursor equality, frequency bounds and aligned seek are explicit native
policies, not original driver comparison. Physical consumed position, primary/
streaming rings, live output loss/default change and full original comparison
remain pending.
