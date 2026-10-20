# High tranche 1: terrain execution (2026-10-19)

Execute the seventeen bounded terrain/palette/light contracts in the High plan.
Original comparison, native scene checks and live replacement remain separate.
The selected original build is `40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.

## Reproducible entry point

`tools/test-rendering-high-terrain.py` runs the existing component comparisons
with prospective behavior/source claims, a frozen source root, normal and
ASan/UBSan preview/recipe-inspector builds, retained failures and manifest
bookends. Run it inside the storage guard; `--help` names the required paths.
`--behavior` selects bounded retries, and `--jobs 1..3` bounds concurrency.
No missing asset, mismatch, missing report or ambiguous output directory passes.

The installed launcher executable differs from the comparison build. The
`MNM_TERRAIN_EXECUTABLE` input override selects an isolated verified copy for
these runners while preserving each original hash check and immutable-input
verification. The installed launcher tree is preserved. Source provenance
includes this input-selection helper, component wrappers and compiled models.

## Validation repairs and retained refusals

- The default installed executable hashes to `1060a739ae220f4dfcf0e86b848df229fd5e37ab253d3d000e39ca2601c6c4cb`, so the initial preflight refused it. A byte-verified isolated copy of `original/Arcane_Nocd/Chaos.exe` supplies the supported build; both input preparations retain manifest bookends. The installed launcher was preserved.
- The sandbox refused 32-bit compatibility execution (including an observed `SIGSYS`). The exact same selection helper completed outside the sandbox under GDB: 262,144 RNG words, 207,840 admissions and 78,584 choices matched. Subsequent original comparisons use the required host execution permission.
- Optimized GCC 16 and GCC 15 preview builds stopped on a Qt `qendian.h` array-bounds warning expanded from the shared command decoder. Strict-warning Debug (`-O0`) and ASan/UBSan (`-O1`, non-PIE) GCC 15 builds completed. This is no optimized-build or performance acceptance claim.
- The region oracle matched all 131,072 native RGB565 words, while its obsolete floor-scaled RGBA expectation differed in 146,874 bytes. The existing [native presentation policy](surface-dib-ddraw.md) uses bit replication and matched every RGBA byte. Six component wrappers now check that documented policy separately; their exact original WORD comparisons remain unchanged. An exhaustive 65,536-word check and a negative old-floor control cover the expectation helper. This is no new original-driver RGBA comparison.
- Parallel `xvfb-run` wrappers sometimes returned failure during cleanup after completing their scene comparisons. Those failures remain failures. Private Xvfb processes now allocate display numbers through `-displayfd`, with per-case environments and cleanup limited to the owned process. Three simultaneous servers and controlled-exception cleanup pass. The exact cause of the earlier server exits is not established by this repair.
- Initial CTest discovery exposed an unbuilt terrain asset-test executable. That Not Run result is retained. After building the missing target, all 20 discovered terrain/palette tests pass in each of the normal and sanitizer builds.

No recovered terrain algorithm or renderer presentation policy was changed by
these harness repairs. The final comparison outcomes are recorded separately.
Missing installed recipe sections and other historical excluded domains remain
explicit; no substitute assets or live-replacement claims are supplied.


## Build and run

Use a new frozen copy of the selected sources and new build/output directories.
The retained `source/`, compile databases and build logs under
`working/tests/rendering-high-terrain-20261019/run-001/` identify the exact inputs.
Build `apps/terrain-preview` with `BUILD_TESTING=ON`, including
`mnm-terrain-preview`, `mnm-region-recipe-inspect` and all selected terrain/palette
test executables. Use GCC 15 Debug for the ordinary preview and
`-fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie` with linker
`-fsanitize=address,undefined -no-pie` for the sanitizer build. Original comparison
helpers remain separately compiled by their component runners.

Prepare prospective declarations before execution with
`tools/draft-coverage-claims.py` for the 17 behavior IDs in High tranche 1. Select
an isolated, hash-checked `Chaos.exe`. Invoke the storage guard with an 8 GiB
estimate and `python3 tools/test-rendering-high-terrain.py --executable ... --claims ...
--source-root ... --normal-build ... --sanitized-build ... --output ... --jobs 3`.
The tool retains every command and component report hash. It does not launch a
game or change installation/prefix state. Model comparisons, software scene
checks, native safety refusals and remaining original domains stay distinct.

## Recorded result

All 17 selected tasks pass with stable source/build fingerprints and immutable-input bookends. Raw artifacts, prospective declarations, earlier failures and compiler/build receipts remain pinned under `working/tests/rendering-high-terrain-20261019/run-001`.

| Contract | Result | Selected counters |
| --- | --- | --- |
| `RE.terrain_map` | PASS | `{"images": 96, "maps": 683, "original": {"all_match": true, "installed_cells": 7595200, "installed_maps": 683, "synthetic_cases": 2048, "synthetic_cells": 387962}}` |
| `RE.terrain_sections` | PASS | `{"images": 96, "maps": 683, "original": {"all_match": true, "installed_cells": 30084800, "installed_maps": 683, "synthetic_cases": 8192, "synthetic_cells": 410500}}` |
| `RE.terrain_region` | PASS | 3,136 original cases; 46,656 blocks; 2 installed regions; 32 scenes; 40 decoded recipes; 12 retained refusals. |
| `RE.terrain_catalog` | PASS | `{"maps": 564, "original": {"all_match": true, "cases": 2087, "decoded_recipes": 40, "descriptors": 26888, "installed_catalogs_matched": 39, "installed_recipes": 40, "native_refusals": 1, "refusal_reasons": {"Region catalog MAP header": 1}, "threshold_quirk_cases": 291}}` |
| `RE.terrain_selection` | PASS | `{"maps": 616, "original": {"admission_checks": 207840, "all_match": true, "candidate_tables": 4096, "choices": 78584, "installed_descriptors": 1232, "location_tables": 10240, "native_rotation_mask_checks": 831360, "rng_seeds": 256, "rng_words": 262144}}` |
| `RE.terrain_constraints` | PASS | `{"maps": 683, "original": {"accepted": 18150, "admissions": 39344, "all_match": true, "candidate_tables": 7200, "choices": 7200, "connector_searches": 2048, "installed_descriptors": 1736, "installed_maps": 683, "location_tables": 500, "prunes": 8192}}` |
| `RE.terrain_solver` | PASS | `{"maps": 683, "original": {"all_match": true, "backtracked_cases": 1940, "installed_maps": 683, "multi_backtracked_cases": 231, "native_refusals": 0, "placements": 3136, "pruning_carries": 2048, "removals": 7056, "solves": 3755, "successes": 2067}}` |
| `RE.terrain_specific` | PASS | `{"maps": 683, "original": {"all_match": true, "cases": 10786, "installed_maps": 683, "multi_block_cases": 2430, "native_refusals": 140, "notices": [1280, 1337, 1472, 1636], "placed_blocks": 24438, "refusal_reasons": {"Region internal connector has no descriptor": 140}, "suppressed_descriptors": 1636}}` |
| `RE.terrain_generation` | PASS | `{"maps": 683, "original": {"all_match": true, "attempts": 19883, "cases": 5701, "exhausted_cases": 1552, "installed_maps": 683, "multi_block_cases": 782, "native_refusals": 17, "notices": [128, 128, 64, 128], "refusal_reasons": {"Region fixed rollback before grid": 17}, "reset_cases": 5718, "retry_cases": 1636, "retry_successes": 84, "seed_wrap_cases": 241, "successful_cases": 4149}}` |
| `RE.terrain_generated` | PASS | `{"maps": 564, "original": {"all_match": true, "cases": 2204, "failed_generations": 1010, "installed_cases": 156, "installed_plans": 155, "installed_recipes": 40, "native_refusals": 4, "plan_blocks": 15849, "plans": 1194, "refusal_reasons": {"Region catalog MAP header": 4}}}` |
| `RE.terrain_camera` | PASS | `{"reference": {"all_match": true, "fixtures": 8192, "state_comparisons": 139264}}` |
| `RE.terrain_traversal` | PASS | `{"cases": 96, "images": 192, "traversal": {"all_match": true, "cases": 18428, "generated_cases": 18432, "margin_visits": 309472, "orientations": [{"cases": 4096, "native_domain_rejections": 0, "physical_alias_visits": 0, "view": 0}, {"cases": 5119, "native_domain_rejections": 1, "physical_alias_visits": 202, "view": 1}, {"cases": 4095, "native_domain_rejections": 1, "physical_alias_visits": 1219, "view": 2}, {"cases": 5118, "native_domain_rejections": 2, "physical_alias_visits": 22, "view": 3}], "records": 3089824, "wrapped_visits": 946449}}` |
| `RE.terrain_submission` | PASS | `{"summary": {"cases": 15204, "records": 15317}}` |
| `RE.palette_preferences` | PASS | `{"admission_dispatch_fixtures": 2112, "frames": 120, "native_frames": 240, "refusals": 10, "successful_fallbacks": 6}` |
| `RE.palette_shading` | PASS | `{"frames": 200, "native_frames": 400, "palette_counts": 8, "refusals": 6}` |
| `RE.terrain_lighting` | PASS | `{"field_fixtures": 564, "final_buffer_byte_comparisons": 3594685, "frames": 36, "kernel_byte_comparisons": 13202112, "native_frames": 72, "original_source_stamps": 1695, "refusals": 18}` |
| `RE.terrain_creature_lighting` | PASS | `{"compared_builds": 3, "output_sha256": "deaa406cbc34e840c121af18072323829acba09a8145534b7ea43f2238bc47fe"}` |

Counts overlap across composed contracts; they are not additive coverage or whole-function completeness.
All 20 native terrain/palette CTests pass in each build. The four harness tests include exhaustive RGB565 expansion, and two display tests cover concurrency/cleanup.
`RE.terrain_traversal` now records an isolated original comparison. Generated-plan composition retains native-policy comparison N/A. Every understanding/implementation/integration/replacement stage is unchanged.
The broader next steps in the dashboard preserve missing installed inputs, original CFG/file/dialog/allocator/entity lifetimes, full world integration, hardware performance and live replacement as separate work.

## Coverage and presentation checks

The accounting audit reports no errors and all 18 new execution records have
current source fingerprints. The 33 planning tests and 21 HTTP/backend tests
pass, including the new tranche-1 result/status-boundary check.

The separate production-browser check was blocked by a concurrent static-review
link mismatch for `NR.directdraw-retirement`: its new execution and scope
resolution JSON inputs are not yet in the global reviewed-path crosswalk. This
is retained in `working/tests/rendering-high-terrain-20261019/run-001/dashboard-blocked.json`;
no presentation pass is inferred from the synthetic HTTP tests. The bounded
engine comparisons and their evidence registration remain valid independently.

Two commits landed during execution. The normalized committed-history review
covers `0dc91365aed634b16c61f31cc8f34b2746eee62a..2c90c3f20760c3025e6c5d58e8c61f0d27a48b1f`:
all 296 changed file versions have exact or composed receipt coverage; normalized
source and affected evidence links have no missing targets or changed-record
hash mismatches. No retrospective receipt was needed. This audit asserts no past
gate pass or fresh validation of those committed engine changes. Uncommitted
tranche-1 changes retain separate exact reviews and gate output.

The incremental history review through `4ca24a4af9ec173c90f31eb35f2b953ea98a8946`
also covers all 16 changed file versions, with no missing receipt pairs or
committed source links. All 64 affected evidence records retain their recorded
hashes. This later reservation-admission work has separate engine evidence.
The [generated audit summary](coverage/summary-high-terrain-execution-20261019.md)
is a dated snapshot; use the audit command for subsequent source freshness.
