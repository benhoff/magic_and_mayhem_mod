# Selected forward ANI controller contract

Reviewed 2026-10-04. `reconstruction/animation/no_cd.hpp` models the selected
forward start/tick behavior of the pinned No-CD build. It is kept separate from
native asset storage, the renderer and preview application policies. No live
game routine is replaced. The [ANI layout](../formats/ani-native-loading.md)
and [retained trace evidence](animation-forward-contract.json) describe the
input and comparison boundaries.

## Confirmed selected operations

Original routine windows:

| Routine | No-CD address |
| --- | --- |
| File/header/table/version loader | `0x004644d0..0x00464ab4` |
| Sequence count / first record | `0x00464b20` / `0x00464b30` |
| Controller constructor | `0x00464c50` |
| Start selected sequence | `0x00464cb0` |
| Preserve phase when switching sequence | `0x00464e20` |
| Advance one controller call | `0x00464ec0` |
| Initial control dispatch | `0x00465000` |

Observed 48-byte controller fields are sequence at +0, displayed-record pointer
at +4, active at +8, delay at +12, elapsed at +16, remaining repeat count at +20,
next-record pointer at +24, animation object at +28, break flag at +32, reverse
mode at +36 and reverse scan counters at +40/+44. These are build-specific
static/in-experiment facts; their addresses are not stable live pointers.

Forward start clears delay, elapsed, repeats, displayed/next pointers and break
flag, stores the explicit numeric sequence, marks it active and selects its
first record. If that record is a sprite, it sets the displayed record but leaves
the next-record cursor at the same record. The first tick therefore selects the
first sprite again. If the first record is a control, initial dispatch processes
controls until a sprite or stop; events do not stop this initial dispatch.

Each ordinary active tick compares elapsed and delay as unsigned integers.
If elapsed is less than delay, it increments elapsed and keeps the displayed
frame. Otherwise it dispatches records until a sprite, event or stop, then
resets elapsed to zero. A delay of N therefore inserts N calls that only retain
the previous frame before the next dispatch call; it does not encode milliseconds.
Inactive ticks return zero without changing the state.

| Opcode | Forward effect |
| --- | --- |
| 0 | Select the record's SPR index and finish dispatch |
| 1 | Set delay to the argument's unsigned 32-bit word |
| 2 | Set repeat count to the argument's unsigned word |
| 3 | If repeats >0, decrement and jump by the signed argument in records |
| 4 | Jump by the signed argument when break flag is zero; otherwise clear flag and continue |
| 5 | Return the raw signed event argument and finish an ordinary tick; retain the displayed sprite |
| 6 | Clear active and displayed record, return 1, then advance cursor |
| Other | Skip the record within the bounded native sequence |

Relative targets are `currentRecord + argument`. The original adjusts the cursor
by `(argument - 1) * 44` and then applies its common +44 increment. Event numbers
are preserved; gameplay consumers and resulting actions are not implemented.
Stop's return value 1 is a raw controller result, not a universal event category.
The native `requestBreak` setter is compared using an explicit fixture write to
the original controller's break field; original live setter/caller coverage is
not established.

The static `0x00464e20` method changes numeric sequence while preserving the
relative next/display record positions and other state. This path, reverse
mode and reverse scan counters are not implemented or executed by this model.
Selected callers combine configuration-derived sequence bases with direction
indices, including `&7` patterns at `0x004948c7`/`0x0049499b`; this supports a
bounded directional-table interpretation but does not identify compass ordering
or establish every action mapping. The preview selects sequence IDs explicitly.

World update contains eight controller calls at `0x0046b74f..0x0046b795` after
the creature pass. Other UI/entity consumers call the controller too. This is
call-order evidence, not a recovered global frame rate. No relationship to wall
time, display refresh, pause or the complete simulation clock is asserted.

## Native safety and validation

`NoCdAnimationPlayer` owns a copied sequence. It exposes state and a selected
SPR index, uses indices instead of engine pointers, and retains all input record
metadata. The model caps sequence length and each dispatch to 65,536 records,
rejects control transfers outside its selected sequence and rejects negative
selected sprite indices. These are native safety policies; unsafe original
inputs are not executed for diagnostic-equivalence claims. A safety exception
can leave intermediate controller state; discard or restart that player rather
than treating it as an accepted tick.

```bash
cmake -S reconstruction/animation -B working/build/animation-model
cmake --build working/build/animation-model --parallel 4
ctest --test-dir working/build/animation-model --output-on-failure
python3 tools/test-animation-contract.py
```

Static export: `working/decompiled/animation-support-wkrx24or/`, with original
manifest logs and disassembly/caller artifact hashes in
[inventory evidence](../formats/ani-inventory.json). Final native/original run:
`working/tests/animation/run-txr69khq/`; original manifests verify 2,927 inputs
before/after. Native source and compiled helper/inspector hashes are retained.

The runner privately maps the original PE32 image and calls its unmodified
forward start/tick routines. Original animation objects contain bounded fixture
header/table/record pointers; file loading, sprite loading, Win32 APIs and the
game process are not executed. Native model results never modify original
reference input/state. Each trace has a five-second process deadline.

All **347 traces / 22,555 states** match: **340 selected installed sequences**
across every version-5 ANI file and seven synthetic control fixtures. Every
trace compares the start state and 64 calls: raw return event, selected SPR index,
next/display record indices, active, delay, elapsed, repeats and break flag.
Fixtures cover initial sprite repetition, initial events, delays, repeat jumps,
conditional breaks, signed events, unknown opcode skip and empty sequences.
The native fixture additionally checks invalid extents, jump escape and dispatch
budget failure. ASan/UBSan pass with leak checking disabled.

Confidence is high for these selected forward states. It does not establish
all sequences/ticks, the older conversion paths, reverse/directional switching,
gameplay event handling, all record metadata or live scheduling equivalence.
