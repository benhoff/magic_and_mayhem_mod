# Single Player setup / map action bridge

## Scope and recovered baseline

Opt-in `tools/run-qt-shell.sh --live-menus` extends Main/Quick integration with
Create Single Player, original setup data, Map/Cancel/Start, human portrait and
colour cycling, and removal of opponent slots 3/4. Engine initialization and
random opponent choices remain authoritative. Native rule/handicap edits are a
local draft until Map, a player control, or Start submits all seventeen values.
Returning from Map refreshes the draft from the engine. Original preferences
and QuickBattleSetup persistence operate inside the disposable installation.
No default rule values or balance policies are introduced by the adapter.

Start hands presentation and input back to the original Wine viewport. The
original engine decides between spell selection and loading; Qt neither skips
spell selection nor creates a game itself. Commands suspend during this handoff
while host heartbeats continue. A fresh, ready Main/Quick Battle tick on the same
engine thread, with unchanged acknowledgement and cleared handoff, restores the
native root menu. Explicit fallback and lease retirement remain permanent.
Native spell selection and in-battle menus remain separate milestones.

Contracts below apply only to No-CD Chaos.exe SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Read-only Ghidra/disassembly exports and artifact hashes are in
`working/decompiled/battle-menu-support-zlewbfiq/` and
`working/decompiled/battle-menu-support-meme7vx5/`. Definitions created during
export were discarded. The reproducible exporter is
`tools/export-battle-menu-support.py`. Confidence is high within the selected
callbacks, control storage, and mapped fields; unrelated decompiler types are
not promoted to engine contracts.

| Engine contract | Pinned recovery |
| --- | --- |
| Single Player | object 0x658970, ID 14, vtable 0x5c6534; tick slot +0x10 delegates to 0x5595d0 |
| Setup text buttons | thiscall 0x4ad6a0(owner,index), ret 4, EAX 0; 0 Cancel, 1 Start, 2 Map |
| Setup player controls | thiscall 0x4ad6f0(owner,index); 0 cycles human wizard/name, 1 cycles human colour, 2/3 remove opponents |
| Rule storage | thirteen dwords at 0x6c375c in native row order: Mana, Health, MagicItems, SelectionTime, Law/Neutral/Chaos Talismans, GameTime, Lives, PlacesOfPower, ManaSprites, Artefacts, ControlLimit |
| Players | four records at 0x658f10, stride 0x133; bounded name at +0, wizard +0x15, handicap +0x19, colour +0x1d, lives +0x2e; active name differs from bounded original NoPlayer string at 0x6ead98 |
| Setup controls | slider pointer table owner+0x5f; original current value slider+0x61, min/max +0x65/+0x69, callback +0x25, index +0x2d; text pointer table owner+0x53, map label index 13, label text pointer +8 |
| Rule setter | thiscall 0x4cdf40(slider,value) clamps and updates the original control, then invokes its registered callback 0x4ad860 itself; do not invoke that callback twice |
| Lives side effect | original rule callback index 8 also updates all four player lives records |
| Opponent removal | 0x4ad4b0 resets name/wizard/colour/handicap and frees the original handicap slider; removal has no add-player counterpart |
| Map selector | object 0x690448, ID 25, vtable 0x5c6940; tick slot +0x10 is 0x5595d0; caller ID dword owner+0x15b must be 14 |
| Map catalog | original list owner+0x57; count list+0x49, rows pointer list+0x45, row stride 0x101 (256-byte name plus selected flag), selected index +0x5b |
| Map selection | original thiscall 0x4d1060(list,index) can toggle a selected row; clear with -1 first, then select requested zero-based row |
| Map OK/Cancel | thiscall 0x4bbbf0(owner,index), ret 4; 0 gets selected name through 0x4d1df0 and its first matching row through 0x4d1ed0, stores row+1 at 0x6c3758 and updates original setup label; nonzero Cancel returns without changing the map |
| Start branch | 0x4ae670 uses spell selection if MagicItems and any talisman count are nonzero (pending 0x6f2aa0); otherwise original loading (pending 0x6903b8, return target Quick). Original resume also completes spell-selection-to-loading work |

## Version 2 wire and orchestration

`protocols/include/mnm/menu_v2.h` defines a separate 32768-byte, little-endian
mapping with magic `MNMMCMD2`, version 2. V1 remains supported and independently
tested. No engine pointers, C++ object layouts or widget types cross the channel.

| Byte offset | Owner | Contents |
| --- | --- | --- |
| 0 | initialization | magic (8 bytes), u32 version, u32 size |
| 16 | host seqlock lane | sequence, alive, heartbeat, request ID, action, expected generation, argument, seventeen u32 rule/handicap values |
| 128 | engine seqlock lane | sequence, generation, screen, ready, ack, status, thread, Start destination (0 none, 1 original spell selection, 2 original loading) |
| 160 | engine payload | map ordinal |
| 164 | engine payload | original catalog count (0 when map list is not initialized) |
| 168 | engine payload | thirteen rule values |
| 220 | engine payload | four 48-byte player records: active/wizard/colour/handicap u32, then bounded 32-byte NUL name |
| 412 | engine payload | original map label, bounded 128-byte NUL string |
| 540 | engine payload | up to 128 original-order map names, 128 bytes each |

Names are original Windows-1252 bytes, decoded explicitly by the host. Reserved
bytes initialize to zero. The engine lane seqlock covers both state and payload.
Data changes increment the generation, as do owner/screen/readiness changes.
Malformed pointers/strings/counts suppress readiness. Oversized catalogs or
names fall back rather than truncate or guess a map identity.

Actions 1–3 retain Main/Quick/Quit meanings. V2 adds 4 Open Single, 5 Setup
Cancel, 6 Setup Map, 7 Setup Start, 8 Setup Player (argument 0..3), 9 Map OK
(argument one-based original row), 10 Map Cancel, 11 Apply Setup. Status 6 means
invalid transaction. Shared thread, owner, generation, monotonic request, lease,
fade and no-retry guards are the existing Main/Quick contract. A request is
consumed even on rejection.

All seventeen settings are checked before any original setter runs. Controls
must have readable recovered bounds and the expected rule callback, receiver
and index. A missing inactive opponent handicap slider is accepted only with
value zero. Only changed existing controls call the original setter. Engine
clamping is preserved: snapshots need not be aligned to the native slider step,
so `setEngineSetup` accepts bounds-valid original defaults without snapping.

Single/Map callbacks are direct calls to pinned original code, surrounded by
bounded observation records. Only their selected tick vtable slots are hooked;
Main/Quick retain their existing callback trampolines. Launch staging pins the
complete executable and DLL hashes; added slots and callback prologues are
checked before hook writes. Hook failure retires the adapter.

`LiveBattleMenuController` translates semantic widget signals and authoritative
snapshots; `LiveMenuSession` handles process and command lifecycle; `MenuBridge`
handles wire data. Widgets contain no game offsets. The embedded live viewport
stays at the launcher's 800x600 game size rather than resizing the Wine desktop
to the surrounding shell; Start activates that viewport for original input.

## Validation and remaining boundaries

Static recovery, synthetic bytecode checks, native wire/model tests, and live
execution are separate evidence. Reproduction:

```bash
cmake -S apps/qt-shell -B working/build/qt-shell
cmake --build working/build/qt-shell --target mnm-qt-shell menu-battle-bridge-test single-player-battle-test --parallel 4
ctest --test-dir working/build/qt-shell -R 'qt-(menu-bridge|menu-battle-bridge|single-player-battle|map-selection|main-menu|quick-battle-menu|shell-help|shell-startup)' --output-on-failure
./tools/test-menu-observer.py --battle
./tools/test-menu-observer.py --channel
./tools/test-live-menus.py --battle direct
./tools/test-live-menus.py --battle spells
```

The V2 PE32 fixture executes original setup/map button, slider setter/rule
callback, and list select/get/index bytecode at pinned VAs. It verifies an
invalid final value rejects the whole transaction, Lives propagates to players,
human handicap updates, map bounds/OK, Cancel, Start dispatch and missing
inactive handicap guards. Display/text services, float conversion, common fade
helper, tick and actual Start loading are stubbed: this is ABI/data guard
validation, not whole-engine equivalence.

The live direct scenario deliberately chooses Mana 150, Lives 3, human handicap
10 and MagicItems 0 in a disposable copy, navigates Cancel and Map Cancel/OK,
selects original map row 2, cycles human wizard/colour, removes slot 4, and
Starts. These values are test input, not new defaults. The spells scenario keeps
the engine's MagicItems/talisman defaults. Live screenshots require inspection
before claiming battle loading or original screen equivalence.

Eight targeted Qt CTests passed (wire V1/V2, setup, map, Main, Quick, help,
startup). V2 bytecode/guard checks passed at
`working/tests/menu-observer/run-9y9jobte/`, including removal's absent-slider
cases. An earlier V2 guard pass is `run-3yyi41qk/`.

The first live direct run, `working/tests/live-menus/run-7kqc5axf/`, verified
setup/map edits and player cycling but rejected a second removal request when
its slider had been freed. Static recovery confirmed removal has no add-player
behavior; native inactive removal buttons are now disabled and missing inactive
sliders are permitted only with zero handicap. The first PE32 attempt at
`working/tests/menu-observer/run-h70mgta3/` used an incomplete original list
lookup byte range and failed on its omitted success-return block; the exporter
fixture range was corrected before the passing runs.

Direct live integration passed at `working/tests/live-menus/run-iltd_t6r/`,
experiment `working/experiments/menu-observer/run-cgd94p5j/`: requests 1–12,
setup Cancel, Map Cancel/OK, native edits reflected in original snapshots,
original wizard/colour/remove callbacks, original Start destination 2. The
10-second post-Start X11 capture was inspected and shows real forest terrain,
wizard and battle HUD. That capture also exposed stale menu pixels beyond the
800-pixel original game surface when Qt enlarged the desktop; the host now fixes
its live game container to 800x600. This initial capture is evidence of loading,
not evidence that the old enlarged viewport was visually correct.

Default spell-selection integration passed at
`working/tests/live-menus/run-gmb8_jop/`, experiment
`working/experiments/menu-observer/run-kt7m5xqu/`: the same native navigation and
edits while retaining original item/talisman defaults, followed by original
Start destination 1. The corrected-size 25-second X11 capture was inspected and
shows the original ingredient/talisman selection screen with clean viewport
edges. Each launch uses a distinct Wine prefix; the observed thread ID is not
assumed stable across sessions. Both tests retain original drawing and gameplay.
All 2927 immutable source files verified before and after each completed run.

V1 command guards still passed at
`working/tests/menu-observer/run-2hagp_c6/`. The final extended V2 fixture at
`working/tests/menu-observer/run-e1x73hzu/` also checks Main Quit and Quick
Create Single Player through V2 before exercising setup/map transactions.

Extended direct run `working/tests/live-menus/run-sc7ci36s/` inspected clean
battle and original result presentation after the viewport fix, but did not
prove return to a menu. Follow-up `run-bp0wkz_0/` used genuine XTest input,
reached the original Game Over screen, then selected Continue: its 45-second
capture shows play resumed. The explicit menu-return assertion failed. This
is observed original behavior for that run, not a basis for treating the native
result preview's caller navigation policy as recovered engine behavior. The
return fixture now uses original result Quit and requires an observed Main or
Quick tick after the retired-channel handoff.

Final direct battle/return validation passed at
`working/tests/live-menus/run-mwd9kjvo/`, experiment
`working/experiments/menu-observer/run-fpi80gue/`. Genuine XTest input used the
original result Quit button; the 45-second X11 capture was inspected and shows
Quick Battle. The retired channel subsequently published screen 22 with request
12 still acknowledged and status 5, confirming the original Quick tick resumed
without reactivating native commands. The process remained alive for the bounded
90-second run. The 10-second capture shows the actual battlefield inside the
fixed-size viewport. This establishes the selected original return path, not
native result/menu replacement or longer gameplay equivalence.

## Automatic native menu restoration after battle

The V2 channel stays alive after acknowledged Start. The application suspends
all semantic requests and continues host heartbeats; it leaves spell selection,
battle and result controls in the original viewport. No battle tick hooks are
added. The absence of supported menu publications during play does not trigger
the application's menu-state timeout. The launch process still controls session
completion, and explicit fallback/engine retirement remain permanent.

On a ready original Main/Quick tick, the adapter clears the handoff marker.
The application requires a new publication and generation, the same engine
thread, the unchanged Start acknowledgement, successful status, and a ready
supported root menu before restoring native presentation. Reopening setup then
refreshes actual rules, map and newly generated players through the existing
snapshot/generation checks. Request IDs remain monotonic across both battles.
No pointers, gameplay policies, or original menu drawing are replaced.

Pinned original-bytecode fixture `working/tests/menu-observer/run-ny02fmyj/`
passes the new return-marker and subsequent Create Single Player checks;
eight targeted Qt checks pass. Before/after manifest verification reports all
2927 original files intact.

First live repeat attempt `working/tests/live-menus/run-5mlmtljq/`, experiment
`working/experiments/menu-observer/run-xatxcfdi/`, restored Qt after the first
battle and loaded the second battle with fresh generated players. Inspected
captures confirm both battlefields and first Qt Quick return. Its 120-second
launcher bound expired before the second return: the report correctly fails.
The repeat protocol now allows 180 seconds and rejects a timeout-driven exit;
passing requires two native returns and original Back/Quit with launcher status 0.

Second repeat attempt `working/tests/live-menus/run-krsebyaq/`, experiment
`working/experiments/menu-observer/run-ecmy3az_/`, reached both native returns
and original Quit (acknowledgement 16). Inspected second-battle capture shows
actual gameplay. The launcher classified successful early game exit as a smoke
failure and returned status 1. The repeat harness now uses normal launch with an
independent 180-second deadline; smoke mode retains its existing survival policy.


Completed repeat evidence: `working/tests/live-menus/run-6xzs_hiy/`, experiment
`working/experiments/menu-observer/run-tmjlduru/`. The corrected validator passes
on the recorded live evidence: native stages through acknowledgements 0–14,
two Start handoffs, two ready Qt Quick returns, regenerated second-setup players,
original Back/Quit callbacks, final acknowledgement 16, retired channel and normal
launcher status 0. Both battlefields and the second restored Qt Quick screenshot
were inspected. Original manifest checks before and after the run preserve all
2927 files. The final channel screen is 0 after shutdown; return proof comes from
the ready stage-12/stage-14 snapshots and captures, not the post-Quit screen.
The original post-run assertion incorrectly required a menu after shutdown and
was corrected; raw live evidence was retained and rechecked with:

```bash
./tools/test-live-menus.py --battle direct --battle-repeat --validate-run working/tests/live-menus/run-6xzs_hiy
```

Confidence is high for this bounded repeated direct-loading path. Longer play,
physical desktop input/focus, spell-selection return paths and native in-battle
menus remain unvalidated. Explicit fallback remains permanently original for
that session; original logic/drawing and balance remain unchanged.

## Follow-on spell selector

The shell now uses the separate [V3 pre-battle spell bridge](spell-selection-menu-bridge.md) for Quick Battle Single Player selection. The V2 contract above remains supported. UI19 records the recovered inventory/recipe bindings, original selection callbacks, bounded live Start/return/Quit validation and remaining timer/campaign boundaries.
