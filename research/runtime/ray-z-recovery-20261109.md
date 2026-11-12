# Original ground picking and Z recovery — 2026-11-09

The selected original path uses sprite opacity membership during terrain
traversal. It does not intersect a geometric terrain ray. The recovered native
model now reproduces that bounded path, including the raw terrain Z and final
hit-class precedence. Confidence is high within the registered valid-input
scope; runtime binding remains outstanding.

The build is NoCD `Chaos.exe`, SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
All addresses below are for this build. Existing discovered function boundaries
already contain these anchors; this change does not widen the binary inventory.

## Recovered contracts

| Contract | Original evidence | Native behavior |
| --- | --- | --- |
| Sprite membership | `57dcf0..57dd58`, fastcall ECX frame, EDX anchor X, stack anchor Y/point, RET8 | Signed half-open bounds after 32-bit wrapped anchor-minus-origin and right/bottom arithmetic; alternating transparent/opaque RLE run membership, including zero-length runs. Opaque color/index zero remains a hit. |
| Terrain probes | `4f8ac9..4f8b6b` inside `4f8960..4fbcb1` | A nonzero declared body admits probing body/first/second masks. Probes precede child draw suppression by flags4/8000, so a suppressed child can supply the only hit. Above-bound frame indices fall back to frame0. |
| Ground coordinates | `4f8c3e..4f8c5e` | Stores scene annotations column+7d, row+81, layer+85 to globals `6c5488`, `6c5484`, `6cbb64`. Every later admitted hit overwrites XYZ. |
| Terrain depth | `4f8d30..4f8d49` | Updates scene+1ed and selected row `6c3750` only for a strictly greater depth. This is independent of the last XYZ publication. |
| Four views | `4f83f0`, `4fbe00`, `4fc3f0`, `4fc930` | Composes the existing recovered traversal, including camera bounds, concealment/cut admission, flags and carries. View0's inclusive cut differs from the rotated paths. |
| Initial state | `4fcfa2..4fcfb4`, `4fcff7..4fd018`; outer `4fd315..4fd33c` | Ground XYZ reset to -1; source mouse point copied into scene; actor/artifact/terrain depths seeded to `-64*mapWidth`. Fixtures supply these documented preconditions. |
| Hit classification | `4fd160..4fd1c0` inside discovered `4fcf50` | Actor must strictly exceed both artifact and terrain depths and exist; artifact is analogous. Otherwise ground wins iff ground X is not -1; otherwise none. Actor/artifact ties fall back to ground even when both exceed terrain. |

Z is the admitted traversal annotation, not necessarily the physical cell's
storage layer. The controlled view2 carry reads physical layer1 while publishing
column==width and logical layer0. Do not reconstruct this value from a storage
index or from actor Z minus a constant. Original observations include **504**
worlds where last published XYZ differs from the hit supplying maximum terrain
depth. The semantic traces compare both outputs independently.

`reconstruction/rendering/ground_picking.hpp/.cpp` owns masks, cells and results;
it composes existing traversal/submission code without game pointers, Qt or
runtime hooks. The separate CMake target is under
`reconstruction/rendering/ground-picking/`. Mask limits 2048x2048, grid
limits 128x128x32, validated binary opacity and safe collection bounds are native
policies. Invalid inputs are refused before publishing results. This does not
recover the original's malformed RLE, unchecked pointer or malformed one-past
collection behavior. Gameplay object services are outside this model.

## Independent execution

`RZ09.original02` / `RZ09.native02` cite
[comparison02](ray-z-recovery-results-20261109/comparison-02.json), payload SHA-256
`859bb684d0e345e672991465b982bb501a163bfed5b8c4dbb17cdecbcaddff69`.
One isolated 32-bit process executes unchanged original mask, terrain cell and
four traversal functions from a privately mapped hash-checked PE with code RX.
For classification only, it executes the unchanged 96-byte block in a private
RX allocation with a test return/register envelope; this is not a whole
scene-builder or Region callback execution. The independent native process
makes the entire original PE mapping inaccessible with PROT_NONE.

The byte-identical 5,328,050-byte semantic traces include all point decisions,
XYZ/sentinels, depth, selected row, ordinary submission counts, changed tile
flags and classification results. Trace SHA-256:
`aaaab4c540593d81972232bb1aebb4d92dc2a9810548ade94759404c053670d9`.

- **4,067,638 point cases**, including every pixel and one-pixel border of all
  1,753 installed Forest terrain SPR frames, transparent/zero-color synthetic
  masks, zero-length/long runs and signed-coordinate extremes.
- **23,173 terrain scenarios**: deterministic camera/cut/player/fog cases and
  all 1,753 installed TTD definitions across four views and three source roles.
  There are 9,741 hits, 13,432 misses, one explicit physical/logical layer carry,
  and 2,072 hits with suppressed draw roles. Eight unsafe traversal cases are
  recorded native-domain refusals, excluded from successful original comparisons.
- **1,000 classification cases**: 125 signed depth tuples and eight candidate/
  ground-presence combinations, including ties and signed extremes.
- ASan/UBSan native controls pass, including nine owned-input refusals, atomic
  input preservation and an exclusively suppressed-child hit. A standalone
  Release CMake build and CTest pass 1/1.

`RZ09.upper04-original` / `RZ09.upper04-native` cite
[upper04](ray-z-recovery-results-20261109/upper-04.json), payload SHA-256
`1ce6916be47bd70feaf7ebce655c5881764bc33a4f89069c7f639b19e155f3e4`.
Four additional unchanged-original cases hit **Z31** at logical 64,64 in an owned
128x128x32 grid, in every view. The opaque source pixel has color zero; suppressed
child flags remain unchanged. Height 496 stays below the queue's 500 branch.
Their 120-byte traces match exactly, SHA-256
`f3f86e291ce255301a19ffaba28dc835020afba6699e0945d7d22609c6f02353`.
Only these four missing original worlds were acquired. The previous original
matrix was retained; all its sources/inputs and the exact tested helper binary
were rechecked, and fresh native output matched the retained original trace.
The combined total is **23,177 terrain scenarios**, with lower hits at Z0–11
and the four focused upper-boundary hits at Z31; arbitrary combinations at every
intermediate layer are not exhaustively enumerated.

Both successful producers bookend original-artifact consumption with the
manifest verifier and retain exact binaries, source preimages, compiler inputs,
commands, raw semantic observations and useful diagnostics through SQLite.
No original file is patched; no game session is launched.

## Retained live observations and remaining integration

[Retained observation analysis](ray-z-recovery-results-20261109/retained-observations-01.json),
SHA-256 `083ee31090475c03c3d8d638f141126b056c3a6c0473267e11edcd8c50cb60cd`,
independently decodes all 288 prior picking callback/World-return witnesses:
144 cell/144 object, joined by frame, returned World, thread and cursor. All have
actual hit class 2. Actor Z minus raw ground Z is 0/1 in default and 1/2 in software;
a universal actor.z-1 rule is unsupported. These retained captures contain
raster/minimap arguments, not the complete ordered cell inputs needed to run
this new ground model. This analysis supplies observations, **not** a native
world prediction or a new live comparison. Earlier runtime/compiler/controller
source phases retain their original hashes.

The existing runtime world-cell provider still delegates hit membership to the
original. `apps/world-scene/picking.cpp` remains the separate native presented-
pixel policy. Installed resource lifetimes, actor/artifact suppliers, full
Region callback4fdad0, same-World continuity and live replacement are still
outside these four recovered contracts. Existing finite rendering acceptance
rows and both final decisions are unchanged by this recovery.

## Reproduction and retained failures

Run `tools/test-ground-picking.py --help` for explicit PE/SPR/TTD/output/report
arguments. Run `tools/test-ground-z-boundary.py --help` for the focused boundary
producer, which additionally requires the retained comparison report. Outputs
must be new, bounded paths; generated reports use `coverage_evidence.write_report`.
The shared four-attempt budget in `working/ray-z/attempts.sqlite3` is exhausted:
two successful isolated original processes and two zero-execution refusals.
A further acquisition requires a newly reviewed batch; this closeout does not
reset the counter or any older game budget.

The standalone build is `cmake -S reconstruction/rendering/ground-picking -B
<temporary-build>`, then `cmake --build <temporary-build> -j2` and `ctest
--test-dir <temporary-build> --output-on-failure`. Use the machine storage guard
and task-specific TMPDIR as required by AGENTS.local.md.

[Operational controls](ray-z-recovery-results-20261109/operational-controls-01.json)
retain the initial pedantic/GNU-thiscall compilation refusal, stopped report-
writer wait before execution, storage-preflight refusal and the upper03
Reader-method typo before execution. The corrected upper04 uses the supported
reader API. None is discarded or relabeled as a successful original comparison.
The evidence is stored as SQLite payloads; tracked generated files are references.
