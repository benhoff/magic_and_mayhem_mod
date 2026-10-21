# Native creature drawing lifecycle

This implements the three previously missing drawing contracts in the pinned
No-CD build (`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`):
creature preparation, projected argument history, and scene queue retirement.
An opt-in PE32 adapter connects these native operations to the existing original
scene producer. It replaces four bodies/spans without calling original helper
instructions. Scene generation, camera changes, sorting, animation and raster
services keep their existing ownership; this is not a new creature simulation
or gameplay balance policy.

| Behavior | Original | Native operation |
| --- | --- | --- |
| `RS.creature-draw-preparation` | `0051e260..0051e307` | Preserve action/subtype/type/definition/target gates and wrapping target counter increment. |
| `RS.creature-draw-preparation` | `0052ab80..0052abb6` | Clear temporary counters only for active slots, retaining entry backing and rereading high-water count. |
| `RS.creature-draw-position-history` | `0051fa60..0051faa0`, overlap helper `0059cb60` | Shift the preceding two 12-byte records, store two raw caller arguments, then read/store descriptor DWORD. |
| `RS.scene-queue-retirement` | `004fd5ad..004fd5c1` | Zero queue count and reset cursor to base, preserving capacity and physical rows. |

The recovered core is [creature_drawing.h](../../reconstruction/rendering/creature_drawing.h).
Its memory interface uses explicit PE32 address values and bounded byte access.
It has no Qt, rendering backend, application widget or host object layout
dependency. The runtime adapter checks borrowed spans with `VirtualQuery`.

## Confirmed decisions and ordering

Preparation is enabled for actions 3, 9 and 10, or action 11 with subtype 1/5.
Otherwise byte `+723` must be nonzero. Type 18 with nonzero `+108` suppresses
that alternate path; other admitted cases test definition `+34` as an unsigned
nonzero DWORD. Creature `+68` must be nonzero. Target `+64c == ffffffff` skips
the update; other targets index the pool at stride `e4b`. Target `+e4`, rather
than its pool-active field `+4`, gates incrementing `+6e4`. Increment wraps at
32 bits. Confidence: high within the bounded, owned helper domain.

The history operation is an overlap-safe move of 24 bytes from `+754` to
`+760`, followed by stores to `+754/+758`. Only then does the original read
the pointer at `+b4` and its DWORD at `+4`, storing it at `+75c`. This is a
DWORD, correcting the earlier register's word description. A descriptor that
aliases a modified history record observes the new bytes. The old static
export ended at `51fa9e`, inside `RET 8`; the complete body ends at `51faa0`.
The exact coordinate meaning and downstream consumers beyond these stores are
outside this helper contract; raw caller arguments are preserved.

The existing producer selects creatures and clears `+74` before preparation,
then generates/consumes the queue, retires it and clears active `+6e4` counters.
Native hooks retain those call sites and ordering. The cleanup loop's high-water
count is at envelope `+228`; its entry base is at `+0`. This differs from the
target-index capacity at pool `+4`. It does not clear inactive slots. The queue
adapter preserves the span's EBX/EDX and meaningful XOR flag postconditions.

## Runtime connection

`tools/build-creature-drawing.py --output <working-build-path>` produces
`MnmCreatureDrawing.dll` and its source/hash manifest without consuming original
artifacts. `tools/prepare-scene-observer.py --native-creature-drawing` stages
the DLL import into a disposable game copy. The default staging path is unchanged.
`tools/capture-scene-game.py --native-creature-drawing` passes the option through
the bounded capture/launch workflow, fingerprints the compiled inputs and labels
the native helper substitution explicitly. Original-active driver observation
refuses this substitution.
The original source executable must match its pinned complete hash; import
staging preserves every original section byte. No files under `original/` change.

DLL import initialization verifies the complete 305 bytes of all four original
bodies/spans before changing any instruction. It prepares all four page
protections before publishing eight-byte entry jumps, restores protections and
flushes instruction caches. Byte or protection preflight refusal changes no
instruction. The original scene-production repetition hook uses separate call
sites/markers; these helper replacements do not widen its extra-draw policy.

`NP.native-creature-drawing-adapter` separately owns terminal refusal for invalid
or inaccessible references: the original out-of-range target path dereferences
NULL+e4, whereas the native adapter terminates explicitly, without forwarding.
History refusal after an unreadable descriptor can follow already completed
history stores, matching the original operation order; transactional rollback
is not promised. No shared cached pointers survive a call or launch. Concurrent
reclamation and exception unwinding have no invented recovered semantics.

## Execution and remaining boundaries

Run the reproducible comparison under the storage guard:

```sh
python3 tools/working-retention.py run --reserve-gib 0.5 -- \
  python3 tools/test-creature-drawing.py \
  --output working/tests/creature-drawing-<new-run>
```

The comparison executes unchanged original helpers in a private fixed-base
i386 image, native owned operations, and the production entry stubs/dispatcher
against the same independent inputs. Six complete 20-KiB owner snapshots per
sequence cover preparation, each of three history frames, queue retirement and
post-draw cleanup. All four packed creature slots, inactive/target flag
differences, action gates, unsigned definition flags, counter wrap, descriptor
aliasing and untouched bytes participate. Separate alias cases distinguish
retained backing from a reloaded count. All 305 individual guard mutations and
four page-protection failures must refuse before instruction changes.

After successful installation, original helper tails and the original overlap
copy entry are poisoned in the private fixture. Calls through original entry
addresses must still match every snapshot and preserve live x87 state and
LastError. The production PE32 DLL builds and its import staging is checked.
The Linux fixture mocks Windows memory-query/protection/error APIs explicitly;
it is not a Windows provider or actual-game execution claim. The isolated queue
span returns through a one-byte external-boundary RET at `4fd5c1`; the twenty
compared queue-cleanup instruction bytes remain unchanged for the oracle.

The immutable result is recorded at
[native-creature-drawing-results-20261020.json](native-creature-drawing-results-20261020.json).
Raw compiler/execution logs, binaries, prospective claims and the staged
executable remain in the named `working/tests/` run. Build/temporary compiler
files are execution outputs; the registered report, source guards, research and
source/review snapshots are durable evidence. Original manifest verification
bookends all original-consuming runs, including failures. Full original scene
generation, unbounded writers/lifetimes, actual-game hook admission and live
replacement milestones remain independent; no live stage is promoted here.

The first source-bound attempt is retained at
`working/tests/creature-drawing-sourcebound-20261020/report.json`. It exposes a
fixture-only stack-query upper-bound overflow near the top of the i386 address
space; the unsigned-distance fixture correction preserves that negative result.
The runtime adapter uses real `VirtualQuery` region extents and does not use the
fixture's simulated stack-range heuristic.
The second attempt at `working/tests/creature-drawing-sourcebound-final-20261020/`
passes the complete lifecycle and production DLL build, then refuses a nine-byte
PE import section name. The corrected `.mncreat` name respects the existing
eight-byte section limit; that negative staging result is retained unchanged.

## Commit-source repetition

The isolated creature-only commit source tree repeats all controls successfully:
[commit-source result](native-creature-drawing-commit-results-20261020.json),
`CREATURE-DRAW.lifecycle-commit-original-20261020`. This excludes unrelated
main-checkout driver-observation edits from the shared capture script. Its
source-bound proof is distinct from the first main-checkout success, whose
original report and fingerprints remain unchanged. Raw outputs are retained
in the registered `working/worktrees/creature-drawing-commit-20261020/` worktree
under `working/tests/creature-drawing-commit-sourcebound-20261020/`.
