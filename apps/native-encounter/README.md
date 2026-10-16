# Installed native encounter

A bounded, headless two-Crocodile encounter using the owned native command session
and selected original CFG/HTH/ANI/MAP/TTD data. It runs without Wine or Qt.
Navigation, autonomous acquisition, original action/ANI timing, ordinary combat,
stun/knockback and physical death share staged state and portable snapshots.
See `research/runtime/native-installed-encounter.md` for evidence and boundaries.

Build with CMake from this directory, using the repository storage guard before
producing build output. The `native-encounter` target has these commands:

```text
native-encounter run INSTALL X Y WIDTH HEIGHT SEED TICKS NEW_SNAPSHOT
native-encounter resume SNAPSHOT TICKS NEW_SNAPSHOT
```

The validated crop is0,0,12,12 of Celtic Plains CPsec01. Seed17 demonstrates
navigation, fighting, stun and death. Seeds42 and91 add knockback and other AI
outcomes. Each invocation admits at most8,192 ticks. New snapshot destinations
must not already exist. Resume reads its entire immutable profile from the native
snapshot; no installation or original executable is accessed by that command.

`tools/test-installed-encounter.py` builds/runs the native and sanitizer corpus,
five core CTests, combat rollback and ten fresh-process resumes. Its output
includes a bounded published-state trace. `tools/test-encounter-trace.py` compares
selected original bodies against that trace, given its successful source-bound
producer report. The original32 oracle build is an offline Linux/i386 research
tool; it is separate from the runtime executable. Neither application imports a
campaign, allocates dynamic actors, implements original World services or changes
creature balance.

`tools/test-paired-encounter.py` runs a persistent independent original32 creature
core beside native32, then checks native64 and sanitizer replay, fresh-process
checkpoints and a reproducible first divergence. See
[`paired-native-encounter.md`](../../research/runtime/paired-native-encounter.md)
for the exact projection and empty World boundaries. The oracle reconstructs its
checkpoint prefix by replay; native snapshots restore directly.


`run-visibility INSTALL X Y WIDTH HEIGHT SEED TICKS NEW_SNAPSHOT` enables
recovered creature perception on the generated regional graph, feeding owner
memory, danger and navigation. Use the32x32 crop at0,0 for the NS77 corpus.
Visibility, shared sightings and observer identities continue through portable
save/resume. `run-knowledge` retains supplied visibility inputs; `run-danger`
retains the earlier native observation policy. The new perception mode admits
ordinary type17 actors and empty object/attenuation columns. It does not own
observation-point fog, local UI reveal or the full World/campaign driver.
See [owner visibility](../../research/runtime/owner-visibility.md) and
`tools/test-owner-visibility-integration.py` for reproduction and evidence.


`run-planner INSTALL X Y WIDTH HEIGHT SEED TICKS NEW_SNAPSHOT` enables the NS78
bounded strategic planner. Use crop0,0,32,32. The setup chooses ordinary legal
stands in one region; sightings produce a singleton task6 candidate, installed
Brain.dat inference selects it against fallback, and its native task continues
orders. CMD1v12 embeds the model and independent strategic RNG; `resume` needs
only its snapshot. Candidate path summaries, weights, cadence and active-task
retention are explicit lab policies. Multiple-memory grouping, original probe
production/risk rejection, other task families and full World AI remain pending.
See [owner strategic planner](../../research/runtime/owner-strategic-planner.md)
and `tools/test-owner-planner-integration.py` for the comparison boundaries.

Fresh `run-planner` sessions now use the NS79 grouped owner adapter: anonymous
memory groups, ordinary regional candidate probes, produced path/danger features,
installed model inference and risk-aware pursuit. CMD1v13 preserves unfinished
group/probe/best/active task work; old v12 checkpoints retain their singleton
policy. See [scope and evidence](../../research/runtime/owner-grouped-planner.md).
The full original owner scheduler, task replacement and other strategic families
remain outstanding.
