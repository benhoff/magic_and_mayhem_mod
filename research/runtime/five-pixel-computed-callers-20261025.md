# Five pixel helper computed caller investigation

No natural original-game caller was recovered for any of the five helpers.
The bounded computed-address search adds evidence to their provisional
`unused_candidate` classification; it does not establish dead code. All five
implementations and the existing 43-route denominator remain. The historical
observed union of 38 routes is not a fresh execution result.

This batch starts at local master `d77015d4` in its own worktree. It extends the
[earlier direct-branch investigation](pixel-route-branch-callers.md) rather than
repeating its every-byte CALL/JMP and stored-address searches. No game dispatch,
configuration, executable bytes, inputs or counters are modified.

## Result for each helper

| Route | No-CD body | New decoded direct callers | New computed value candidates |
| --- | --- | ---: | ---: |
| PCX WORD | `54b5f0..54b7fb` | 0 | 0 |
| Brighten | `58d770..58d8b6` | 0 | 0 |
| Row copy | `58d3b0..58d4a7` | 0 | 0 |
| Raster splice15 | `57e1b0..57e540` | 0 | 0 |
| Minimap camera3 | `5532f0..5536bc` | 0 | 0 |

Ends are exclusive and retain the already registered recovered ranges. Static
decoder roots explicitly include these helpers, even though they have no known
natural caller. Discovering their bodies cannot establish runtime reachability.

The final [original-image record](five-pixel-callers-results-20261025/central-image01.json)
is `FIVE1.central-image-20261025`. Its 45,731 seeds include all 6,757 current
central function entries, retained flow sites and numeric targets, PE entry and
the five helper entries. Recursive decoding follows direct edges and call continuations. It
retains 515,966 instructions with zero decoding failures or overlapping starts.
This is a seeded discovery graph, not an entrypoint-only reachability proof.

The new local value analysis considers immediate/register copies, ADD/SUB,
XOR/OR/AND, shifts, INC/DEC/NEG/NOT, scaled LEA, initial file-backed DWORD reads,
indexed indirect operands and PUSH/POP/RET candidates. It clears local state at
calls, branches, known incoming targets, gaps, stores, partial-register writes
and unsupported instructions. The report preserves every unsupported mnemonic
and reset count. No inter-block or interprocedural value proof is attempted.

## Indirect frontier retained

The historical inventory has 3,717 computed-flow sites. Fresh bytes decode
3,716 as indirect transfers; the other at `5c2f2e` is the original `CC`/`INT3`,
also labeled `INT3` in the historical record. It is retained as a classification
discrepancy, not rewritten into a function call or removed from old evidence.

The scan finds 3,719 indirect CALL/JMP sites overall:

| Decoded operand category | Sites | Remaining boundary |
| --- | ---: | --- |
| Direct import-address-table slot | 858 | Loader-installed platform target |
| Other absolute memory | 140 | Runtime global contents and writers |
| Indexed/receiver memory | 1,674 | Object identity, table contents, index and lifetime |
| Register | 1,047 | Runtime value and upstream producer |

The 915 available initial-image values are candidates, not recovered runtime
targets. In particular an import thunk initially contains a name RVA or ordinal;
that number is not the platform function address installed by the loader.
Propagated reads retain their import provenance and are not counted as caller
recovery. No value in this bounded analysis points into any of the five ranges.

Three outgoing virtual calls are absent from the historical flow census:
`58d487` (`FF 91 80 00 00 00`), `58d494` (`FF 92 80 00 00 00`) in row copy, and
`58d8a8` (`FF 91 80 00 00 00`) in brighten. Each calls receiver-table offset
`+80`; actual callee identities remain unresolved here. These are outgoing
services inside already recovered helper ranges, not newly found callers into
the five helpers. An incomplete central dispatch entry preserves them without
changing historical inventory boundaries or hashes.

## Reviewed original selectors

Nine byte-checked stores in CPU initializer `484470` retain the actual branch
targets for its four dispatch globals, including the override to `57de00`.
Neither CPU branch nor that override selects `57e1b0`. This excludes splice15
from this particular chooser, not from every possible indirect route.

The four-entry minimap table at `555e08` and its three CALL bodies select
`5527a0,552f20,5527a0,552b50` (camera0/2/0/1). Camera3 remains unselected by this
particular dispatch. The other three helpers have no new decoded direct caller
or supported local computed-address candidate. Unknown providers do not become
safe or unreachable through absence of a candidate.

## Controls, provenance and reproduction

[Final controls](five-pixel-callers-results-20261025/central-controls01.json),
`FIVE1.central-controls-20261025`, pass 34 tests using the installed Capstone5
decoder on actual x86 bytes. Positive cases exercise split RVA/base, encoded
pointers, arithmetic/wrap, scaled LEA, memory-indirect and pushed return targets.
Refusals cover unknown registers/memory, branch joins, call clobbers, incoming
roots, gaps, partial writes, aliasing stores, segment bases, unsupported
instructions and exclusive helper ends. Graph controls retain unrooted bytes,
overlaps and decode failures and refuse instruction-budget exhaustion.
These are scanner controls, not original-game or pixel comparisons.

Both original-manifest bookends verify all 2,927 immutable inputs. The executable
is pinned to SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
All final source fingerprints are stable. Reports and earlier source variants
are SQLite-backed references, including the initial failed control assertion
caused by duplicate provenance sites. The correction deduplicates the trace;
that failure and its exact sources remain retained. Successful narrower
controls02/image01 and controls03/image02 retain their own fingerprints.

```sh
python3 /absolute/main/tools/working-retention.py run --reserve-gib 0.25 -- \
  python3 -B /absolute/task/tools/inspect-five-pixel-callers.py \
  --mode controls --report research/runtime/five-pixel-callers-NEW/controls.json
python3 /absolute/main/tools/working-retention.py run --reserve-gib 0.25 -- \
  python3 -B /absolute/task/tools/inspect-five-pixel-callers.py \
  --mode image --report research/runtime/five-pixel-callers-NEW/image.json
```

The runner declares the exact behavior contract and required sources before
execution. It refuses an unsupported build, instruction-budget exhaustion,
changed sources or an existing report. Failed attempts remain durable.

Confidence is high for the pinned decoded bytes, the two specific selector
exclusions and the stated local scan. Overall natural reachability remains
provisional. Runtime-computed/encoded pointers beyond the modeled local
operations, cross-block/interprocedural propagation, dynamic/external code,
foreign modules and other builds remain unresolved. No live game was launched;
no new integration, original comparison, replacement or five-route completion
milestone is claimed.

The [local audit input receipt](five-pixel-callers-results-20261025/local-audit-inputs01.json)
retains exact missing historical research text in the task SQLite store for
normal working/staged checks. Raw capture archives remain in their existing
stores. This is local evidence retention, not new engine validation or publication.

The additional [planning guard](five-pixel-callers-results-20261025/planning-baseline01.json)
runs38 tests with one inherited fixture failure:the expected unrated list omits
eight already present cursor/screen contracts. The base and current registers
produce the identical list;this diagnostic is outside that capability track.
No planning weights,statuses or fixture expectations are changed here.

Final review explicitly compares the older6,675-entry seed census with the
current central6,757-entry inventory. The final scan includes all82 additional
function entries and2,081 additional decoded instructions. Its81 additional
seed locations reflect splice15 already being an explicit helper root.
The3,719 indirect-site categories and zero candidate callers are unchanged.
Earlier image03/controls04 retain their narrower source/contract fingerprints.
The runner now selects the registered inventory and refuses a missing
prospective inventory dependency. The first local finish completed reviews and
verified backups but encountered a transient Git index lock before staging;
those receipts remain append-only and grant no current-code validation.
