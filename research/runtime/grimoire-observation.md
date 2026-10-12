# Grimoire live observation preparation

The opt-in probe forwards the pinned original Grimoire tick `0x4c7890`, through
vtable slot `0x5c6b80`, and records bounded before/after raw state. It does not
dispatch actions, claim native book ownership or bypass original drawing/logic.
The [contract](grimoire-engine-contract.md) motivates a separate observation:
Grimoire's packed base, return field and custom tick differ from standard menus.

The proposed diagnostic wire is `MNMBOOK1`, version1, 128-byte records following
a 16-byte header. Each record has 32 little-endian words. Addresses are research
values, not host pointers or native application object layouts. The sequence is
bounded to256; unchanged states are deduplicated separately before/after tick.
Screen ID, initialized and cursor-aux fields are raw observations and are not
assumed to establish readiness, knowledge or page/level identity.

| Word | Value |
| --- | --- |
| 0–5 | Sequence, phase1/2, original thread, receiver, vtable, raw screen field |
| 6–9 | Current owner, depth, parent, owner's raw screen field |
| 10–13 | Common initialized/fade/counter and custom return `+0x1e` |
| 14–18 | Catalog/display pointers, chapter `+0x42`, section `+0x46`, unknown cursor auxiliary `+0x4a` |
| 19–23 | Transition states, temporary-control count, realm-request alias `+0x1a2e`, context `+0x1a3a` |
| 24–27 | Allocated callback vtable/method/receiver and global campaign context |
| 28–31 | Realm request/kind/region and preserved LastError |

Receiver `0x6e0088 + 0x1a2e` is exactly global `0x6e1ab6`; the two recorded
request values intentionally expose that alias. It is not an ignored idle
counter. All raw-state changes remain visible; only LastError is excluded from
deduplication. Missing opt-in leaves the original slot untouched. Installation
checks the relevant vtable prefix, exact tick entry bytes and target slot before
obtaining protection and creating a new file. It writes only that one slot and
restores page protection. Receiver/type/readability checks protect capture,
while rejected captures still forward the original tick.

The synthetic private fixture validates installation rejection, correct slot
replacement, 304 stubbed tick forwards, return/LastError preservation, unchanged
state dedup despite changed LastError, rejected receiver capture, serialized
parent return and the 256-record bound. The actual original tick is replaced
only in the private fixture after expected-prefix verification; this is native
probe validation, not original tick/drawing comparison. Earlier link-input and
incorrect alias-dedup attempts remain retained, separate from the corrected run.

The [settled-page live result](grimoire-settled-live-observation-20261012.json)
passes original Main/NewGame/Region/Grimoire/Escape/Region twice, then Region
Cancel/Main/normal Quit. Both opens reach the visible Introduction contents with
transition fields zero, catalog/display present and callback
vtable5c6b8c/method4c77c0/receiver6e0088. One original thread owns the book at
depth5 with Region18 below it. Both raw screen and common initialized fields stay
**zero even when the book is fully readable**. Native admission must therefore
use the book's identity/resources/ownership contract rather than those fields.

Escape sets custom return `+0x1e`; original fade reaches counter6, tick pops to
Region18/depth4, cleanup clears catalog/display/callback/context and temporary
controls, and Region readiness returns. Catalog/helper addresses happened to be
reused across the two opens; allocated context addresses changed. Their reuse
does not make them stable across launches. The [earlier live result](grimoire-animation-close-observation-20261012.json)
closed during the opening animation and is retained with that narrower scope.

The raw `+0x4a` value is1 for these contents, while chapter/section are0/0 and
four temporary entry controls exist. This does not identify cursor-aux as a
declared article level or validate arbitrary entry/page/knowledge/dynamic text.
Native reader projection, formatted dynamic values, other callers/realms,
mouse Close/page navigation and full lifecycle/cadence comparisons remain
separate later steps. Original graphics/audio/rendering and every original tick
body remain active; no native book ownership or replacement is asserted.

The [final synthetic fixture](grimoire-observer-final-fixture-20261012.json)
pins current probe/decoder sources; the [existing Main/Quick regression](grimoire-default-menu-regression-20261012.json)
also passes with the book opt-in absent. The generated source census records
implementation/test links, while the committed-history audit preserves the
reviewed `b49bc43..556ded9` versions separately from these current changes.
