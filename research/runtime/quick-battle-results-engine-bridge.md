# Quick Battle results engine bridge

Implemented 2026-10-05. Normal Qt live sessions use a V5 results bridge, while
Mini integration remains disabled. The supported scope is original context 1,
the original Quick Battle results object directly over the gameplay singleton,
and engine-present Continue/Quit controls. Spectate/multiplayer, campaign results,
portrait sprite binding, complete victory conditions and longer play remain
separate. No gameplay balance or simulation timing changes are introduced.

## Why results came next

The proposed Mini validation run reached original Game Over after Escape.
Static recovery confirms that contexts 1/2 bypass the normal Mini ingress and
request Quick Battle results. See [Mini recovery and failed live path](mini-menu-engine-bridge.md).
The integration follows that existing flow instead of changing engine modes.

## Pinned recovered contracts

All addresses apply only to No-CD SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
`tools/export-quick-result-support.py` creates a fresh disassembly/vtable/reference
and read-only Ghidra export, verifies the hash before/after, hashes evidence and
verifies all 2927 immutable originals around consumption. Selected final export:
`working/decompiled/quick-result-support-h7nwohsg`.

| Contract | Evidence |
| --- | --- |
| Constructor / singleton / ID | `0x00474c70` / `0x006deec8` / 26 |
| Vtable / tick slot | `0x005c5ef4` / `0x005c5f04` → shared `0x005595d0` |
| Original data/world update slot +0x2c | `0x004757c0`; updates display through `0x004759f0`, conditionally forwards gameplay when byte +0x84 is zero |
| Entry / exit / resume / suspend | `0x00474e70`, `0x00474ea0`, `0x00474eb0`, `0x00474ee0` |
| Initialize / draw / button callback | `0x00475100`, `0x004757f0`, `0x00475860` |
| Labels / buttons / callback object | object +0x4f / +0x57 / +0x4b |
| Registered callback | vtable `0x005c5f58`, function +4, owner +8 |
| Control callback / local index | control +0x25 / +0x2d |
| Display string ownership | label +8, populated by `0x004d28c0` |

The constructor sets ID 26. Initialization allocates 26 labels, four portraits
and three buttons, registers the original callback and removes controls according
to bytes +0x83/+0x84. The three local indices are Spectate 0, Continue 1 and Quit 2.
The adapter rejects an existing Spectate control and publishes only present
Continue/Quit controls. It verifies every offered receiver/function/index.

The callback preserves ESI and returns with `ret 4`. It calls the original sound
service, sets return flag +0x43 for indices 0/1, and for index 2 also sets byte
`0x006dbc1a=1`. The bridge invokes this callback rather than writing either flag.
The original common screen tick, draw, control input, results updater, parent
resume and subsequent teardown all remain active.

`0x004759f0` sorts original player pointers, skips No Player slots and fills
column-major label rows: names 6–9, kills 10–13, deaths 14–17, handicap 18–21,
scores 22–25 (zero-based). The adapter copies the current bounded NUL-terminated
CP1252 strings from those actual display labels. Qt neither sorts rows nor
calculates scores. Empty names suppress inactive rows. Portraits still use
native text placeholders, matching the existing preview boundary.

## Wire and application boundaries

`protocols/include/mnm/menu_v5.h`: magic MNMMCMD5, size 73728; V1–V4 offsets
remain unchanged. The seqlock protects a 2608-byte results payload at byte 42100:
offered action bits (Continue=1, Quit=2), original context, depth 1–15, reserved
zero, then four 648-byte rows. Each row contains active boolean, reserved
portrait index UINT32_MAX and five 128-byte NUL-terminated display strings.
Actions 16/17 are semantic Continue/Quit, mapped to indices 1/2 only in the runtime.

Runtime readiness requires the current owner, expected object/table/ID, context
1, original gameplay singleton `0x006cbb78`/table `0x005c5dd8`/ID 2 as parent,
valid controls/labels, initialized state and no fade/pending/replacement/return
or original confirmation. Payload/ownership/readiness changes advance generation.
Engine thread, lease, seqlock, acknowledgement, stale/outstanding/once-only
requests and permanent fallback retain their existing guards. The slot is patched
only for V5, after verifying its original value and callback prologue; original
common tick is forwarded. Unsupported context remains in the original viewport.

MenuBridge validates all payload fields and allowed actions. LiveMenuSession
reacquires a fresh ready results screen after battle and returns to the original
viewport after either acknowledged callback. The engine alone decides the next
screen. LiveResultMenuController supplies display-only data and semantic actions
to QuickBattleResultWidget. Repeated display refreshes preserve an available
focused action. Original-results compatibility harnesses explicitly keep V3;
normal live sessions use V5. Window close on a ready supported results screen requests original results Quit,
then follows the existing Quick → Main → Quit path. Widgets contain no engine
addresses or wire offsets.

## Validation and limits

Isolated original-bytecode fixture:
`python3 tools/test-menu-observer.py --results`, evidence
`working/tests/menu-observer/run-0tgcwxf4`. Unhooked/bridged Continue/Quit match
return/quit effects, direct ABI/nonvolatile-register and LastError checks;
bridged once-only/stale commands, parent/context/receiver/Spectate/modal refusal
and retirement pass. Common tick, sound, display and world services are stubbed.
Initial fixture runs `run-8wcc_ub3`/`run-ehmlvaic` faulted because fixture executable
permissions began at 0x004a0000, above the recovered callback. The script-controlled
fixture now protects its reserved range from 0x00470000. These failures were in
the synthetic PE image; no original binary was modified. V3 regression evidence
with final observer source is `run-m9hdq0p1`; V2 setup/map regression evidence is
`run-42xvmie4`. Eight focused Qt checks pass, including refreshed action focus,
malformed V5/V4 wires, inherited V1/V2/V3 bridges, Spellbox and result widgets.

Live run `working/tests/live-menus/run-z2r15j5w`, experiment `run-d_cz6_tp`:
Qt Main/Quick/setup/map/controls → original battle → original Escape → native
results ready ID 26/ack 12 → native Continue → original gameplay → Escape →
native results ack 13 → native Quit → Qt Quick ack 14 → Back/Main/Quit.
Final ack 16, one engine thread, exact original callback trace ending
(26,1),(26,2),(22,3),(3,4), launcher status 0 and retired channel. Raw final engine
rows match the captured report. Captures `qt-battle-stage-12.png`,
`original-results-continue.png` and `qt-battle-stage-14.png` were inspected.
They show actual rows, returned gameplay and restored native navigation.

Reproduce:

```bash
python3 tools/export-quick-result-support.py
python3 tools/test-menu-observer.py --results
ctest --test-dir working/build/qt-shell -R 'qt-menu-(result|mini|battle|spell)-bridge|qt-menu-result-controller|qt-menu-bridge|qt-quick-battle-results|qt-spellbox' --output-on-failure
python3 tools/test-live-menus.py --battle results
python3 tools/test-live-menus.py --battle results --exit-from results
```

High confidence within recorded bytecode/wire/action/live ownership scope.
This does not establish full pause semantics, timer/audio freezing, original font
or portrait equivalence, Spectate/network/campaign behavior, hardware keyboard
or mouse equivalence, complete victory/defeat paths or long-session reliability.
Original drawing remains active beneath native presentation; explicit fallback
permanently returns ownership to original controls for that session.

Final harness run `working/tests/live-menus/run-yw3m6of9`, experiment
`run-055p1cup`, also passed with the same original callback/ack/normal-exit
requirements. This run additionally checked actual Qt label text against each
engine display row at both native results presentations. Controller refresh/focus
and focused bridge/widget regression checks are recorded in the coverage ledger.
The score rows in these short sessions have zero kills/deaths/scores; nonzero
values have synthetic display coverage, not live scoring equivalence.

Window-close run `working/tests/live-menus/run-8tb0ynfd`, experiment
`run-uuzr8cg2`, passed the same callback/ack/display and normal-exit checks.
Closing the Qt window on the second results presentation dispatched original
results Quit, then Quick Battle Back and Main Quit, with no duplicate action.
