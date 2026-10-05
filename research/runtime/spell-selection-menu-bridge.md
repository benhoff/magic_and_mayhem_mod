# Live pre-battle spell selection

The Qt Portmanteau now has a separate live controller for Quick Battle's
Single Player pre-battle selector. Campaign and multiplayer selectors remain
outside this adapter. Original menu drawing and battle loading remain active.
No recipes, costs, inventory generation or gameplay balance are changed.

## Recovered contract and confidence

Static evidence comes from the pinned No-CD executable, exported with
`tools/export-spell-menu-support.py` and `ExportSpellMenuSupport.java`.
Complete disassembly/decompilation is in
`working/decompiled/spell-menu-support-zfvck0xg/`; the exporter verifies all
2927 immutable inputs before and after reading the binary. Addresses below
are build-specific, not stable pointers for arbitrary executables.

The screen is ID 7, singleton `0x006f2aa0`, vtable `0x005c775c`.
Its tick is `0x00576830`, vtable slot `0x005c776c`. Unlike the root/setup
menus, its pending screen and return flag are at `+0x12` and `+0x3e`.
Mode `+0x250 == 1` identifies the supported Quick Battle path.
Initialization/fade, current screen ownership, tutorial modal `+0x264`,
carried ingredient `+0x94` and carried source `+0x42` guard readiness.
Player `+0x90` must identify one of the four pinned player records.

Three counts at `+0x84` authorize at most seven talismans per alignment.
Control arrays `+0x218/+0x21c/+0x220` have stride `0x86`; cell `+0x7a`
is alignment and `+0x7e` ingredient. The 25 shelves at `+0x214` have stride
`0x82` and ingredient at `+0x7a`. These controls are authoritative;
`+0xb4` is a cache of 63 physical assignments, refreshed during teardown.
Every offered ingredient (ID 0–20) must occur once across the controls.

Recipes come from the original 63-word table `0x006f4c08`, indexed by
`ingredient * 3 + alignment`. Original CFG getter `0x0058a680` supplies
ingredient names from `0x006f4d48` and spell names from `0x006f4f60`
(index spell ID + 5). The native artwork index is original recipe ID + 2.
The adapter validates original control callbacks and receiver pointers.

Talisman callback `0x00576380` takes `0x7fe + alignment * 7 + slot`;
shelf callback `0x005765d0` takes `0x7d3 + shelf`. Original slot setter
`0x00579710` updates recipe artwork and display state. The adapter brackets
source and destination callbacks with original held-input flag `0x006f5288`
set to 1 then 0, on the engine thread, requiring no existing held input.
It first returns existing assignments to shelves, then installs the requested
loadout. These callbacks retain original carried state, sounds and tooltips.
The complete request is checked for duplicates, unavailable ingredients and
unavailable slots before any callback runs. Callback failure is not an undo
transaction; unexpected state fails into the original viewport.

Original OK callback `0x00576810` is a thiscall with one stack argument
(`ret 4`), not the zero-argument tick ABI. It sets return flag `+0x3e`.
Original shutdown `0x005782a0` snapshots controls through `0x00575890`,
then rebuilds the player's spell list through `0x00575940`; original setup
resume loads battle. Original Escape also accepts/continues. Qt therefore
uses **Reset edits** for local draft reset and **Start battle** for acceptance.
The shell does not invent a cancel-to-setup action.

These are high-confidence static findings for the pinned build. Isolated
callback execution and live acceptance are separate evidence below.

## Versioned bridge and ownership

`protocols/include/mnm/menu_v3.h` preserves V2 lanes and map/setup payload,
using magic `MNMMCMD3` and size 65536. Host assignment words at byte 40000
share the host seqlock. Engine spell payload at byte 18000 shares the engine
seqlock. It contains owner index, original remaining seconds, ingredient
bitset, three counts, 63 assignments, 25 shelf entries, 63 recipes and bounded
Windows-1252 names. Empty entries are `UINT32_MAX`. No pointers cross the
channel. V1 and V2 clients remain supported. Integers are little-endian
32-bit words; the existing lanes carry sequence, lease and acknowledgement.
The spell payload offsets below are relative to byte 18000.

| Offset | Field |
| --- | --- |
| 0, 4, 8 | Player index, signed original remaining time, offered ingredient bitset |
| 16 | Three talisman counts |
| 28 | 63 signed assignment words, 21 per alignment |
| 280 | 25 signed shelf words |
| 380 | 63 original recipe IDs |
| 632 | 21 names of 128 bytes each |
| 3320 | 63 spell names of 128 bytes each |

Names are NUL-terminated Windows-1252; unused payload bytes are zero. Host
assignments at byte 40000 use the same physical layout and empty sentinel.

Semantic action 12 submits the whole loadout. Lease, generation, readiness,
thread ownership and once-only request guards remain in force. Timer changes
publish fresh seconds without invalidating generation or replacing local Qt
edits. Setup Start waits for ready screen 7; spell acceptance suspends commands
and exposes original battle/loading. A fresh ready Main/Quick tick restores Qt.
Automatic original timer completion likewise hands control to the viewport.
Explicit **Use original menus** permanently retires the session channel.
The original timer can finish selection automatically. Unsubmitted Qt edits
remain local and are discarded in that case; use **Start battle** to commit
them before the deadline. Timer expiry with a local draft has not been live
validated. Closing from this selector exposes the original viewport, since
there is no original cancel-to-root action.

Presentation is `SpellboxWidget`; `LiveSpellMenuController` translates typed
inventory/loadout intent; `LiveMenuSession` owns lifecycle; `MenuBridge` owns
wire encoding. Pinned addresses and original gestures remain in `runtime/menu/`.

## Validation

Fourteen selected Qt CTests pass, including the new `qt-menu-spell-bridge`,
existing V1/V2 bridge checks, Spellbox, setup and map checks. The new synthetic
V3 check covers malformed counts/pools, whole-loadout rejection, extra host
lane publication, outstanding-request refusal, local reset, timer-only draft
preservation and permanent retirement. These are synthetic checks.

`tools/test-menu-observer.py --spells` executes extracted original shelf,
talisman, slot setter and OK bytecode at pinned PE32 addresses. Drawing,
tooltips, audio, text catalog access and normal tick/loading are fixture stubs.
Runs `working/tests/menu-observer/run-kpy2sskg/` and
`working/tests/menu-observer/run-bhuk0isy/` passed; the latter additionally
checks removal of an existing assignment and held-input rejection. Earlier
`run-z4zulzln/` failed after callbacks due to calling OK with the tick ABI;
correcting the required stack argument resolved it. Keep that failed run as
ABI-discovery evidence, not as live success.

An initial live run `working/tests/live-menus/run-i6mx2aye/` reached Qt
selection, accepted the requested control assignments, showed chosen spells
in the battlefield HUD, restored Qt Quick and completed original Quit. It
exposed a duplicate battle-start notification at the later loading transition.
The session now guards automatic handoff with `!inBattle_`, and the harness
requires exactly one handoff and one return. This first run is discovery
evidence; the corrected final round trip is recorded below. Physical desktop input/focus, every ingredient/recipe,
campaign/multiplayer selection, previews, native in-battle UI and long play
remain separate milestones. The standalone supplied-inventory preview retains
its existing API and evidence in `spellbox-qt.md`.


The corrected live run `working/tests/live-menus/run-dl0en6mq/`, staged as
`working/experiments/menu-observer/run-saejiozo/`, passed the independent
validator and before/after checks of all 2927 original files. It reached ready
screen 7 at acknowledgement 12, exercised native assignment and Reset edits,
then selected ingredient IDs 0 and 1 into physical talisman positions 0 and 1.
Original shelf/talisman traces were `(2003,2046,2004,2047)`, followed once by
original OK (semantic action 12). All 63 published original control assignments
matched the Qt request. The inspected battlefield capture at 25 seconds shows
the selected spells in the original HUD. This is control/HUD evidence; a
separate byte comparison of the rebuilt player spell list or actual casting
was not performed.

The harness counted exactly one battle handoff and one native return. Original
result controls returned to Qt Quick (screen 22, acknowledgement 13), followed
by original Back/Quit and final acknowledgement 15. Launcher status was 0;
no smoke/deadline termination was used. The inspected Qt stage-12 and stage-13
captures show native selection and restored native Quick respectively. The
report includes initial shelf order, requested and engine assignments, exact
callback trace and artifact hashes. Recheck it with:

```bash
./tools/test-live-menus.py --battle spells --validate-run working/tests/live-menus/run-dl0en6mq
```

High confidence applies to this bounded Single Player path. Cross-alignment
assignment and removal have isolated callback evidence; a live all-recipes,
all-alignments sweep remains outstanding. Original menu drawing was retained.

The unchanged V2 setup/map fixture also passed after this integration at
`working/tests/menu-observer/run-y2lvk1r4/`, with before/after immutable checks.
