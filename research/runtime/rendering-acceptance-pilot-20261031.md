# Public native rendering acceptance pilot — 2026-10-31

EP03 has a bounded positive execution receipt. The public launcher reached the
Main/setup route, started the first offered Quick Battle, presented composed
World output, delivered Qt Right-key input to the original game, returned and
entered World a second time, then completed the original Quit route. Independent
reference reconstruction matches every captured native presentation. This closes
the pilot pipeline; supported-player acceptance and original recovery stay open.

## Reviewed inputs and actual results

The source base is `86a03890226269f2b86b044ecb512a06cefba16e`; each receipt binds
the actual modified compiler closure, native admission sources and binaries.
The unchanged original executable is `nocd-40209ca7`, SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Both original-manifest verifications passed around each original-consuming run.
The acquisition identifies all 4,848 installation files. Reference assets match
that installation and remained unchanged through comparison.

| Proof | Actual retained result |
| --- | --- |
| [Public acquisition](rendering-acceptance-pilot-results-20261031/default-acquisition-04.json) | Default flag, enabled intros, original terrain lighting, no native lighting capture; two World visits, ten original menu callbacks, resize/focus/repeated controls and normal Quit |
| [Independent comparison](rendering-acceptance-pilot-results-20261031/default-comparison-04.json) | 44 reconstructed Worlds, 73,735 original raster/primitive calls; 113 native swaps, including 58 World swaps; 105,071,100 full viewport pixels compared; zero mismatches |
| Actual paint accounting | 2,900 paints and 2,899 swaps, including metadata-only movie/blank paints; seven unpresented paints and five coalesced source bindings retained separately |
| Original input effect | Two owned Right down polls and two releases, bound to Qt channel sequences after actual World swaps; original callback return site `0x469b9b` in the pinned build |
| Lifetime exit | No forwarded legacy rendering; original Quit; stopped private Wine process set; zero surviving graphics devices, surfaces, DCs, clippers, palettes or pixel bytes |
| [Actual software probe](rendering-acceptance-pilot-results-20261031/software-probe-05.json) | Public `--software-rendering` flag, skipped intros, two World visits and normal Quit; no full pixel acceptance claim |
| [Controls](rendering-acceptance-pilot-results-20261031/controls-02.json) | Nine capture/history tests, three receipt/restore tests, native observer lifetime fixture, trace-enabled PE32 callback/LastError fixture |
| [Admission](rendering-acceptance-pilot-results-20261031/acceptance-01.json) | One immutable `EP03.pipeline` receipt, exact evidence hashes, current source admission, successful reopen and independent SQLite restore; zero full requirement rows closed |

The offered selection is row 0, ordinal `1`, “Celtic Forest One (small)”. The
ordinal is an original menu selector, not a `MAP*.DAT` filename or a fixed
procedural seed. The concrete captured inputs define the comparison instance.
The public **Integer sharp** viewport control was selected; internal resolution
and original generation were unchanged. The actual Xvfb provider is llvmpipe for
both runs. This supplies no hardware-versus-software differential evidence.

## Independence and presentation boundaries

`AcceptanceCapture` is opt-in diagnostic interchange. Commands and complete
native viewport RGBA are collected independently, then retained directly in
SQLite. The default public player uses the GUI render owner. Its mutable lease
is bound to the quiescent command state at each actual paint; it is explicitly
not reported as an immutable worker export. GPU sampling completion precedes
the full framebuffer read, and Qt's swap signal binds the completed paint.
Unpainted/coalesced output does not become a presented-frame claim. Movie and
blank paints retain metadata but have no native-plane comparison claim.

After the player exits, `acceptance-world-history.cpp` rebuilds initial canvas
creation and CPU producer history from owned inputs. The separate 32-bit
`acceptance-world-reference.cpp` executes the pinned original drawing routines
for each World. Only this oracle history receives original results. No original
destination capture or native player output seeds it. The NumPy interpreter
independently applies the complete native command history, imports oracle World
planes and compares the entire viewport, including black borders. PRESENT takes
a snapshot: subsequent surface mutations cannot silently update the reference
for a previously presented texture. This actual cohort contains no explicit
GPU-to-CPU reads; support in the collector is not execution proof for that path.

The trace at `0x469b9b` establishes consumption of the Qt-held Right bit at an
original import callback. It does not recover the complete input/camera function.
Synthetic trace-enabled tests separately verify callback result and LastError
preservation, including fallback. Native target observer tests cover construction
through the real factory and destruction of a superseded renderer.

## Retained failures and remaining limits

All earlier receipt IDs and hashes remain unchanged, including failed sandbox
display creation, a missing installation manifest, the original short deadline,
the missing GUI observer, incomplete collector admission, and the initial
standalone control-build configuration failure. The omitted failed preparation
session is preserved by [supplementary retention](rendering-acceptance-pilot-results-20261031/retention-01.json).

`default-comparison-02` retained 284,932 startup pixel mismatches while all 37
World swaps matched. The oracle then incorrectly used mutable surface contents
instead of PRESENT snapshots; the run also used noninteger Fit sharp scaling.
The final reference fixes PRESENT history and the final pilot uses Integer sharp.
This does not establish the cause of every historical startup mismatch or certify
Fit sharp/Smooth. Their current reproduction and rounding reference remain work.

This pilot does not certify movie/audio content, every CPU upload producer,
all operations/callers, campaign/save/load/results, fullscreen, restart/recovery,
other maps/seeds/durations, full software acceptance or physical scanout. Worker
capture plumbing is built but the public pilot does not execute that owner.
The existing seven player and eight original-recovery scope conditions remain
visible. All 156 broad requirement/configuration rows remain pending until
per-row adequacy and dependency/reuse/failure joins are implemented and reviewed.

## Repository workflow and next work

The frozen artifact 1–5 definitions and historical evidence are unchanged.
`rendering_pilot_ledger.py` adds a separate append-only SQLite execution ledger
bound to their exact dossier fingerprint. Append validates collector types,
evidence/source phases, complete comparison counts, actions, retirement and
oracle separation. Read/restore revalidate the same payload and identity rules;
restore cannot promote either decision or any full requirement row. Historical
receipt validity is separate from current source freshness. This is the narrow
EP03 admission layer, not the complete EP10 aggregation/migration implementation.

Continue with EP04's normal journeys and EP05's progression/results using this
pipeline. Reproduce and isolate required operation/caller gaps in EP06. Add full
software proof with justified reuse in EP08 and fault/restart proof in EP09.
EP10 must join exact dependencies, configuration, failures and reuse per
requirement before EP11 can issue the two independent verdicts. Do not keep
rerunning already passing unrelated subsystems; change shared history or
presentation code only with a fresh affected pilot. No immediate user decision
blocks these engineering tasks.

Reproduction requires the reviewed private installation, isolated admitted
prefix seed, compiler dependencies and sufficient bounded storage. Prepare
`working/game-nocd`, `working/manifests/game-nocd.tsv` and
`working/fixtures/prefix-seed` using the existing installation/launch tooling.
Prefer RAM-backed staging without weakening either durable-drive reserve.

```sh
cmake -S apps/qt-shell -B working/build/qt-shell \
  -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build working/build/qt-shell \
  --target mnm-qt-shell mnm-acceptance-world-history --parallel 4
g++ -m32 -std=c++17 -O2 -fno-pie -no-pie -MD \
  -MF working/original-reference.d tests/acceptance-world-reference.cpp \
  -o working/original-reference
python3 -B tools/run-rendering-acceptance-pilot.py --report <new-owned-acquisition>
python3 -B tools/check-rendering-acceptance-pilot.py \
  --acquisition <new-owned-acquisition> --report <new-owned-comparison>
python3 -B tools/probe-rendering-software-pilot.py --software --skip-intros \
  --report <new-owned-software-probe>
python3 -B tools/test-rendering-pilot-controls.py --report <new-owned-controls>
python3 -B tools/rendering-pilot-ledger.py --acquisition <new-owned-acquisition> \
  --comparison <new-owned-comparison> --software-probe <new-owned-software-probe> \
  --reason '<concrete scoped review>' --report <new-owned-admission>
```

Use unused paths beneath `research/runtime/rendering-acceptance-pilot-results-20261031/`;
the tools refuse overwrites. Choose the SQLite store before acquisition. Physical
capture files under ignored `working/` are bounded interchange; retain citable
bytes, failures and backups before retiring RAM execution output.

## Storage maintenance

Reviewed cleanup removed 651 inactive reproducible build files, reclaiming
1,350,668,288 allocated bytes. Twenty-one current consumers were held; the current
Qt build, newest two target products, unique inputs, evidence and pins survived.
Complete cold verification, dual catalogs, identity/activity rechecks and an
actual restore preceded removal. Two identical immutable cold release caches
then retained both paths and all bytes through a shared inode, releasing another
689,106,944 allocated bytes. This is allocation recovery, not a net free-space
claim after acquiring the new evidence.

The execution evidence database remains pinned on the main drive. Its exact
SQLite snapshot has a separately verified cold backup and explicit reference-base
dependency; [the backup receipt](rendering-acceptance-pilot-results-20261031/backup-01.json)
records the full SHA-256 identities and actual restore. Later bookkeeping uses a
small separate SQLite store. Both durable-drive reserves remain mandatory.
