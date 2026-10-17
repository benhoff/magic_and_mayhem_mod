# Live GPU-copy validation

Review/execution date: 2026-10-17 UTC. `NP.native-gpu-copy-live-validation`
records a current NoCD native V3 journey through World/HUD interaction, Save,
Load, both display rebuilds, changed-canvas reload and normal Quit. The
[scoped live result](native-gpu-copy-live-campaign-20261017.json) passes;
the [complete acquisition report](native-gpu-copy-live-campaign-raw-20261017.json)
retains its original source inventory and run configuration unchanged.

## Actual callers and branches

All **867 admitted game-side GPU-copy calls** match the native command stream,
with no admission/result errors or destination mismatches. The census captures
the current executable module base and extent; return PCs below are for the
pinned `nocd-40209ca7` build, not stable process pointers across launches.

| Return PC | Existing wrapper entry | Observed calls |
| --- | --- | ---: |
| `0x0058c05d` | `0x0058bf40`, whole opaque-copy wrapper | 443, including 4 full opaque bootstraps |
| `0x0058c488` | `0x0058c360`, opaque rectangle wrapper | 9 |
| `0x0058c5be` | `0x0058c4a0`, opaque rectangle/BltFast wrapper | 415 |

These return sites and wrapper identities already appear in the
[drawing inventory](render-drawing-inventory.md) and binary register. This adds
current native ownership/copy observations, without expanding their recovered
semantic scope. Confidence is high for the counted calls and output checks.

Attached-clipper, aliased/self, keyed, empty-plan and partial-error GPU calls are
all **zero in this journey**. Their implementation remains covered by the
[23 final-source headless cases](native-gpu-copy-plan-controls-20261017.json).
This run does not close their installed-game caller gap. No new renderer failure
or unsupported reader was reproduced, so no new graphics admission was relaxed.

## Independent copy destinations

`--gpu-copy-diagnostics` enables a bounded native-host observer and a separate
scalar PE32 caller census. Both are absent from ordinary launches. The host reads
the actual source and complete destination before a COPY, executes the GPU COPY,
captures the completed destination, then computes the expected image with an
independent CPU loop. Every destination word, including untouched pixels, must
agree. Expected/captured pixels never enter rendering, the World provider, CPU
reply publication or facade history transfer. This preserves stale-reader guards.

The observer checks **1,072 COPY commands / 283,014,400 destination words**, with
zero differences. Its conservative World-ancestry selector also checks copies
after CPU writes; the caller census identifies the 867 actual GPU-route calls
within that larger set. Command IDs and complete COPY sequence ranges are matched
to each census row. For self-copies the validator additionally requires the one
full opaque snapshot before all pieces, though no such call occurs live here.

Sixteen retained source/before/actual/expected cases receive a separate Python
recomputation over **5,288,000 destination words**. Hash changes, equal-but-wrong
expected/actual images, unsafe filenames, missing callers/sequences, incorrect
resources, duplicate records and incomplete/overflowed evidence refuse. The
[focused controls](native-gpu-copy-live-controls-20261017.json) also deliberately
omit a GPU copy and require the native comparison to reject its incorrect output.

There are **3,216 diagnostic host readbacks / 4,209,452,800 transferred bytes**.
These are explicit test overhead. Ordinary World/native/RGBA return readbacks stay
zero; the existing two producer CPU materializations are counted separately.
The run is software functional evidence, without a sustained performance claim.

All 419 Worlds have balanced completion, import and verified-return counts, zero
pending work and acknowledged normal retirement. A successful command END requires
all command surfaces and palettes retired. The census closes without dropped rows
or overflow, and the private Wine server stops. This does not add a semantic COM
teardown, physical context-loss or in-place recovery claim.

## Original World and default-mode regressions

The [independent original-code comparison](native-gpu-copy-live-storage-original-20261017.json)
passes all four captured World checkpoints:

| Phase | Source generation | RGB565 words | Mismatches |
| --- | ---: | ---: | ---: |
| Before Save | 38 | 480,000 | 0 |
| After Save | 100 | 480,000 | 0 |
| After Load | 227 | 480,000 | 0 |
| After rebuild/Main Load | 350 | 480,000 | 0 |

The complete source prefix is replayed through the last checkpoint. Both authored
original baselines also agree at these inputs; that is not general baseline
independence. CPU-reader/sequence controls pass and all replay surfaces retire.
Original manifests verify 2,927 files before/after. The original executable hash
is `40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
The unchanged [full comparison acquisition](native-gpu-copy-live-storage-original-raw-20261017.json)
retains all input hashes. These are selected World rasters, separate from the
complete copy-destination checks and full physical presentation equivalence.

Freshly rebuilt default-mode binaries, with copy diagnostics disabled, pass all
[222 corrected storage frames and exact intermediate witnesses](native-gpu-copy-live-portable-storage-20261017.json)
and [323 Quick Battle frames](native-gpu-copy-live-portable-quick-20261017.json).
Final RGB tolerance stays one channel value and alpha is excluded. The original
238-frame historical baseline is unchanged and was not relabeled or rerun here.
Generic 14-journey/21-operation acceptance and complete replacement remain false.

## Reproduction and retention

Use a supported private installation and its matching
`working/manifests/game-nocd.tsv`, the main checkout's storage guard, a private
display/prefix and original-manifest bookends:

```sh
python3 tools/test-native-campaign.py --command-wire-version 3 \
  --resident-world --profile-resident --capture-resident \
  --gpu-copy-diagnostics --other-pixel-producers --legacy-audio \
  --save-load --recreate-world --normal-quit --difficulty 1 \
  --min-fps 5 --timeout 600 --skip-movies --source-game working/source-game
python3 tools/test-native-storage-comparison.py --campaign <passed-report.json>
```

The passing campaign is retained in the registered second-drive checkout at
`working/worktrees/gpu-copy-live-20261017/working/tests/native-campaign/run-rae3jfa8`.
Its observer experiment is `run-3dz589e5`; it retains the census, command-channel state,
input/flow records and source capture (694,463,146 bytes). The 16 raw pixel cohorts
and all 1,072 hashed comparisons remain under the campaign's `copy-diagnostics`.
The [identity/retention record](native-gpu-copy-live-provenance-20261017.json)
pins the actual source/staged executable, adapter DLLs, observed module extent,
census and every retained native pixel cohort.
See the [diagnostic file contracts](../formats/native-gpu-copy-census-v1.md).

The first launch (`run-vto8dqbd`) stopped before original game entry because the
isolated checkout lacked its launcher manifest. The supported manifest's
executable/JPEG/CD hashes were verified before retrying. An earlier build targeted
the main cache because the retention guard resets child working directories;
the corrected builds use absolute second-drive paths. Both failed builds/setup
records remain retained and receive no passing execution credit.

Scoped evidence keeps rendering/host/runtime/compiler/launcher sources and the
original fingerprints. The complete raw inventory additionally included unrelated
native skirmish/game sources; their exclusion from the scoped record grants no
validation credit to those independent policies. No old evidence hash is refreshed.

## Subsequent active route classification

The [current all-copy follow-up](native-copy-route-reachability.md) observes
4,075 active calls: 1,828 keyed copies all use CPU-backed sources, while 1,666
GPU-source calls remain opaque/disjoint. No active clipped/self call occurs.
The GPU ancestry observer independently checks 1,788 destinations, including no
keyed command; CPU-keyed output equivalence is not added. Seven terminal bypasses
are counted exclusions. The 867-call acquisition above remains unchanged.
