# Protected external source snapshots

Capture selected source bytes after accepted read-only lease acquisition and after
checked worker completion, before restoration. Compare the complete byte planes,
retain exact source/API/lease/command identities and refuse incomplete or changed
versions. Original snapshots stay external comparison oracles. Fixtures precede
original execution. Other routes, hardware completion, cadence controls and full
semantic acceptance remain pending.

`MNM_DRAW_SOURCE_HELD_SNAPSHOT=1` requires the worker-completion lease. The
recorder copies the first plane immediately after accepted protection and lease
arming, then copies the second after checked command completion, before the
Scene restoration call. Both calls execute under the journal mutex; the original
API/worker bodies and bounded wait execute outside it. File-copy overhead changes
diagnostic cadence. Original pixels never enter native replay input.

The recorder requires full 800×600 RGB565 geometry, stride 800, identical known
live metadata before/after each copy and matching canvas/API/lease across the
pair. The second plane must match every byte of the first; copy, identity or
byte mismatch refuses with exit 99. The Scene lease independently checks all
covered regions remain read-only before restoring the owner’s original pages.
This does not establish whole storage lifetime or every possible alias writer.

`source-held-snapshot.bin` uses `MNMSHB01`, a 64-byte header and 192-byte rows.
Header words 2–7 are version 1, row size 192, build tag `0x40209ca7`, maximum
2048 rows, maximum 1024 planes and 1 GiB plane budget; words 8–15 are zero.
Row words 0–12 are ordinal, phase (1 after acquisition, 2 before release), API
entry, owner thread, GetTickCount, source and destination API IDs, copy result,
plane ordinal, byte count, lease ID, emitted commands and completed commands.
Words 13–15 are zero; words 16–31 and 32–47 hold before/after source-point V1
metadata. Phase 1 counts are zero; phase 2 requires a positive equal count.
Phase 4 is terminal, with stop reason in word 2, plane count and total bytes
in words 8–9; other payload words are zero. Planes use
`source-held-oracle-NNNNNN.565`. Decoding crosschecks both lease journals and
requires pair hashes to match, keeping hardware completion separate.

The four-case PE32 fixture verifies actual read-only source pages during each
copy, exact plane bytes, restoration before a subsequent write and refusal on
byte mismatch, changed canvas and failed copy. Five malformed decoder checks
also refuse. Synthetic permission/copy order is separate from original-game
observation and native comparison.

Original run 01 closes 119 accepted pairs (238 planes) with identical bytes
through 224 checked worker returns and no observed protected write. Source-only
native prediction matches 104 selected physical witnesses (239 excluded), and
external held-snapshot comparison matches 208 selected planes / 99,840,000 pixels
with zero differences. The original observer retains 343 API and 342 post-EGL
physical witnesses. Other snapshot pairs are outside this restricted native
comparison cohort; no complete equivalence is asserted.

Reassessment: the next acquisition increment uses the existing engine cadence
observer to retain actual counter-advancing update identities, then extends the
owned source prefix toward 120 contiguous windows. A World draw return or a
passive gameplay snapshot is not sufficient update identity. Cadence controls,
whole lifetimes, other routes and full semantic journeys remain pending.
