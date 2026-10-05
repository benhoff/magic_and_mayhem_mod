# Mini Menu engine bridge: activation deferred

Initial milestone prepared 2026-10-05 at the user's request to work offline without tests.
**Activation is disabled by default. No fixtures, CTests or game sessions were
run for the initial milestone.** Compilation is recorded separately below and does not
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
Opening stays with original engine input; see the follow-up recovery below.

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

## Battle parent recovery and validation follow-up

Reviewed 2026-10-05 after the user authorized isolated checks and a bounded game
run. The extended read-only export is
`working/decompiled/mini-menu-support-u32g7omf/`, with immutable checks before
and after (2927 files). It adds the gameplay vtable, lifecycle, input, wrapper
and one-second callback evidence. Earlier compilation-only evidence above
remains a distinct milestone.

The gameplay singleton is `0x006cbb78`, vtable `0x005c5dd8`, screen ID 2.
Screen push (`0x00557040`) calls the previous owner's suspend slot +8 before
changing ownership and entering the pushed screen. Pop (`0x00557130`) exits
the child and tail-dispatches the restored parent's resume slot +0xc.
The Mini adapter now requires this exact parent object/table/ID. The host wire
still carries only ID 2, and rejects other parent IDs and network context 5.

Gameplay suspend (`0x0046ae90`) clears input/control services through
`0x004a5000`, `0x004d7410(-1)`, `0x004d3c60`, and `0x004d9d60`; it forwards
slot +0x18 to a non-null object at gameplay +0x100b3, uses
`0x00463cb0(1)`/`0x00464080`, and tail-jumps to `0x0056fae0` on `0x006b0198`.
Resume (`0x0046aef0`) clears byte +0x100b2, sets `0x00689938=1`, calls the
paired `0x004d7690`, `0x004d3c70`, `0x004d9da0` services and child slot +0x1c,
uses `0x00463cb0(0)`/`0x00464080`, reloads battle tooltip CFG, and conditionally
restores additional state when byte +0xfdee is nonzero. Service names and
complete timer/audio effects remain unresolved; this is not a native pause bit.

Gameplay input has a branch at `0x0046c1af` that, outside contexts 1/2 and
with byte `0x006c4859` clear, sets Mini object `0x006a5088 +0x53` to 2 and
pushes that object. The decompiler's input values are translated codes, not
necessarily Windows virtual-key codes; live original Escape ingress must be
checked rather than inferred from that switch alone.

Shared Mini tick uses `0x004a5070` as an input/control service (its inspected
body walks controls and mouse state), not evidence of a frozen timer. The
one-second callback `0x00469a60` checks `0x006db8d2`, updates the counter at
`0x005fc868` and notification flags. The alternate world-update wrapper
`0x004757c0` can call gameplay independently of the top screen. Consequently
full simulation/mana/timer pause remains a separate observation milestone.

Reproducible checks added:

- `python3 tools/test-menu-observer.py --mini`: original callback bytes at
  their pinned addresses, unhooked/bridged Cancel, Preferences and Quit;
  ABI/nonvolatile registers on direct calls, adjacent fields, LastError,
  once-only/stale requests, modal/parent/network/special-exit/owner refusal,
  and permanent retirement. Audio/allocation/dialog and common tick services
  are stubbed; this does not validate confirmation UI or parent resume.
- `python3 tools/test-menu-observer.py --mini-disabled`: same V4 installation
  with the compile gate off; require unchanged Mini tick slot and retirement.
- `qt-menu-mini-bridge`: V4 commands, outstanding/stale requests, modal state,
  malformed/unsupported payloads, seqlock and retirement. Existing focused
  V1/V2/V3 bridge, Spellbox and Mini widget checks remain separate regressions.
- An initial Quick Battle Mini harness was attempted, then removed after the
  original Escape path disproved its premise. Campaign Mini live validation
  needs a separate recovered campaign ingress harness; do not reuse Quick
  Battle as a substitute.

Initial fixture `run-bp_sfkon` failed because the new V4 fixture had not staged
its inherited V3 spell-hook bytes; the runtime correctly retired the channel.
The fixture now initializes both inherited hooks before installation. Corrected
Mini evidence is `working/tests/menu-observer/run-i2w0bcmf`; disabled-gate
checks are `run-c4tad_bw`. Six focused Qt checks passed in
`working/build/qt-shell-mini`. Live results are recorded below when complete.

The attempted live Mini run `working/tests/live-menus/run-o8kl528p` (experiment
`run-kyb6a173`) never acquired a native Mini screen and hit its bounded deadline.
Its 34-second capture shows original Game Over. Full instruction-reference
recovery in `mini-menu-support-swnxspvl` found only one normal gameplay ingress,
which excludes contexts 1/2. The input function translates Escape to scan code 1
via MapVirtualKeyA; in those contexts it sets gameplay +0x100b2/+0x10326 and
requests the original Quick Battle results flow instead. Other Mini object
references use special mode +0x53=4 or construction/destruction. No context
flags were changed to force Mini eligibility.

This is evidence against the proposed Quick Battle Escape → Mini validation
path. Campaign Mini ingress/parent behavior, live confirmation and pause remain
unvalidated. Keep Mini activation OFF. The next supported live flow is now
[Quick Battle results](quick-battle-results-engine-bridge.md), using V5 with
Mini still independently disabled. Normal live sessions therefore use V5;
V3 compatibility harnesses remain available. Earlier default-V3 statements in
this document describe the initial preparation milestone.

## Campaign gameplay activation (UI31)

[Separate V9 Cancel integration](campaign-mini-cancel-engine-bridge.md) now admits the observed campaign gameplay Mini (mode 2, layout 0, context 5, World parent). Context 5 is confirmed for campaign and cannot be labeled network-only. Original Escape ingress and native Cancel/Escape World returns are tested automatically. V4 remains experimental; Realm mode 4, other actions, confirmation/quit and timer pause remain pending. Earlier evidence and failed Quick Battle ingress attempts are retained.
