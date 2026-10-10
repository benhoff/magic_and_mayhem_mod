# Native portrait rendering and bounded presentation damage

The native command renderer previously reconverted the entire 800×600 output
for every PRESENT, including small portrait/HUD and cursor copies. Each owned
surface now retains two independent optional dirty envelopes: diagnostic RGBA
and leased GPU output. Writes merge their destination rectangles; first
allocation, palette mutation and storage swap force a full conversion. An
unchanged output does not run the conversion shader. All admitted mutations
remain ordered and all pixels must still be defined before presentation.
GPU producer/consumer fences and source/lease lifetimes are unchanged.

Synthetic tests compare complete independent CPU color images against GPU
framebuffers and diagnostic images after each upload/copy/key/palette/swap.
They assert bounded conversion counts, independent cache dirtiness, unchanged
frame reuse and explicit row uploads in all four canonical formats.

The physical smoke restores repeated portrait selection/recentering during
combat and adds an explicit post-victory stress phase. It retains the 20 FPS
phase and portrait averages, 10 FPS windows and two-second stall limits.
Stable face crop [716,507,759,538] is independently compared to original-owned
pixels within one channel value; full World and animation equivalence remain
pending. Game health, mana, positions and balance remain unchanged; the native drawing cadence policy is described below.

Execution validation is recorded below; historical negative evidence remains intact.

## Legacy draw-skipping configuration

Pinned No-CD 40209ca7 config reads at 0x4e4d03 (SkipFrameEvery),
0x4e4d7d (SkipXFrames) and 0x4e4e0b (MaxSkipXFrames) update
0x6e1ef8/0x6e1efc,0x5e1418 and 0x5e1414. The supplied DEBUG values
are 1/1/1. Config parsing accepts zero. Pacing 0x4e3e40 resets suppression
0x6e1f00, uses skip counters when 0x6e1efc and 0x5e1418 are nonzero,
and on the admitted skip branch omits waiting and marks suppression.
The non-skip wait interval is multiplied by(skipEvery+1)/skipEvery.
With 1/1 and nominal 50 ms, this batches two updates around a 100 ms
wait and suppresses alternate drawing. The zero branch uses the nominal
interval directly; zero SkipXFrames also bypasses adaptive skip adjustment.
World 0x46afc0 checks suppression at 0x46b91a and 0x46bb9d after
creature work. Other gates/timing modes and full equivalence remain open.
Confidence: high for these pinned instructions and config mappings; complete
scheduling semantics are not recovered. Read-only disassemblies are retained
under working/tests/native-campaign/portrait-{pinned,world}-disassembly.txt.

The native installation now disables all three skipping controls in both
plain and encrypted CFG/chaos.cfg variants. It validates all variants before
writing, round-trips encrypted bytes and records before/after hashes. The
runner verifies the staged hashes before Wine starts. Selected game-speed
preferences and all other config bytes stay unchanged. This intentional native
presentation policy removes batching; exact original timing equivalence is
not claimed. No original executable instruction is patched for pacing.

## Executed evidence and boundaries

The pre-fix [baseline](native-portrait-baseline-20261010.json) completed 15
post-combat recenter cycles over 41.69 seconds, passing narrowly with a 10 FPS
whole-phase minimum. It did not restore the original negative during-combat
reselection loop. The [renderer-only negative](native-portrait-damage-negative-20261010.json)
restores that loop and adds 22 post-combat cycles. It performs 67.10% fewer
conversion pixels than converting 800×600 for each of its 7072 PRESENTs,
but still fails 9.12 FPS portrait and 9.93 FPS resumed windows. This retained
failure motivated the explicit native no-skip configuration policy.

[CPU profiles](native-portrait-cpu-profile-20261010.json) retain 25-second
99 Hz samples of each private native Qt/Wine pair during post-combat
recentering, with zero lost samples. Original GetTickCount and busy-wait
PCs 0x4e3f89–0x4e3f93 remain substantial. Different gameplay state and
timing prevent treating these as paired benchmarks or hardware gains.

[Native fixture execution](native-portrait-damage-gpu-20261010.json) passes
normal and 1.5× HiDPI independent full-frame comparisons, all 65536 RGB565
values, independent CPU/GPU cache checks and zero ordinary readbacks/uploads.
Separate incremental regression passes 354 complete frames across 32 command
partitions and 30 rejection/error cases. Fourteen smoke report tests reject
missing inputs, changed face images, fallback, incomplete timing and slow
windows. Three configuration tests cover exact byte preservation, idempotence,
wrong-section/missing/duplicate/malformed keys and both-copy preflight failure.

The [passing live result](native-portrait-passed-20261010.json) binds all
prospectively declared sources and unchanged hashes. A native Zombie cast
creates slot 9 with independent 0/15→1/15 control counts; combat Zombie slot 10
deals 12 original melee damage events to enemy Redcap slot 4. Cornelius
finishes the same target 8→0 in original melee event 2485. The helper restores
recentering during combat and completes 22 post-combat cycles over 61.18 seconds.
Its 56.375 seconds of complete interior samples average 45.52 native/32.83 paint
FPS, with minimum 16.53 FPS; whole-phase gates also include the capture
boundaries. Stable face pixels differ by at most one channel value.

| Phase | Seconds | Native FPS | Visible FPS | Worst window FPS |
| --- | ---: | ---: | ---: | ---: |
| Cast, combat and portrait stress |132.540|41.39|30.44|16.41|
| Mini Cancel/resumed gameplay |63.762|58.56|38.66|17.04|

The session completes 9443 native frames/6608 visible paints, with zero recovery
or original-window fallback. Conversion touches 1,744,502,160 pixels rather
than 4,532,640,000 whole-frame pixels:61.51% less conversion work for this
actual command sequence, including initial/menu frames. This measures work,
not a paired wall-clock speedup. Both fixed 20 FPS averages and 10 FPS window
floors pass without rounding or threshold changes. All 2927 original files
verify before and after. Both staged config variants preserve unrelated bytes
and the game-speed preference; no health/mana/position or original instruction
writes alter gameplay. Precise scheduling equivalence, other maps, animated
World pixels, native GDI, hardware and complete drawing replacement remain open.

The retained raw [combat trace](native-portrait-passed-20261010.bin) matches the
executed trace SHA-256. [Native face image](native-portrait-native-20261010.png)
and [original-owned image](native-portrait-original-20261010.png) preserve the
independent diagnostic inputs; moving World content is not pixel-equivalent.

## Combined ranged follow-up

[Fresh combined gameplay evidence](native-campaign-fireball-portrait-passed-20261010.json)
adds player Fireball damage and invalid-target/insufficient-mana UI refusals to
the same strict native portrait route.22 post-combat cycles over61.48seconds,
a living126HP wizard after ordinary retreat, independent stable face pixels and
60seconds resumed input pass. This uses the current V2 observer/capture retry
sources. Earlier source hashes, failures and CPU profiles remain historical.

## Stationary HUD wait regression and native cooperative policy

The final V3/normal-Quit run retains [a strict failure](native-campaign-final-portrait-slow-20261010.json):
25portrait cycles over61seconds, but57.563seconds of complete interior samples
average16.712FPS for both native publications and paints. Whole phases pass;
normal original Quit exits0. The [profiled reproduction](native-campaign-final-portrait-profiled-slow-20261010.json)
repeats16.705FPS. Removed speculative scene clicks leave the pointer over the
wizard HUD; earlier active-cursor evidence cannot establish stationary throughput.
Native/original portrait images agree visually, including the hovered face layout.

[Owned20second99Hz evidence](native-portrait-wait-profile-20261010.json) retains
4,692samples with zero losses and exact Qt/Wine process IDs. Original busy-loop
PCs0x4e3f89..0x4e3f93 and kernel32 GetTickCount dominate the original main thread;
software Mesa also consumes CPU. Installed Wine11.16 GetTickCount at image offset
1b480 reads DWORD[0x7ffe0320]. Wine's historical [shared-clock discussion](https://www.winehq.org/pipermail/wine-devel/2020-August/171503.html)
explains possible coarse updates, without proving this installed build's exact
scheduling. These are unpaired CPU samples, not a causal native rendering speedup.

The native cooperative policy guards the six-byte original import load,
all12busy-loop bytes, base0x400000 and actual loaded GetTickCount identity. Only
returnPC0x4e3f8b in a configured live native session yields Sleep1 before forwarding
the original clock. Entry/exit LastError and actual return bits are preserved;
other callers forward immediately. Game speed, frame limit, wait target and
pacing instructions are untouched. Atomic close/refusal/queue-failure/end/stop
flags disable yields without touching concurrently unmapped channels. PE32
byte/caller/state/clock/LastError fixtures pass before live use. This intentional
hosting policy needs live validation; precise scheduling/physics/balance
comparison and hardware/replacement equivalence remain pending.

## Stationary portrait failure and rejected yielding prototypes

The final stationary hovered-portrait workload exposes a gap in the earlier
cursor-heavy pass. Two source-stable full workflows finish casting/combat and
normal Quit but fail the unchanged 20 FPS portrait average at 16.7121 and
16.7054 FPS. Their complete reports and original gameplay/spell/menu traces are
retained as `native-campaign-final-portrait{-profiled,}-slow-20261010.json`.
The exact owned 20-second CPU profile has 4,692 samples, zero lost samples,
and 541 direct samples at the pinned original busy-wait instructions. These
are original wait observations, not evidence of native renderer time or a causal
speedup. Installed Wine11.16 kernel32 GetTickCount reads shared DWORD0x7ffe0320;
full original clock and scheduling equivalence remains unestablished.

The guarded Sleep1 prototype triggers native GAP and fallback after 8 seconds;
its cross-thread surface history is explicitly refused. Sleep0 avoids that GAP
in a later bounded run but still produces 16.6946 native/paint FPS over57.264
interior portrait seconds. Twenty-five cycles and genuine Fireball110to90 are
observed, then the original wizard dies after resumed random physical orders.
The smoke misses native Main in stage7 and times out. The failure guard now
includes that stage. Neither prototype is a portrait performance fix. Sleep0
remains experimental opt-in (`--enable-pacer-yield`), disabled by default.
Historical Sleep1 fixture evidence is preserved separately; real timing,
threading, complete surface history and physics equivalence remain pending.

A separate failed Sleep0 aiming run retains native/original overlap images:
Redcap is above the wizard and none of the original five bounded aim points
produce a Fireball cast. Higher points are added, retaining the two actual-cast
limit; the subsequent failed portrait run observes the required original
Fireball damage. No health, mana, position or AI values are written.

## Real higher-resolution uptime policy

The rejected yields led to a separate real-clock probe. In1024 standalone
Wine11.16 samples separated by Sleep1, original GetTickCount advances67 times
in16/17ms steps; loaded WinMM timeGetTime advances1023 times in1/2ms steps.
Its measured uptime stays0..16ms ahead of the coarse clock. Synthetic tests
also cover DWORD wrap, both32ms admission boundaries, disabled forwarding,
and preservation of original entry/exit LastError. The raw four-DWORD records
and source-bound report are retained in `native-precise-clock-fixture-20261010*`.
This is standalone host evidence, independent of original game execution.

The native command route now requests a guarded real WinMM uptime clock.
The original import identity, six-byte load and complete12-byte wait loop must
match; loaded timeGetTime and a shared uptime epoch within32ms are required.
Calls through that original import still invoke original GetTickCount to retain
its error-state contract, then return actual timeGetTime milliseconds. Values
are never fabricated, offset, scaled or frozen; natural DWORD wrap remains.
The original wait instructions/target and selected game-speed preference stay
unchanged. The finer clock stays fixed for the admitted process lifetime, even
when native rendering closes, so shutdown never switches between clock epochs.
Full original timer, animation, physics and threading equivalence remains open.
`--disable-precise-clock` retains the original coarse clock for diagnostic runs.
The separate Sleep0 policy remains opt-in and is not required for this clock.

The resumed phase after successful combat now uses camera keys, rotation,
scene hover, HUD selection and wheel input for60seconds without additional
random ground orders. The first phase retains real summon/ranged/melee/blocked
casts and ordinary movement. This isolates presentation stress from the
previous low-health wizard being ordered back into combat; the report marks
`camera_only` explicitly. A natural defeat remains a distinct unvalidated
menu branch, and any premature native Main/Defeat/Result remains a failure.
Live workflow results are recorded separately below when available.

## Executed precise-clock combined workflow

`native-precise-portrait-passed-20261010.json` prospectively binds the current
clock/renderer/menu/input/test sources. Native menu entry and original World
readiness pass, followed by two ordinary Zombie summons, real Fireball100to80,
seven observed player-Zombie melee HP losses and the wizard's finishing hit on
the same enemy. Invalid target and insufficient-mana cases spend no mana/create
no Zombie. Twenty-five stationary portrait recenter cycles span61.296seconds;
56.477interior seconds measure20.0081native and painted FPS, worst19.8413FPS,
and stable face maximum channel error1 against independent original pixels.
The average has little margin above the fixed20FPS floor; this is a bounded
software-Mesa observation, not a hardware or long-session throughput guarantee.

Resumed camera/HUD/wheel stress spans60.431seconds, then native Mini Quit with
original No, resumed World, Yes, native defeat Continue and native Main Quit
ends the launcher normally at0. Whole initial/resumed native rates are30.959/
57.701FPS and painted rates25.959/37.885FPS. All unchanged20average/10window/
2second stall gates pass, with8642native frames,6499paints, zero recovery and
no fallback. Both immutable manifests pass and declared sources stay unchanged.
This renews only the stated scenarios; Cure and generic repeated Mini Cancel
are independently checked. Original timing/physics, natural defeat, full surface
history, complete GDI/drawing/simulation replacement and other hardware remain
outside the result. Earlier negative reports and their hashes remain intact.
