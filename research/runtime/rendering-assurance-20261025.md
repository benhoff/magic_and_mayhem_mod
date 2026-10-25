# Native rendering assurance renewal — 2026-10-25

This batch leaves the web UI unchanged. It renews selected implementation and
comparison proofs, reconciles the API/family crosswalk against those proofs, and
keeps original comparison, native integration, live bypass and complete-session
acceptance independent. All executable original comparisons use NoCD SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.

## Renewed component proofs

| Scope | New execution | What was checked |
| --- | --- | --- |
| Owned surface backend | `RA25.surfaces.native` | 5,114 retained original/driver draws, 538,272 native/DC pixel checks, 576 DC phases, 1,182 access steps and 40 native guards |
| Formats and pitch | same surface execution | 1,400 driver draws, 2,800 signed-row imports, 667,800 word/descriptor/presentation checks, 2,150,400 packed bytes and 60 refusals |
| Ownership/recovery model | same surface execution | 11 recorded flip results, 2,128 ownership words, 6,656 palette entries, 864 indices, 16 supplied recovery compositions and 4,480 recovered retry traces; physical lost-draw recovery stays separate |
| Clipping | same surface execution | All 408 retained driver cases, including two partial-write failures, match current native GL outputs |
| Panel pixels | `RA25.panels.original` / `.native` | 7,490 unchanged-original comparisons / 2,160,320 words; 19 separate atomic native refusals |
| Fade caller state | `RA25.fade.original` | 320 original/native executions over 16 owned inputs; full guarded words, GPR/stack, defined flags, FX state and callback results; 256 original-once refusals |
| Direct-word sprites | `RA25.word.original` / `.native` | 256 unchanged-original comparisons / 119,288 pixels, 647 native refusals and the portable host CTest |

The four immutable reports are
[surfaces](rendering-assurance-results-20261025/surfaces-03.json),
[panels](rendering-assurance-results-20261025/panels-02.json),
[fade](rendering-assurance-results-20261025/fade-02.json), and
[word sprites](rendering-assurance-results-20261025/word-02.json).
Each resolves through SQLite, retains its prospective claims and source bytes,
and verifies the original manifest before/after execution. Parent reports retain
the child reports and command diagnostics. Standalone driver replay receives
native-integration credit only; it is not a new original-game comparison.

## Reviewed category mappings

[The new crosswalk](rendering-coverage-scope-assurance-20261025.json) preserves
the twelve required surface categories, nine drawing families, conditional
backlog and all historical World/body-bypass records. The earlier crosswalk is
unchanged. Fresh driver-subset proofs now link the supplied format, copy, fill,
palette, clipping, overlap, lifetime, flip and bounded recovery mechanisms to
their categories. Fresh panel/fade/word comparisons renew only their selected
locked-writer, sprite, UI and transition subsets. The current N19 terrain
submission proof is separately linked to the existing ordinary-terrain subset.

Some support in a category does not close the category. Unknown callers,
unobserved writers, alternate-canvas ownership, complete producer reachability,
physical loss/restore, game-session parity and sustained performance stay open.
No original-comparison or live-replacement status is promoted by a mapping edit.

## Retained acquisition failures

The shared default working executable had a different hash from the required
immutable NoCD image. Initial panel/fade/word attempts correctly refused it.
The batch-local component fixture now reads only the immutable executable.
The first surface attempt rejected noncanonical compiler dependency spellings;
the producer now canonicalizes them. The second failed before producing native
output when the shared `/tmp` quota prevented reliable Xvfb startup. A private
RAM-backed `/tmp` admits the unchanged renderer; all surface comparisons pass.
Both surface failures and
[the initial fixture refusals](rendering-assurance-results-20261025/initial-fixture-refusals.json)
remain retained. These setup failures do not establish renderer defects.

The live owner runner now accepts `--no-native-music`, forwarding the existing
supported presentation option and recording that choice. The default is
unchanged. This permits rendering validation without a prepared CD music
catalog; it supplies no audio validation. Early live attempts separately retain
the missing catalog, missing installation manifests and retail-versus-NoCD CD
configuration admission failures. The isolated source installation now verifies
all three required NoCD installation files before launch. Original media and
the user's working installation remain unchanged.

## Execution and remaining acceptance

The [strict native-only journey](rendering-assurance-results-20261025/native-only-campaign-04.json)
passes 615 balanced Worlds, 9,823 native optional-producer calls, 19 exercised
routes, zero forwarding and zero surviving graphics owners, followed by original
exit zero. Its software engine cadence is approximately 12 FPS; this functional
journey does not satisfy the independent performance targets. Movies and native
music were explicitly disabled. Earlier staging attempts remain retained.

The [actual failure/restart journey](rendering-assurance-results-20261025/native-only-restart-01.json)
passes the complete 13-event transition sequence: two different live World
sessions, one producer failure, 534 ms of stable terminal cleanup, a fresh
launcher/channel/generation, then five balanced Worlds and normal Quit. The
cancelled generation is marked restarted and receives no completion credit.
These records promote only the appropriate native-policy integration stages to
live observation. Original pixel comparison and live replacement stay separate.

The full [software](rendering-assurance-results-20261025/software-sessions-02.json)
and [NVIDIA](rendering-assurance-results-20261025/hardware-sessions-01.json)
cohorts reproduce the same result independently:

| Reference | Frames per backend | Result |
| --- | ---: | --- |
| Quick Battle | 323 | Every native GPU and displayed-output comparison matches |
| Explicit corrected-upload storage | 222 | Every native/source/default/physical comparison matches |
| Unmodified storage | 238 | Retained ordinal 170 discrepancy: 126 GPU plus 126 physical pixels |

The [fresh acceptance result](rendering-assurance-results-20261025/correct-output-acceptance.json)
passes the existing user-selected `critical-rendering-correct-output-v1` policy.
It accepts correct native output against the explicitly corrected reference on
both backends and preserves the unmodified Wine artifact as a nonblocking
diagnostic for that policy. It does not assert parity with the unmodified
reference, change any pixel/ordinal/tolerance, or accept the whole renderer.
The earlier unmodified-reference failure is reproduced, not repaired or erased.

The acceptance CLI now resolves SQLite reports directly and accepts prospective
policy claims. It refuses changed declared sources and does not silently reuse
component claims as newly declared policy claims. All eleven acceptance tests
and sixteen rendering-category guards pass, including new SQLite/provenance
refusal tests. The three scene/session CTests pass in each backend acquisition.

Reproduce component phases from an owned checkout with the immutable `original/`
input, a pinned NoCD executable at `working/game-nocd/Chaos.exe`, and locally
available verified SQLite evidence:

```sh
python3 tools/working-retention.py run --reserve-gib 0.2 -- \
  python3 tools/test-rendering-assurance.py --phase surfaces \
  --output "$PWD/working/tests/assurance-surfaces-next" \
  --report "$PWD/research/runtime/assurance-surfaces-next.json"
```

Use `panels`, `fade` or `word` for the other bounded phases. Existing child JSON
is bounded interchange under ignored `working/`; the parent writes durable
results directly through the SQLite writer. RAM builds and captures use a
private user/mount namespace to avoid shared temporary-filesystem quotas. The
user explicitly authorized a temporary 88 GiB main-drive reserve for this batch;
the checked-in 90 GiB policy is unchanged. Acquisition remains monitored by the
main checkout's guard. Temporary execution outputs and durable evidence are
identified independently at closeout.

## Evidence retention and source boundary

This owned batch starts at Git `90e9f1e1486eee609dba1eec07cd445f7a18b51c`.
The [raw archive catalog](rendering-assurance-results-20261025/raw-execution-catalog.json)
retains exact file identities and confirms every member was read back and
verified from the SQLite-backed [raw archive](rendering-assurance-results-20261025/raw-execution.tar.gz).
This includes raw observations, caller outputs, native/GPU frame planes, protocol
records and diagnostics from completed attempts, including negative results.
It excludes reproducible builds and reusable installation/Wine-prefix payloads.
Those batch-created RAM outputs are temporary execution inputs; the original
media, source checkouts, fixture inputs and immutable evidence are retained.
Local completion retains verified SQLite backups without publishing evidence.

The [coverage reconciliation](rendering-assurance-results-20261025/coverage-reconciliation.json)
records current independent subset validation in 12/12 surface categories and
7/9 drawing families, with current implementation-stage proof in 12/12 and 5/9
respectively. It retains the full remaining boundaries. Category
support is not a completion percentage and does not pool historical bypasses
into a claim about current source.

To renew the complete recorded cohorts, run the new producer separately with
`--backend software` and `--backend hardware`, distinct `--output` directories
under `working/`, and new `--report` paths. It retains both positive profiles and
the expected unmodified-storage failure without changing the oracles. Evaluate
the resulting bundles with `tools/check-critical-rendering-acceptance.py
--policy critical-rendering-correct-output-v1 --software-report <software>
--hardware-report <hardware> --claims <prospective-policy-declaration>
--report <new-report>`. Native journeys use `tools/test-native-owned-campaign.py`
with the existing `--native-only` and journey options plus the explicit
`--no-native-music` option used here. These commands require the same isolated
source installation and stopped reusable prefix admission as the retained runs.

