# Original Grimoire control discovery

Build: No-CD `40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
This is an incremental investigation, separate from the offline
[native reader](grimoire-qt.md). The reader currently exposes installed entries
with native pagination and placeholder dynamic fields; those are not recovered
campaign knowledge, engine page identity or caller-return contracts.

The first candidate `0x006c5148` is **Character Screen**, not Grimoire. Its
constructor `0x004b3900` installs vtable `0x005c66b4` and ID20; initialization
`0x004b43e0` loads CharacterScreen resources and upgrade-step costs. Earlier
campaign research reversed the two destination names. Original addresses and
historical records remain unchanged; this investigation corrects their labels.
Character initialization candidate `0x4b43e0` has no owning function in the pinned
discovery inventory. Its literal resource identity is linked through the final
vtable, while the body stays unassigned. Full initialization, stat/upgrade/cost
semantics and native Character integration remain pending.

Actual **Grimoire is `0x006e0088`**, constructed by `0x004c3620` with final
vtable `0x005c6b70`. Initialization `0x004c3a40` loads Grimoire.cfg, Grimoire.txt,
tooltips, book/art/tab/page-turn resources and the original text/catalog state.
Region Entry action2 requests it and records realm/region context. Action3 is
Spellbox/Portmanteau (`0x006f2aa0`); action4 is Character Screen. Installed Region
Entry config labels corroborate that standard-button order. Realm auxiliary
action3 requests Grimoire; action2 instead requests Character Screen.

Grimoire has a different packed layout from standard menus: parsed catalog at
`+0x16`, text/display helper `+0x1a`, return DWORD `+0x1e`, surfaces
`+0x26/+0x2a/+0x2e/+0x32`, page transition state `+0x3a/+0x3e`, current chapter
`+0x42` and section `+0x46`, callback object `+0xa00`, temporary control count
`+0x1a2a`, activity/reset `+0x1a2e` and context `+0x1a3a`. Standard-menu
`+0x33/+0x43` pending/readiness assumptions do not apply. The constructor does
not assign the common screen-ID field; its live value still needs observation.
Only nine vtable prefix slots through `+0x20` are reviewed: `4ca300,4ca320,`
`4ca330,4ca340,4c7890,4c7920,4732d0,4ca2a0,4ca2b0`. The exported following
words are adjacent data, not additional claimed methods.

## Original action dispatcher

`0x004c77c0..0x004c7886` is thiscall(receiver, unsigned action), returning that
action and cleaning one argument. If either page-transition state is nonzero,
it clears both and swaps `+0x26↔+0x2e` and `+0x2a↔+0x32` before dispatch.
Every branch clears activity/reset `+0x1a2e`.

| Action | Original behavior |
| --- | --- |
| 3999 | Fade, set return `+0x1e=1` |
| 1999 | Delegate Previous helper `0x004c6db0` |
| 2999 | Delegate Next helper `0x004c6910` |
| Other | Delegate selector `0x004c72d0(action/124, action%124, 0)` |

The private original fixture executes 32 combinations: four transition states
and Close/Previous/Next/five packed selectors. It checks swaps, flags, exact
helper arguments, return value, stack and EBX/ESI/EDI/EBP preservation. Fade,
page selection, navigation and rendering helpers are stubbed. UINT_MAX is
forwarded without a chapter/section bounds check: a native adapter must validate
the actual original catalog before calling this dispatcher. This is dispatcher
evidence, not live pagination, knowledge/stats or complete resource behavior.

## Custom lifecycle, static only

Entry thunk `0x4ca300` directly calls initializer `0x4c3a40`, then clears return
`+0x1e`. Tick `0x4c7890` calls ordinary book update/drawing helpers while return
and fade are clear; returning with fade clear pops through `0x557130`. During
fade it calls original fade work, increments `+0xd` and clears fade `+0xc` after
the previous counter exceeds4. Escape handler `0x4c7920` takes three arguments,
checks WM_KEYDOWN/VK_ESCAPE, fades and sets `+0x1e`. These are static findings;
their live stack/thread/entry/cleanup cadence still need observation. The common
initialized field alone is not established as book readiness.

Native Grimoire admission, original chapter/section/level availability, formatted
dynamic text, live opening/closing and caller returns remain pending. No native
Grimoire ownership is enabled by these static/private findings.

[Identity](grimoire-character-identities-static-20261012.json),
[dispatcher](grimoire-dispatch-static-20261012.json),
[lifecycle](grimoire-lifecycle-static-20261012.json) and
[private execution](grimoire-dispatch-original-20261012.json) retain original
hashes, exact source fingerprints and per-run static artifact identities.
The native source census and reviewed committed range are separate accounting
evidence; they do not validate book integration or replace the original body.

`tools/export-grimoire-menu.py` consumes only the pinned original executable,
records references to the receiver and Grimoire path strings, and retains the
full static disassembly for bounded follow-up. Optional entry exports use a
read-only Ghidra project. Static references are candidates, not verified live
control identities or complete function coverage. The original manifest is
verified before and after every export, including failure.
