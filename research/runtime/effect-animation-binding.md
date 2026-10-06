# Owned effect animation binding

Build: no-CD `Chaos.exe`, SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.

## Static caller resolution and native adapter

After [animation selection](effect-animation-selection.md), static caller
`00494be3..00494c26` resolves the returned ordinal into a 16-byte effect metadata
entry at collection `+438`. Each entry contains four DWORDs:

| Entry offset | Observed use |
| --- | --- |
| +0 | Numeric ANI sequence passed to original start |
| +4 | ANI asset ordinal, collection `+8` table with stride `114` |
| +8 | Copied unchanged to effect `+21e` |
| +c | Not read by this caller window |

The controller is effect `+160`. The caller binds the selected ANI object through
whole `00464ca0` at `00494c1a`, then starts the numeric sequence with reverse flag
zero through `00464cb0` at `00494c26`. The asset-offset arithmetic resolves to
ordinal times276 (`114` hex). No facing is added in this window. This mapping is
static disassembly evidence; TL22 does not execute the inline resolution window
or the surrounding type setup dispatcher.

`bindEffectAnimation` accepts the selected ordinal, an owned metadata table and
owned normalized ANI objects. It resolves the entry and sequence extents, copies
the selected records into the existing `NoCdAnimationPlayer`, and starts forward
playback. The returned binding retains the selected ordinal and all four entry
words, preserving the property and unused opaque word without assigning renderer
or gameplay meanings. It owns its sequence independently of both input tables.
The existing [forward controller model](animation-forward-contract.md) supplies
SPR selection and an owned displayed-record snapshot, including local metadata.

The input tables remain unchanged. A failed binding produces no result and does
not change an existing binding. Metadata/asset/sequence bounds, nonempty selected
extents, at most 65,536 records, nonnegative sprite ordinals and terminal stop are
native ownership/safety policies. Initial control escape or dispatch-budget
failure propagates from the existing player; later tick failures retain that
player's documented discard/restart boundary. Reverse playback is not admitted.

## Original controller comparison

[TL22 report](effect-animation-binding.json) executes whole unmodified
`00464ca0`, `00464cb0` with reverse argument 0, and `00464ec0` in a private PE32
mapping. Initial control dispatch uses the real original child routine. No
callbacks, stubs or binary patches are substituted. ANI tables and controller
allocations are authored fixtures; original file/metadata loading is not executed.

Eight distinct asset objects use original `114` strides. Seven sequences per
asset exercise initial sprites, initial events, delay, repeat jumps, conditional
break loops, skipped unknown opcodes, immediate stop and signed events. Four
initial byte fills expose missing resets. Each of the 224 fixtures binds and
starts twice, with 64 ticks per start: 448 binds/starts, 28,672 ticks and 29,120
compared states. The second binding restarts the used original controller.

Binding alone changes only controller `+1c`; all other controller/guard bytes are
checked unchanged. Forward start/tick compares numeric sequence, bound asset,
next/display record positions normalized to owned indices, active, delay, elapsed,
repeat count, break flag, forward flag and raw events. All 44 displayed-record
bytes, including nine metadata words, match. Original forward calls preserve
unmodeled reverse scan counters `+0x28/+0x2c` (relative controller offsets) and external
guards; their preservation is checked against each initial fixture, but no native
reverse-counter semantics are invented. Asset catalog/header fixtures and every
ANI offset/record array remain unchanged.

Original PE32, native64 and ASan/UBSan output streams match. Strict normal and
sanitized units and standalone CTest pass. Units compose the selector with binding,
check restart and record-metadata ownership after destroying the input tables,
and test invalid metadata/asset/sequence/extents, negative sprite records, missing
terminal stop and initial self-loop refusal. Source/executable hashes remain
stable; all 2,927 immutable originals verify before and after.

```sh
python3 tools/test-effect-animation-binding.py
```

Confidence: high for the compared forward controller composition and bounded
native ownership; caller metadata resolution remains static evidence. Historical
controller, selector, motion and placement implementations/evidence are unchanged.
This provides owned animated SPR selection, not a complete original effect draw
or scene-production claim. Installed effect metadata/ANI/SPR relationships,
complete type dispatch, direction variants, child effects, actual property-word
consumers, full rendering/lighting integration, creation ownership/lifecycle and
live scheduling/replacement remain separate milestones.
