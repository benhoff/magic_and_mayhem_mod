# Twentieth bounded rendering batch (2026-10-24)

Three existing partial implementations now have current-source execution proof
for their selected offline scopes. Understanding remains partial, original
comparison remains recorded, and integration/replacement remain none. This does
not close the original world, allocator, special-effect or scene aggregates.

- `RE.terrain_map`: 2053 synthetic maps/945562 cells and three installed maps/76800 cells match original whole-cell output; native atomic/refusal/projection controls and installed sanitizer comparisons pass.
- `RE.palette_preferences`: 3724 VIDEO admission/loader-dispatch cases across 14 prior unsigned values and 63 GLOBAL_OPTIONS cases match the original; strict native repeated-sign refusals pass with a retained pre-fix counterexample.
- `RE.palette_shading`: 510 RGB565/RGB555 palettes at all counts 2..256 match 16776192 original words; 252 odd-count allocation defects remain original-only. Native selector and guard tests pass in normal and sanitizer builds.

The parser fix refuses a second sign after a leading plus. Its shared template
serves LightCurve, ColourFactor, LightPower, ColourPower, AmbientLight,
LightRamp and TerrainLightLevels. All seven fields receive malformed-sign tests;
a malformed final option returns an error rather than publishing partial fields.
`NP.palette-option-strict-parsing` records intentional native strictness with no
original comparison credit. A missing VIDEO setting preserves any prior unsigned
count, including a value outside the builder's supported 2..256 domain; builder
admission remains an independent guard.

Map controls cover late invalid definitions (including every top-layer position),
invalid geometry/catalog/extent refusal before mutation, metadata/source ownership,
reference/object projection counters, all four owner bits and toroidal edge masks.
The original fixture adds 1x1x32, 128x1x2, 1x128x2, 128x128x2 and 128x128x32 cases.
Installed Forest sections CFsec01/02/38 are read-only inputs. Projection/base bytes
come from the independent existing Python transcription; whole-cell surface bytes
are compared to unchanged original 0x4ecf20 instructions.

Palette controls keep every check active in Release builds and verify every shift
0..7 over -32768..32768 (524296 selector cases), signed-32-bit extremes, invalid
chains/counts/levels/powers and complete owned outputs. Original 0x582440 body
bytes are unchanged; a byte-checked private allocator adapter observes the 508-byte
odd-count underallocation with bounded extra storage/canaries. The reused boundary
fixture also executes 3267 camera checks incidentally; this batch claims no new
camera behavior or status. Native complete allocation and checked refusals are
policies, not assertions of the original unsafe allocation/failure behavior.

## Evidence and reproduction

[Execution 2](rendering-twentieth-three-results-20261024/execution-2.json) holds
prospective contract claims, exact compiler dependency/source pins, input hashes,
commands/log identities, six normal/ASan/UBSan controls and all original comparisons.
[Release controls](rendering-twentieth-three-results-20261024/cmake-release-controls-final.json)
record three passing CTests from the standard terrain-preview project. Original
manifest bookends verify all 2927 immutable inputs; the PE hash is
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
No original/installed binary, Wine prefix, game balance or GPU path is changed.

Run `tools/test-rendering-twentieth-three.py` with new absolute `--output` under
this checkout's `working/tests`, new `--report` under `research/runtime`, and the
existing read-only installed game `--asset-root`. Launch through the main
checkout's retention guard. The tool uses its own TMPDIR, derives only three MAP
interchange fixtures, compiles native/sanitized and PE32 private-mapping controls,
and writes success or failure directly through the SQLite report writer.

The retained [pre-fix counterexample](rendering-twentieth-three-results-20261024/parser-regression.json)
fails with the new test against [exact prior source](rendering-twentieth-three-results-20261024/pre-fix-parser.bin).
The first compile hit the host /tmp quota and is retained separately as an
[environment failure](rendering-twentieth-three-results-20261024/parser-build-temp-refusal.json).
[Execution 1](rendering-twentieth-three-results-20261024/execution-1.json) stopped
at a warning-as-error from adjacent test loops before any original routine was
called; its exact [pre-format source](rendering-twentieth-three-results-20261024/pre-format-shading-test.bin)
is retained. Existing output-directory refusal prevented accidental reuse during
the regression retry; the new directory uses worktree-local compiler temporaries.
All generated research reports and source variants are verified SQLite references.

## Remaining boundaries

`RE.terrain_map`: Original whole-world production, allocator/failure/update lifecycle, object/entity admission and complete scene/live replacement remain separate from selected owned ordinary geometry.

`RE.palette_preferences`: Malformed original atoi/atof conversion is not equated to strict native parsing. File-loader/installed configuration integration, special appended effects, full scene/allocator ownership and live replacement remain separate.

`RE.palette_shading`: Broader nondefault floating-point and special-effect/ordinal dispatch, original unsafe allocator/failure lifetimes, complete scene integration/live replacement and unresolved candidate 0x57e1b0 remain separate. The mode0..9 aggregate remains partial.

Historical image and installed-wide results retain their original source hashes.
They are not refreshed or promoted by this bounded renewal. Shared parser/model
source edits can stale other evidence; those historical stages remain pending their
own comparisons. Full original-world production, nondefault floating-point
acceptance, driver/resource lifetimes, full scene pixels, live replacement and FPS
are outside this batch. The new parser policy remains unrated in planning.

Durable evidence and exact unique failed sources are in the pinned SQLite store;
compiler binaries/logs, decoded MAP interchange and standard build output are
retained temporary execution outputs under `working/tests`, not new installations.
Local completion retains verified SQLite backups before committing; no evidence
publication or remote source push is requested.
