# Native foundation closeout review (2026-10-19)

This review covers the dashboard's owned-world foundation, scoped mixing/playback,
and three supported asset-reader milestones. Their native implementations were
already supplied. The old central entries recorded source review only and left
their implementation/integration dimensions partial or absent. This work adds
prospective execution accounting and an explicit supported-input boundary; it
does not change engine algorithms or gameplay balance.

## Reproduction and admission

```sh
python3 tools/working-retention.py run --reserve-gib 0.35 -- \
  python3 tools/test-engine-foundations.py
```

The runner creates private Debug builds, preserving assertions in existing C++
fixtures. It declares behavior/scenario identities and source hashes before
building, checks compiler dependencies against that declaration, requires every
named CTest with no failures/skips, and checks source stability afterward. Logs,
JUnit results, executable hashes and the declaration stay under the new run.
Fixtures generate their own inputs. No original artifacts, installed game files,
Wine, display server or audio device are consumed.

The standalone native profile assertion fixture covers query semantics, exact and
short capacities, malformed/duplicate/non-ASCII input, owned lifetime, repeated
short reads and propagated I/O errors. The older profile utility emitted query
records for comparison but supplied no standalone assertions. Its historical
Wine comparisons remain separate and unchanged.

## Milestone boundaries

| Dashboard milestone | Reviewed completion boundary | Work deliberately still open |
| --- | --- | --- |
| Owned world state & checkpoints | Native bounded generation handles, cleanup/release, transactional lifecycle and native-v1 wire/publication/idle continuation | Original scheduler/lifetime and legacy saves; the existing broader `NP.checkpoints` v1-v8/preview evidence retains its own freshness |
| Scoped mixing & playback | Native PCM voices/mixer composed with bounded Qt push-output queue; synthetic partial-write/failure/lifecycle and consumed-clock controls | Physical speaker output, timing/fidelity, original DirectSound equivalence and complete manager semantics |
| Scoped file access & profiles | Native containment/handles, bounded ASCII profile policy and supported persistence envelopes | General Windows APIs, concurrent mutation, complete saved-world interpretation |
| Scoped map & region loading | Version-6 MAP grid, version-4 TTD raw records and native recipe parsing | Older MAP conversion, all terrain field semantics, procedural generation and entity creation |
| Scoped graphics asset loading | Supported version-2/4 SPR, version-3/4/5 ANI, 1/8-bpp CUR and 8-bit one-plane PCX readers | Other encodings, original allocation/error behavior, action selection, auxiliary-plane interpretation and complete drawing |

`AS.save_world` stays partial: parsing a save envelope or structural byte grammar
does not restore a complete playable world. `WORLD.sandbox` and the scene track's
preview lifecycle milestone are also unchanged. Comparison and live replacement
remain independent of these native-policy closeouts.

## Results

All controls pass in the source-stable private run
`working/tests/engine-foundations/run-qp98ww7o/`: two world CTests, six audio
CTests, fifteen asset CTests and the standalone native profile fixture. Assertions
remain enabled in both Debug builds. No engine algorithm repair was required.

The immutable [execution record](engine-foundations-native-controls-20261019.json)
registers `FOUNDATIONS.world-native-20261019`,
`FOUNDATIONS.audio-native-20261019` and `FOUNDATIONS.assets-native-20261019` for
fourteen native-policy contracts. Twelve old partial entries become scoped with
headless integration; the already scoped voice/mixer entries gain current
scope/scenario-bound execution. The first declaration preflight failed on an
incorrect fixture-header name before engine execution; its refusal record remains
under `working/tests/engine-foundations/run-o_x6k2ld/`.

All five dashboard milestone requirement lists are satisfied by the resulting
statuses. Four have current scoped execution support. The simulation milestone
also requires the broader existing `NP.checkpoints` contract: it remains scoped
with historical execution, and this run deliberately does not renew its entire
v1-v8/preview input domain. The UI must preserve that current-validation limitation.

Confidence is high within the reviewed native scopes and exercised synthetic
inputs. Original comparison, integration and live replacement remain independent;
no entire engine track or full gameplay restoration is declared complete.
