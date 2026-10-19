# Native rendering implementation follow-up — 2026-10-18

This increment replaces the remaining clipped image-copy routine and extends
the owned 2D facade. It also adds opt-in durable graphics checkpoints and
quiescent context rebuilds. It does not assert complete game replacement.

## Whole image copy

The pinned No-CD40209ca7 entry533a00..533b1a is checked in full before an
eight-byte entry jump. Its clipping, byte-coordinate preparation, source and
destination row stepping, forward DWORD/residual overlap and copy loop now run
natively. The separate checked533aec row entry remains available to its scoped
ABI controls. No original instructions in the replaced function execute through
the entry. Caller orchestration and other original image methods remain outside
this replacement.

Disassembly confirms the non-WORD branch initially uses x as its source stride
when computing the clipped start, then uses image width for subsequent rows.
The implementation preserves this observed branch, rather than silently correcting
it. The native entry preserves LastError and the thiscall callee-saved registers;
it validates representable owned row spans before requesting CPU history.

The PE32 fixture executes the exact original283-byte routine at a separate
address against independent source/destination storage.528 combinations of format
branch, signed x/y and clip rectangles compare all4096 destination bytes. Original
entry/body bytes are poisoned after native installation. This is a bounded
function-body comparison; it does not establish all application image callers.

## Extended owned 2D operations

The CPU facade implements all256ROP3 truth tables, owned tiled pattern surfaces,
source/destination key ranges and overrides, nearest-neighbour mirroring and
quarter-turn rotation. Self source/pattern operands freeze the original backing.
GPU-ineligible results continue through owned CPU CREATE/UPDATE publication.
There is no diagnostic CHECK-to-producer path.

The facade creates/enumerates indexed8 and canonical16/24/32-bit primary modes.
Fresh non-WORD primary contents are obtained through the owned snapshot service;
the World producer itself still has its declared RGB565 contract. Palette objects
support2/4/16/256 entries, primary flags and reserved black/white endpoints for
256-entry palettes without ALLOW256. Smaller RGB palettes are zero-extended in
the versioned256-colour wire contract. Successful reserved-endpoint updates are
observed from actual entries, rather than assumed to equal the requested values.
Byte-entry hardware-index palettes are facade objects, not interpreted as RGB
by the bridge. Alpha/Z operations, arbitrary-angle rotation and overlays remain
outside this2D contract; none is a demonstrated required game caller.

The native-policy fixture checks everyROP table against an independent bit oracle,
16rotated/mirrored copies, key overrides, seven palette classes, four primary
depths and mode enumeration/filtering. Existing150five-format transformations and
protected owner-Lock/raw-instruction controls remain part of the regression.

## Graphics checkpoint recovery

`tools/run-native-owned.py --device-recovery ...` enables
`MNM_NATIVE_DEVICE_RECOVERY=1`. Completed render turns and asynchronous production
outputs retain native RAM checkpoints. These perform additional GPU readbacks,
separately counted as recoveryReadbacks. The ordinary rendering profile is
unchanged. Capture includes native pixels, partial validity, clip state, palette
knowledge, stable surface IDs and live mutable/immutable frame leases. Expired
frame tracking is pruned even when checkpoint mode is disabled.

On a presentation context failure, the GUI returns its frame lease, stops
sampling and requests a serialized owner rebuild. Both command and World owners,
including the asynchronous production thread, rebuild contexts from native RAM.
Command channels, World generations and the original game process are retained.
Frame consumers rebind against the rebuilt sharing group. Recovery rejects
changed state after the last completed checkpoint and is bounded to three host
attempts. A queue recovery barrier also works while both immutable output slots
are leased and a subsequent job is waiting.

Real OpenGL controls rebuild into a different sharing group and back, preserving
pixels, palettes, clip state, undefined regions and outstanding frame leases.
The production queue control retains both leased outputs through its rebuild,
then continues the next source update and ordinary owner teardown.

This supplies **quiescent graphics recovery**, not arbitrary physical-reset
recovery during a draw, a failed producer command, a hung driver or a lost Qt
platform surface. The isolated recovery follow-up now handles those renderer failure classes
without restarting the game; see [native isolated recovery](native-isolated-render-recovery.md).
The local quiescent API and its historical controls remain bounded to completed
checkpoints. Actual hardware resets and recovery overhead still need acceptance.

## Accounting

New policies are NP.native-extended-2d, NP.native-clipped-image-copy and
NP.native-device-checkpoints. Older execution reports and fingerprints remain
unchanged. New source changes leave their historical results historical; a build
pass or census refresh does not renew complete-session or hardware acceptance.
