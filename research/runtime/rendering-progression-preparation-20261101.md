# Campaign/progression scenario preparation — 2026-11-01

`NP.rendering-progression-scenarios` prepares EP05's three missing journeys.
The drivers are runnable bounded attempts, with semantic assertions and the
existing independent World-history oracle connected. No live campaign case was
executed in this preparation batch. Successful tooling guards and a linked player
do not establish victory, natural defeat, next-map progression or acceptance.
`GP.progression` retains its existing static understanding and original-owned
implementation boundaries. The subsequent [live execution record](rendering-progression-live-20261101.md)
retains startup/launcher corrections and the first real failed progression
attempt's natural defeat; preparation success has not become mission completion.

| Case | Primary route | Required outcome |
| --- | --- | --- |
| progression | Fresh Apprentice → Forest of Pain → Camulos/portal → original report OK → Realm → Plains of Greenhenge | Victory discriminator 1, actual region 2 admission and fresh presented World |
| victory | Separate fresh Apprentice run through the same legitimate mission-end route | Original victory report, verbatim title/21 strings, original OK and fresh Realm; later-map entry also supplies bounded cleanup |
| defeat | Fresh Apprentice → ordinary movement into hostile melee → wait for natural report | Defeat discriminator 0, original OK and fresh Main/Realm; no abandon before the primary report |

Each acquisition selects its frame mode through public Preferences, then runs
one ordinary-input helper (540 seconds maximum) and a 780-second overall driver
deadline, plus 40 seconds of collector cleanup allowance. Failed helpers, unavailable
regions, wrong outcomes, Quit-labelled reports, renderer errors, reloads and
same-map re-entry fail. Ordinary-input navigation/combat/dialogue heuristics remain
unvalidated candidates. The driver never writes health, mana, flags, script state,
campaign rewards or original simulation memory. A stalled conversation dismissal
or projected ground order is an attempt, not an input-consumption assertion.

The [current prepared inputs](rendering-progression-results-20261101/inputs-10.json)
pin the No-CD executable, Celtic realm/region configurations, both mission scripts,
preferences, and the ordered Specific/Random recipes. They hash all 124 candidate
MAP/NOD/EVT/MPS files: 13 Forest and 18 Plains sections. These are the finite
candidate asset closure; execution still must identify actual ordered placements,
rotations, random choices and the assembled grid. The private input copy uses the
installed assets unchanged. The [first refusal](rendering-progression-results-20261101/inputs-01.json)
preserves the unsupported installed executable finding. The
[restoration receipt](rendering-progression-results-20261101/inputs-02.json)
preserves its exact preimage and the canonical original replacement in this task's
private installation. No main-checkout installation or original file was changed.
Preparation also materializes and hashes the private installation manifest
`working/manifests/game-nocd.tsv` required by the public launcher's real preflight.
An existing mismatched/malformed manifest is refused. The preparation commit's
[inputs 04](rendering-progression-results-20261101/inputs-04.json) remain historical.

The unchanged 355,405-byte `native-menu-502b06ab.sav` is retained in SQLite with
SHA-256 `d33f46e282c3c1d7868c04d5746fc2bfbb1f64c8a859eb1428d74efdb547dda5`.
Its role is a separate reload/control candidate. Its map/progression position and
load compatibility are unverified; it supplies no near-win shortcut. Primary cases
all start fresh, so an old save cannot stand in for the missing progression event.

Run from the owned checkout
`working/worktrees/campaign-progression-prep-20261101` on branch
`codex/campaign-progression-prep-20261101`. Physical private game/music inputs are
prepared there. The RAM build under `/tmp/mnm-campaign-progression-build-20261101`
is disposable and may be rebuilt. If no stopped admitted prefix template is
available, the existing public launcher bootstraps a new owned prefix; use
`--prefix-template <stopped-admitted-prefix>` to reuse an explicit template.
The storage guard and existing prefix/output isolation rules remain in force.

```sh
python3 /home/hoff/swdev/magic_and_mayhem_mod/tools/working-retention.py \
  --root "$PWD" run --reserve-gib 5 -- \
  python3 tools/run-rendering-progression.py --case progression --size 800x600 \
  --preparation research/runtime/rendering-progression-results-20261101/inputs-10.json \
  --report research/runtime/rendering-progression-results-20261101/progression-default-high-NEW.json
```

Use new report names for the separate `victory` and `defeat` runs. Repeat the
three cases with `--size 640x480`, and with `--software` for each required software
configuration; those combinations are pending executions, not demonstrated support.
`--skip-intros` is available for diagnostics and must remain labelled in the
configuration. Diagnostic acquisitions/comparisons have distinct kinds and remain
ineligible for nominal acceptance. Enabled intros are the default acceptance candidate.
The main-checkout storage guard changes cwd to its selected root; retain the
explicit `--root "$PWD"` when invoking it from this owned checkout. Every report
name must be new, including retries; the recorded failures are immutable.

```sh
python3 /home/hoff/swdev/magic_and_mayhem_mod/tools/working-retention.py \
  --root "$PWD" run --reserve-gib 3 -- \
  python3 tools/check-rendering-progression.py \
  --acquisition research/runtime/rendering-progression-results-20261101/progression-default-high-NEW.json \
  --report research/runtime/rendering-progression-results-20261101/progression-default-high-compare-NEW.json
```

The comparator requires the existing `mnm-acceptance-world-history` and
`working/original-reference` oracle binaries with their compiler dependency files,
as used by `check-rendering-acceptance-pilot.py`. Both are built in this checkout; the
[oracle build receipt](rendering-progression-results-20261101/oracle-build-01.json)
pins their compiler inputs and binary hashes without executing original code.
Rebuild them after RAM build retirement:

```sh
cmake --build working/build/qt-shell --target mnm-acceptance-world-history --parallel 4
g++ -m32 -std=c++17 -O2 -fno-pie -no-pie -Wall -Wextra -Werror \
  -MMD -MF working/original-reference.d tests/acceptance-world-reference.cpp \
  -o working/original-reference
```

The comparison source and player closures,
all native swaps, contiguous command/World history, owned operation inputs,
RGB565 format/actual extent, original callbacks and zero final resource counts
must match. Captures start before the first World; no destination snapshots seed
the independent oracle. Diagnostic captures never silently become accepted cases.
The collector retains failures and raw artifacts in SQLite, with pinned execution
directories retained until reviewed handoff. It does not update artifact5 rows.

Populated original/native object/vptr/generation handoff acquire/release proof,
precise first-complete-frame boundaries per actual map, original reward/progression
semantics, as-run input consumption and actual assembled-map identity remain
execution obligations. Final zero owners and a successful pixel comparison cannot
alone close that lifetime gap. The comparator records this limit explicitly and
keeps supported-player/original-recovery decisions pending. The retained EP04
watchdog failure is unaffected by this preparation.

The [current tooling review](rendering-progression-results-20261101/guards-07.json)
records 9 mutation/projection/manifest/combat-policy/helper-termination checks
(including 13 semantic rejection mutations), 10 existing pilot tests,
13 existing normal-acceptance tests, five actual Qt public-control cases and
Release Qt player/fixture links with warnings treated as errors. These are preparation/tooling
checks, not original-game comparison or mission completion evidence. Generated
inputs, failed preparation, logs and review are SQLite references; raw save and
executable preimage bytes are retained in the same verified evidence stores.
