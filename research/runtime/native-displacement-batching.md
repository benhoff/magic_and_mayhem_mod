# Bounded native displacement batches

This changes native presentation only. Original simulation, combat, AI, actor
movement, game clocks, executable patches and balance are unchanged.

Consecutive destination-displacement sprites formerly froze and sampled native
storage once per sprite. The scene renderer now composes coordinate mappings
from the owned decoded masks and row-offset tables. All samples for a sprite
resolve against the mapping before that sprite; only then are its writes applied.
Thus overlapping sprites preserve each draw's frozen-history semantics. The
mapping contains coordinates, never framebuffer colours or original output.

Sorted final mappings become disjoint row spans. Identity mappings need no write.
A single GPU snapshot covers their source envelope; the existing integer copy
shader writes spans in blocks of at most4096 instances. Final samples always
address that distinct frozen texture. There is no framebuffer feedback or ordinary
readback. All sample extents, ordering, nonoverlap, total65536-pixel budget and
fully defined RGB565 destination are checked before mutation.

Admission is at most32 consecutive sprites, each at most128x128, with at most65536
visited source pixels and the caller's existing draw/time budget. Hidden draws,
other primitives, unsupported resources and larger sprites end a batch. A single
sprite retains the previous path. Whole-run clipping also retains that path when
horizontal clipping would split a sprite into separate ordered operations.
Vertical clipping, absolute row phase, tight linear row crossings and the explicit
retain-outside-canvas policy are preserved. Software rasterizers (llvmpipe, softpipe and named software rasterizers) and
isolated renderer services retain the
existing serial wire/recovery path. No new service opcode or runtime hook exists.

Prepared-only scenes can compose owned displacement masks without uploading
unused sprite colours; the usual upload protocol remains for every fallback.
The renderer's existing partial-frame presentation refusal remains in force.

## Validation boundary

`tests/displacement-batch-test.cpp` compares complete images against both an
independent frozen-image scalar implementation and the existing serial renderer.
It covers overlapping masks, signed origins, varying row periods and offsets,
viewport and canvas clipping, retained-outside samples, copy/hidden barriers,
caller budgets, prepared uploads, whole-run fallback, multiple instance blocks,
undefined storage and late invalid-span refusal. Existing colour, atlas, upload,
scene and retained-image regressions remain separate controls.

`tests/combat-world-replay.cpp` compares100 full frames in serial/batched/batched/
serial order using the retained heavy World2274 source and the same authored
nonconstant RGB565 background. Both policies keep the previously accepted small
colour batching enabled. Exact source bytes have SHA-256
`58873c3ba289c2defd11020c816b77619561063ebbbf0335ce5761108965df75`.
The complete output remains
`6b53563bcc7377224bffc54df90fcb2f01d5aef1880997476239e3251b33bf25`.
The4096-span candidate reduces596 draws to484 and snapshot pixels from1589057 to
1361966. The final reversed-order M1 run averaged55.09ms serial and43.79ms
batched completion (about20.5% lower). Software fallback remained on the same
serial path, averaging49.04ms versus49.20ms. Exact measurements and
executable/source identities are in `NATIVE12.displacement-controls02`,
`research/runtime/native-displacement-results-20261112/controls02.json`.
These numbers are host/workload observations.

An ungated4096-span run exposed a software-Mesa regression (about84ms versus
50ms serial), so software rendering keeps the established serial scene path.
This negative result is retained, not a performance success.

The earlier512-span prototype was also pixel-identical, but submitted584 draws.
Fixture recipe/layout setup failures are retained as negative development evidence.
All diagnostic readbacks occur after ordinary submission, for verification and
completion timing. Timing includes that synchronization, not just GPU execution.
Warm frozen-frame results do not establish dynamic-upload cost, original image
history, live battle FPS, normal-Quit acceptance or physical scanout. Historical
shared-source evidence retains its original hashes and pending applicability;
this change does not promote unrelated original-engine milestones.

Raw observations, exact sources/scripts/tested executables and failures are in
`working/heavy-native-render-20261112/results.sqlite3`, with verified local SQLite
backups. Compiler intermediates and build trees are reproducible temporary data;
tested binaries and the retained World input remain durable evidence.
