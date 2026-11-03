# Campaign checkpoints — 2026-11-03

`checkpoint-camulos-05.sav` is a ready original No-CD Celtic1 diagnostic fixture:
356,531 bytes, SHA-256
`9ec512d4c05a75c20e7cc7befec8398e112c2ca700917f34e93f5501099ee7c1`.
The wizard stands at CF52 Place of Power `(23,57,6)` with 184 HP and 50 mana.
One living owned Zombie and Camulos remain. The neutral bridge enemy is dead.
The original writer created these bytes; no save or simulation field was patched.

This fixture supports short diagnostics. Loading it does not satisfy fresh
campaign victory, Realm travel, region2 or the nominal twelve-case matrix.
Full saved simulation, object generations, other maps and hardware remain open.

## Recorded validation

[Creation](rendering-progression-results-20261101/checkpoint-seeded-software-low-05.json)
loads a preserved original candidate, saves a new named file, reloads it and
quits normally. [Fresh-process restoration](rendering-progression-results-20261101/checkpoint-fresh-restore-software-low-05.json)
loads the exact new hash in another original process. Both compare all five
active creatures' slot/type/owner/active/HP/position/mana at original storage
completion boundaries, with zero differences. Behavior/action/target are
reported separately and also match in these two runs. All125pool slots are
recorded; inactive payloads are deliberately zero.

| Independent comparison | Worlds | Leased paints | Pixels | Differences |
| --- | ---: | ---: | ---: | ---: |
| [Load/new Save/Load](rendering-progression-results-20261101/checkpoint-seeded-comparison-05.json) | 522 | 632 | 552,684,000 | 0 |
| [Fresh process](rendering-progression-results-20261101/checkpoint-fresh-restore-comparison-05.json) | 39 | 101 | 88,324,500 | 0 |

The independent original/history oracle starts from owned initial producer
history. Original destination captures and native player pixels are not oracle
inputs. The actual player remains native software640; captures close completely,
source/compiler/binary fingerprints remain stable, original manifests pass
before/after, Wine stops and final owners retire. These are diagnostic results,
with `player_accepted=false`, not nominal configuration acceptance.

[Closeout](campaign-checkpoint-results-20261103/closeout-01.json) retains the exact
phase joins and limitations. Current181campaign controls plus two importer
controls pass. The separate native cache-pressure fixture also passes.

## Driver and wire boundaries

`tools/run-rendering-progression.py --checkpoint capture` can use the ordinary
bounded route, stopping at exact CF52 resupply with at least60HP/36mana, or use
an explicit `--checkpoint-seed` retained report. A seed requires a normally
completed original Save/Load flow, source/binary closure, stopped Wine and exact
candidate bytes. Its prior acquisition status is preserved. The new run must
still perform and validate a new original Save/Load pair. Seeded capture records
Load/Save/Load boundaries; ordinary capture records Save/Load. Names start with
`checkpoint-` and a new capture cannot overwrite an existing named checkpoint.

`--checkpoint restore` requires an admitted capture reference and the exact save
hash in the current checkout's owned store. It chooses that original Main Load
row in a fresh process. Both modes require `--case victory --skip-intros`; their
scenario names and validators are explicitly diagnostic. The ordinary
progression assertions and their rejection of reload as progression are unchanged.

`MNMCPS01` version1 contains a64-byte boundary followed by one64-byte row per
pool slot, with a16-byte file header. The opt-in passive observer runs after
original storage Tick when object `+0x43` changes from0to1. Tick EAX is preserved;
the recorded original calls return28, which is not a boolean success value.
No new executable hook site is introduced. The existing guarded No-CD pool
reader validates addresses and active slot IDs at every boundary. No pointer
address is treated as a cross-launch identity.

Each boundary records Save13/Load12, completion1, thread, host tick and capacity;
active rows include slot/type/owner/active/HP/position/behavior/action/target/mana.
The stream is bounded to eight boundaries and256slots. Partial, reordered,
wrong-thread, exhausted or malformed records fail decoding. Selected-field
agreement does not establish full world, AI, RNG, script, projectile, resource,
native parser or serialization equivalence.

## Ready-to-run use

With the verified SQLite inputs available, import the exact save into this
checkout's ordinary owned-save store. Existing saves and conflicting names are
preserved; this is a byte copy, not a serializer:

```bash
python3 tools/stage-campaign-checkpoint.py \
  --acquisition research/runtime/rendering-progression-results-20261101/checkpoint-seeded-software-low-05.json \
  --restore-proof research/runtime/rendering-progression-results-20261101/checkpoint-fresh-restore-software-low-05.json \
  --report research/runtime/campaign-checkpoint-results-20261103/import-LOCAL.json
```

Prepare a new input report in a new checkout using its existing pristine No-CD
working installation and admitted fixture prefix. In the retained owned checkout
`working/worktrees/campaign-checkpoints-20261103`, preparation
`checkpoint-inputs-04.json`, its private game and its fixture prefix are already
available. Use a new output name for each bounded run:

```bash
python3 tools/run-rendering-progression.py \
  --case victory --checkpoint restore --checkpoint-name checkpoint-camulos-05 \
  --checkpoint-reference research/runtime/rendering-progression-results-20261101/checkpoint-seeded-software-low-05.json \
  --preparation research/runtime/rendering-progression-results-20261101/checkpoint-inputs-04.json \
  --report research/runtime/rendering-progression-results-20261101/checkpoint-restore-LOCAL.json \
  --size 640x480 --software --skip-intros --observe-input-admission
python3 tools/check-campaign-checkpoint.py \
  --acquisition research/runtime/rendering-progression-results-20261101/checkpoint-restore-LOCAL.json \
  --report research/runtime/rendering-progression-results-20261101/checkpoint-restore-comparison-LOCAL.json
```

The [import controls](campaign-checkpoint-results-20261103/import-controls-01.json)
exercise conflict/invalid-name/empty refusal, preservation and idempotence, plus
a real SQLite-to-store copy and rejection of an unadmitted candidate. The
[verified copy](campaign-checkpoint-results-20261103/fixture-import-02.json)
retains the exact original file hash. Original save path fields are relative
`realms\Celtic\Celtic.CFG` and `Celtic`; the opaque world contains no checkout
name. This dependency check does not interpret every opaque field as a path.

## Preserved failures and cache policy

Four earlier acquisitions remain failed:

1. [01](rendering-progression-results-20261101/checkpoint-capture-software-low-01.json)
   stops at Region Entry because diagnostic Preferences adds acknowledgements
   before the storage driver's fixed ack2. The correction checks the next
   acknowledgement relative to the actual difficulty request and original value.
2. [02](rendering-progression-results-20261101/checkpoint-capture-software-low-02.json)
   reaches resupply with182HP/36mana, then native rendering hits64surfaces /
   10,826,600pixels before Save. Its recorded input peaks at55declared producer
   canvases; declaration does not prove every failing-packet operation executed.
3. [03](rendering-progression-results-20261101/checkpoint-capture-software-low-03.json)
   completes original Save/Load and normal Quit, but the observer incorrectly
   waits for EAX1 and records no snapshots. Its unchanged candidate
   `fe4365e8f5015fec9f297e35896f7d70900274b1ce60f11c6e9572ea18a5a4df`
   is only the explicit input to the newly witnessed05Save, not an admitted old
   acquisition. The [comparison refuses it](rendering-progression-results-20261101/checkpoint-comparison-refusal-01.json).
4. [04](rendering-progression-results-20261101/checkpoint-capture-software-low-04.json)
   exhausts the original-hover refusal budget during opening. The retained last
   native frame has no dialogue. Qt publishes `(470,226)` while the last original
   mouse callback remains at `(470,230)`, mask64/flags10; no fresh hover is admitted.
   The cause, any motion threshold and creature selection remain unproven. The
   spell-bar receiver field identifies the caster, not selected-creature admission.

The cache repair keeps the64surface/16Mpixel cap. Before producer allocation,
the owner may evict idle scene sprite/colour images that can be rebuilt from
retained sources. Canvas identity, history, validity, decoded resources and
leases survive; incomplete scene frames refuse reclamation. The
[before fixture](campaign-checkpoint-results-20261103/pressure-before-02.json)
reproduces exhaustion; the [after fixture](campaign-checkpoint-results-20261103/pressure-after-02.json)
checks exact capacity, unchanged words, rebuild and zero remaining surfaces.
The first fixture draft lacked a required raster; another used a non-idempotent
half-blend as an identical-redraw oracle. Both failed reports are retained. The
final test uses opaque redraw and separately checks incomplete-frame refusal.
This is intentional native storage policy. No live eviction counter or matched
source/RNG causal A/B experiment is claimed.

## Native save-reader frontier and remaining work

[Inventory03](campaign-checkpoint-results-20261103/save-candidates-03.json)
retains22unchanged owned saves and accepts their envelopes, then rejects every
world at offset300 with `World cell count exceeds limit`. The
[verified05file](campaign-checkpoint-results-20261103/fixture-envelope-01.json)
shows the same rejection despite successful original fresh-process loading.
The limits and production grammar are unchanged. Descriptor DWORDs at296/300
are6,400/153,600; a mistaken count/stride field remains a hypothesis requiring
paired original serialization review. The initial inventory CMake failure and
all exact inputs/producer versions are retained in SQLite. Prior uncommitted
source variants were [retained against their original hashes](campaign-checkpoint-results-20261103/source-variants-01.json).

Next use this checkpoint for physical-input/selection/order admission and
resource/target diagnostics, then obtain a fresh nominal victory/Realm/region2
run before repeating the12case matrix. Actual route suspension, whole saved
simulation, populated object lifetime and native save parsing remain separate.
Historical evidence and the [approach follow-up](campaign-approach-20261103.md)
retain their original source hashes and claims.
