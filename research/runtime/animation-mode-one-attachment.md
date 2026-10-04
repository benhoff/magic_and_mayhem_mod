# Selected creature attachment mode 1

Pinned No-CD executable SHA-256:
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
This contract covers one numeric attachment mode, independently of gameplay
meaning, effect admission, world scheduling and live integration.

## Selection and transitions

`0x0051ff00` is thiscall `(creature, requestedMode, objectArgument)`, callee
pops eight bytes. The selected supported transitions are **0 to 1** and
**1 to 0**. Requesting the current mode returns zero before changing state.
Other transitions have object, flight, audio and gameplay dependencies and
are outside this implementation and original-execution fixtures.

Mode is creature DWORD `+0x7cd`. On entry to mode 1, the original reads entry
**36** at `global[0x006898d0] + 0x240`. Each effect configuration entry has a
16-byte stride. Its first DWORD is animation base; its second is the effect
ANI asset index. The asset table starts at `global[0x006894a0]`, with stride
`0x114`. It binds controller at creature `+0x7d5` using `0x00464ca0`, then
starts **base + creature ANI facing (+0x60c)** via `0x00464cb0`. The controller's
owned asset pointer corresponds to creature `+0x7f1`; display is `+0x7d9`.

The loader region `0x0049c653..0x0049cc11` reads `NumberofAnimations`, allocates
16-byte entries, builds `ANI_` section names and reads `AnimationNo` and
`AnimationFileRef`. References are compared with generated `EFFECTS` plus an
index. ANI objects use the `0x114` stride; paired assets are built as
`sprites\EFFECTS` plus index and `.spr` / `.ani`. The manager at `0x00689498`
holds ANI objects at `+8` and effect entries at `+0x438`, matching the globals
used by the transition caller.

Installed `CFG/Encrypted/effectani.cfg`, after checksum-checked decoding, has:

| Field in ANI_36 | Installed value |
| --- | --- |
| AnimationNo | 40 |
| AnimationFileRef | EFFECTS2 |
| SpritePrinter | NORMAL |
| Data1 | 0 |

Thus this installed recipe selects `Sprites/effects2.ani`, sequence **40 +
admission facing**. Facings 0..7 are valid selected sequences, but this contract does not infer named actions or automatic phase switches
for this attachment. Admission uses the explicit facing at entry. Other calls
may restart this controller from configuration, including `0x00513f50`; the
complete producer/caller admission graph remains unresolved.

Leaving mode 1 calls `0x00464c80` to clear displayed/next pointers, active,
delay, elapsed, repeats and break fields. It retains the sequence ID and
asset binding. The native player retains its owned sequence and clears its
index-based playback state. Reentry starts the selected sequence again.

## Drawing and updates

The selected draw region `0x004faa83..0x004fab3c` admits this layer only when
**current health (+0xe4) is nonzero** and the mode getter `0x0051fef0` returns
1. The test is nonzero, not positive. It selects the child SPR frame from the
controller display, then uses **creature base + first parent attachment point
+ child ANI sprite offset**. The parent's sprite displacement is not added.
The parent's footprint/view adjustment applies to its attachment point; the
child local displacement has no second footprint adjustment. Each SPR still
subtracts its own origin. See the [placement contract](animation-placement-attachments.md).

In `0x0050e670..0x0050e697`, the original ticks the body controller, then ticks
this child when mode is 1. This selected caller ignores the child's returned
event; other callers can consume events and remain outside this scope. Health
does not gate this selected tick call. No stopped-child automatic restart is
present here. Global cadence and which update caller is chosen are unresolved.

The original draw branch assumes a nonnull child display and substitutes SPR
index zero when the argument is above its header bound. The native preview
instead validates selected SPR indices and hides absent child/parent display
records. Those are explicit safety policies; unsafe null/out-of-range original
draws are not executed as diagnostic comparisons. Original sorting, lighting,
projection, clipping, null-parent fallback and gameplay effects remain separate.

## Model, validation and confidence

`reconstruction/animation/attachment.hpp` models selection and the selected
mode/health gates. It rejects facing outside 0..7 and base-plus-facing overflow;
these are native bounds rather than original malformed-input equivalence.
`NoCdAnimationPlayer::stop` implements the selected reset with owned records.
There are no Qt widgets or original pointers in these contracts.

Reproduce with `python3 tools/test-animation-attachment.py`. This verifies the
original manifest before/after, including failures; privately maps the unchanged
pinned PE32; supplies bounded configuration/ANI fixture objects; and calls the
complete original 0-to-1 and 1-to-0 transitions. No code bytes are patched and
no Win32/file/audio/gameplay routines are redirected or executed.

The [retained report](animation-mode-one-attachment.json) comes from
`working/tests/animation-attachment/run-4k8n623o`: **8 admission facings / 520
original controller states**, matching native sprites, next positions, active,
delay, elapsed, repeats, break and raw events. All **520 same-mode requests**
leave the original creature fixture unchanged. **8 removal/reentry pairs**
clear the original playback fields and restart the corresponding native frame.
Installed configuration is decoded independently by the Python reference;
original transition execution uses an explicitly constructed fixture table,
not the original Win32 configuration or file loader.

Confidence is high for selected transition arithmetic/reset and controller
states. Health/mode drawing admission and coordinate composition are static
caller findings supported by separate helper and native pixel evidence;
complete original drawing, gameplay identity and lifecycle are not claimed.
