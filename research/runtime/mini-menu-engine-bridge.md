# Mini Menu engine bridge: offline preparation

Prepared 2026-10-05 at the user's request to work offline without tests.
**Activation is disabled by default. No fixtures, CTests or game sessions were
run for this milestone.** Compilation is recorded separately below and does not
validate pause, resume, confirmation, input or live replacement.

## Static recovery

Pinned No-CD SHA-256:
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
`tools/export-mini-menu-support.py` exports disassembly, vtable words, MiniMenu
string references and Ghidra pseudocode in a read-only project session. The
final selected export is `working/decompiled/mini-menu-support-gy6guhe9/`;
its manifest hashes the artifacts. Before/after immutable checks passed for all
2927 originals. Earlier export `mini-menu-support-cmg154_1` includes adjacent
screen candidates; it is not the selected Mini Menu vtable evidence.

Confirmed for this build by disassembly and vtable/initializer references:

| Contract | Address/offset |
| --- | --- |
| Constructor, screen ID | `0x004b1d80`, ID 17 |
| Vtable / original tick slot | `0x005c6644` / `0x005c6654` |
| Shared tick / window procedure | `0x005595d0` / `0x00559390` |
| Entry / exit / suspend / resume | `0x004b1eb0`, `0x004b1ed0`, `0x004b1f30`, `0x004b1f20` |
| Layout / controls / draw / Escape helper | `0x004b1f40`, `0x004b2100`, `0x004b23a0`, `0x004b2700` |
| Button / confirmation callback | `0x004b23f0` / `0x004b24f0` |
| Common screen owner / depth / stack | `0x006f34e0`, `0x006f349c`, `0x006f34a0` |
| Initialized / fade / pending / replace / return | `+8`, byte `+0xc`, `+0x33`, `+0x37`, `+0x43` |
| Confirmation / callbacks / button count | `+0x3b`, `+0x4f`, `+0x57` |

`0x0068991c == 0` selects five campaign buttons, CFG TEXTBUTTON_1–5.
Nonzero selects three battle buttons, TEXTBUTTON_6–8. The layout method shows
this explicitly. The button callback adds two to the local index in battle
mode, then dispatches through its five-entry jump table:

| Battle intent | Local index | Original effect |
| --- | --- | --- |
| Preferences | 0 | Pending `0x006a4948`, return flag 1 |
| Quit Battle | 1 | Allocate an original confirmation dialog at `+0x3b`; retain current screen |
| Cancel | 2 | Return flag 1; no pending screen |

Both button and confirmation callbacks use ECX receiver, one stack argument
and `ret 4`. Control initialization registers a 12-byte callback object:
vtable `0x005c6674`, function at +4, owner at +8. Its second callback is the
confirmation receiver. The adapter invokes the button callback, never the
confirmation callback. No direct pending, return or quit-flag writes occur.

The confirmation callback sets return flag 1 and destroys/clears `+0x3b` for
both answers. Answer 0 additionally takes an exit branch: special object
`+0x53 == 4` uses a distinct game-quit path; context `0x00689920 == 5` invokes
network-related helpers and sets `0x006dbc18`; otherwise it sets
`0x006dbc19`. Nonzero answers only return. The interpretation of answer 0 as
Yes follows these side effects; actual labels and live result behavior need
validation. The prepared adapter rejects the two special contexts above and
keeps the entire confirmation UI in the original viewport.

Shared window procedure Escape calls the screen's slot +0x18, sets return
flag 1, and may start a fade when byte +0x11 allows it. The Mini Menu constructor
sets that byte to zero. Its helper redraws available controls. Native Escape
uses semantic Cancel through the original button callback; functional return
agreement is plausible from the static paths, but precise redraw/input/sound
ordering is not claimed equivalent.

The shared tick checks pending push, replacement and return before normal
menu work. Return exits the current screen, decrements stack depth, restores
the parent, and calls its resume slot. This is strong static evidence for
screen lifecycle. **It does not establish that simulation, mana, timers or
audio are paused.** No synthetic paused bit or native timer freeze is added.
Battle Escape ingress and the parent's suspend/resume side effects remain
unvalidated; opening stays with original engine input.

## Prepared implementation and gate

`protocols/include/mnm/menu_v4.h` preserves V3 lanes and payloads and adds
magic `MNMMCMD4`, size 69632. The engine seqlock covers eight little-endian
words at byte 42000:

| Word | Value |
| --- | --- |
| 0 | Battle-layout boolean |
| 1 | Original confirmation-present boolean |
| 2 | Stack depth, 1–15 |
| 3 | Parent screen ID; never a parent pointer |
| 4 | Offered actions: Cancel 1, Preferences 2, Quit 4; zero during confirmation |
| 5 | Original context value at `0x00689920` |
| 6–7 | Reserved zero |

Actions 13/14/15 mean Mini Cancel/Preferences/Quit. Original button indices are
confined to the runtime adapter. Generation changes on payload, ownership and
readiness changes; heartbeat, stale generation, engine thread, acknowledgement
and once-only guards remain in force. Readiness additionally requires no fade,
pending/replace/return request, original confirmation or unsupported context.
Snapshot checks the original callback receiver and current stack top.

`runtime/menu/mini.h` contains the guarded snapshot and original callback
dispatch. `observer.c` checks the expected tick slot and button prologue before
patching the one Mini Menu vtable slot. Original tick/drawing are forwarded.
The unvalidated hook only installs for V4 and a compile-time gate of 1. A normal
runtime receiving V4 retires the channel; it installs no Mini Menu hook and
cannot dispatch its commands. V1–V3 default selection is retained.

`LiveMiniMenuController` binds the existing battle `MiniMenuWidget` to semantic
session requests. The widget remains free of wire offsets and engine addresses.
The session observes fresh Mini Menu readiness while commands are suspended
for battle, switches to Qt, and hands Cancel, Preferences and Quit back to the
original viewport after acknowledgement. Original dialogs/results and parent
lifecycle remain authoritative. Explicit fallback remains permanent.

`MNM_EXPERIMENTAL_MINI_MENUS` is a CMake option, **OFF** by default. It defines
`MNM_MENU_MINI_EXPERIMENTAL=1` only in a dedicated shell build. That shell stages
with `prepare-menu-observer.py --actions --experimental-mini`, causing the
runtime builder to use the same gate. The preparation flag requires actions;
the launcher refuses V4 unless provenance records experimental Mini staging.
No ordinary launch flag enables this feature. Normal `--live-menus` creates
V3 and does not construct/load the live Mini Menu. Do not enable the dedicated
build for normal use until the validation steps below are completed.

## Evidence and confidence

The default Qt shell compiled with strict warnings, and separate enabled and
disabled PE32 observer DLLs compiled at
`working/build/menu-observer-mini-offline/` and
`working/build/menu-observer-mini-disabled-offline/`. These are compilation
artifacts, not test evidence. New behavior and regression behavior have **not
been tested**. Existing Main/Quick/setup/spell evidence predates these edits and
must not be counted as validation of this milestone.

Confidence is high for the selected static addresses, button mapping and
stack/confirmation side effects. V4 wire decoding, host/controller lifecycle,
callback ABI execution, pause/resume and live operation remain unvalidated.
Original menu drawing is retained. Campaign saves/load, multiplayer leave,
native Preferences/confirmation/result UI and longer play remain separate.

## Later validation procedure (not executed)

1. Build isolated PE32 cases with original button/confirmation bytes and stubbed
   unrelated services. Compare unhooked versus observed Cancel, Preferences and
   Quit, stack cleanup/nonvolatile registers/LastError and adjacent fields.
   Verify disabled gate leaves the Mini slot untouched and retires V4.
2. Check V4 malformed payload bounds/booleans/action masks, stale/duplicate/
   outstanding requests, original confirmation rejection, timeout/fallback,
   and campaign/network/special-exit refusal. Preserve V1–V3 regression checks.
3. In a dedicated opt-in build/prefix, start a disposable Quick Battle. Record
   actual owner, parent, depth, mode/context and engine thread before commands.
   Confirm original Escape ingress presents native Mini only after readiness.
4. Observe world updates and mana/timer progression with the menu open, during
   Preferences, and after Cancel. Establish original pause semantics before
   describing it as a paused game. Compare to an original-only baseline.
5. Cancel once by button and once by Escape, reopening between returns. Require
   successful original parent resume and continued battle/input. Exercise
   original Preferences OK/Cancel separately; reject any repeated callback.
6. Open Quit, inspect the original confirmation, choose No and require resume.
   Reopen, choose Yes, follow original exit/results and require fresh Qt Quick
   or Main plus normal Quit. Do not auto-accept the confirmation from the bridge.
7. Kill/retire the host channel with Mini/confirmation open; prove original
   controls remain available and native ownership never reacquires that session.
   Inspect captures and verify immutable originals before/after each experiment.
