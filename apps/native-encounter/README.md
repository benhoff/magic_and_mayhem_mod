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
