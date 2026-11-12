# Colour comparisons and rendering frontier — 2026-10-25

This batch expands original colour comparisons and audits remaining rendering
branches and scenarios on committed master through `f966d6a2`. It repairs
constant fills in recorded native World replay. It changes no web UI layout or
gameplay rules and grants no whole-renderer or live replacement acceptance.

## Original comparisons and the native repair

The pinned No-CD `0x00547d80` rectangle routine and the production interior
splice at `0x00547e53` independently execute from the same owned inputs. The
original preparation/footer remain active in the CPU fixture. The
[accepted comparison](colour-coverage-results-20261025/comparison-04.json) and
[CPU result](colour-coverage-results-20261025/comparison-04-cpu.json) record:

| Comparison | Cases | Accepted boundary |
| --- | ---: | --- |
| Existing CPU matrix | 4352 paired calls | 3054 native admissions; 1298 forwarded or hidden controls; physical words and caller state agree |
| Extended CPU matrix | 384 | All four modes, RGB565/RGB555, constants/planes, clipping, sizes through 2048×2048; 304 change pixels and 80 exercise empty clipped results |
| Ordered native World cases | 192 | Complete RGB565 original/native GPU planes agree across 38,199,120 pixels |
| Native World refusals | 5 | Invalid colour, inverted rectangle, payload, foreign active canvas and expanded draw budget |

The two CPU matrices compare 90,011,152 physical WORDs including padding/guards,
unchanged sources, general registers, selected EFLAGS, LastError and full FX
state. Forwarded/hidden controls do not count as native drawing. Source addresses
are owned anew by each process; only the verified original image uses fixed
build-specific addresses.

The World fixture interleaves small colour blends before and after large
constant fills. The native process receives only authored source requests; a
separate process executes original rectangle calls to produce expected planes.
It runs no original World traversal or simulation. These are controlled original
comparisons, not observations that the game naturally reached modes 2/3.

`DrawingScene` now admits owned fill operation 5 on its active World canvas.
`CanvasWorld` intersects the rectangle with canvas bounds and emits ordered
copy-mode colour tiles no larger than 64×64. It retains the 12320 expanded draw
budget. The existing opcode and RGB565 format remain unchanged. This closes
the recorded replay gap for large uniform copies without admitting oversized
source planes. Other live transports retain their own admission boundaries.

Ordinary drawing-scene and drawing-session regressions pass. The colour splice
fixture also needed its existing optional `NativeDrawCpuStoreV1` lookup to
return null for the supplied unprotected CPU-ready plane. That is a fixture
provider repair; no original or production pixel arithmetic changed.

## Remaining rendering frontier

The [reviewed machine-readable frontier](colour-coverage-results-20261025/frontier-reviewed.json)
and [readable scenario index](coverage/summary-colour-frontier-reviewed-20261025.md)
cover 785 registered contracts: 306 recovered behaviors and 479 native policies.
They retain original build/address scopes, independent status dimensions,
current evidence per behavior/scenario and 38 unresolved rendering dispatch
tables. They compute no whole-engine completion percentage.

| Current registered proof | Current | Pending | No claim |
| --- | ---: | ---: | ---: |
| Implementation | 93 | 644 | 48 |
| Original comparison | 63 | 159 | 563 |
| Integration | 10 | 340 | 435 |
| Scoped replacement | 2 | 6 | 777 |

These are checklist counts, not disjoint functions, defects or engineering
tasks. Native policies need not claim original comparison. Many pending entries
share changed dependencies and retain valid historical evidence. The 15 entries
without implementation claims include static ownership/supplier discoveries and
release planning; each scope needs review before assigning implementation work.

The audit indexes 664 registered scenarios. In 655, at least one selected
rendering behavior lacks current integration proof at the declared boundary.
A proof for one behavior cannot close another behavior in the same scenario;
behaviors outside this rendering audit remain explicit. This is not a count of
655 failed game journeys.

The reviewed category crosswalk keeps twelve required surface categories and
nine drawing-route families. The new CPU result adds a bounded locked-writer
comparison; it does not classify every Lock caller or close the effects family.
The shipped journey suite has zero generic presented-session catalogs. Its
component, publication and three recorded EGL session subsets remain valuable
partial evidence; the fourteen-journey/twenty-one-operation acceptance inventory
is separate. This audit executes no new journey or native replacement.

## Next bounded work

1. Diagnose the retained all-phase after-Load timeout, then obtain a complete
   longer Save/Load result and refresh current-source recorded-session parity.
2. Acquire natural live colour modes 2/3 and large-fill observations. RGB555 CPU
   comparison passes; RGB555 recorded World ingestion remains unadmitted.
3. Use the scenario index to select complete original/native integration
   intervals for camera/creature effects, HUD/cursor interaction, menu/movie and
   transition/recovery behavior. Preserve each original function/branch scope.
4. Resolve remaining effect/raster dispatch and source/writer ownership before
   broader hardware presentation or live replacement claims.

The earlier RCI25 216/224-frame results remain immutable historical evidence.
This batch changes native replay sources and does not renew those complete
storage/presentation comparisons. The new evidence renews the specific CPU and
ordered World fixture contracts above.

## Reproduction, negative evidence and retention

`tools/test-colour-coverage.py --report <new research path>` produces the colour
comparison directly through SQLite. Run it with a bounded execution directory,
an owned software X11 display and the working-storage guard. Host `/tmp` and
shared RAM quota issues required a private temporary mount/display namespace on
this host; those setup failures and the partial 176-case run remain unaccepted.
All successful original-consuming runs verify the 2927-file original manifest
before and after execution.

`tools/audit-rendering-frontier.py --report <new path> --markdown
research/runtime/coverage/summary-<new-id>.md` produces a read-only SQLite audit.
Ten frontier guards and sixteen crosswalk guards pass, including source/scenario
binding, audit-error suppression and static-discovery distinctions. Earlier
report/tool variants remain preserved under their original hashes.

The [committed-history review](colour-coverage-results-20261025/history-review.json)
checks 103 file versions from `d77015d4` through `f966d6a2`, with no unresolved
exact receipts. [Source impact](colour-coverage-results-20261025/integration-source-impact.json)
shows the sprite merge changes none of the sources fingerprinted by the passing
comparison. Its new modules receive no validation credit from this batch.

The [raw catalog](colour-coverage-results-20261025/raw-catalog.json) preserves
failed and successful inputs, complete original/native outputs and executed
binaries, with full SQLite restoration checks. Exact earlier execution source
variants are retained separately. Standard `finish-work.py` reviews and verified
SQLite backups precede releasing temporary RAM. The normal 90 GiB main and
100 GiB archive reserves remain unchanged; the review workspace uses RAM and
durable backups use the secondary filesystem.
