# Experimental campaign Portmanteau

V21 extends the existing original mode-1 Quick Battle selector with guarded
mode-0 campaign callers. It requires the original singleton `006f2aa0`,
vtable `005c775c`, Main / Realm [/ Region] / Portmanteau stack (depth 4/5),
context 5/layout 0, loaded named Celtic/Greek/Medieval Realm resources and a
bounded active primary wizard in its 80-record table. Tutorial/modal, carried
item and pending transitions remain unavailable. Earlier versions refuse mode 0.
The V3 inventory, recipe and original name lanes are unchanged; a new 32-byte
lane carries mode/caller/depth/context. Campaign seconds are -1; a countdown
has no meaning in this mode. Widget Done and Escape accept the local draft;
Reset edits is a local draft policy. Original callbacks `00576380`, `005765d0`,
`00579710` and `00576810` retain assignment and acceptance behavior. Original
teardown `005782a0` retains snapshot `00575890` and spell-list rebuild `00575940`.
The bridge hands control to the original caller (handoff 4), and a fresh accepted
Realm or Region can restore native commands. Campaign acceptance never signals
a Quick Battle start. ACK sequence and engine thread must match.

The recovered initialization, controls and teardown are documented in
[spell selection](spell-selection-menu-bridge.md) and the existing hash-pinned
export `working/decompiled/spell-menu-support-zfvck0xg`. Headless evidence `UI63.campaign-spellbox` records 12 private original cases,
11 guard refusals,12 synthetic Qt cases,four regressions and PE32/Qt builds in
`campaign-spellbox-20261012.json`. The earlier compile failure is preserved in
`working/tests/campaign-spellbox/run-42_8ae9w`; C++ token spacing and the fixture
unused variable were corrected before the fresh passing run.
The new private fixture uses authored resources, controls and wizard records;
drawing, tooltip, audio, text service and fade dependencies are explicit stubs
with expected original prefixes. It executes the unchanged original assignment,
OK and control-snapshot bodies. It does not execute full teardown, rebuild,
loaded campaign initialization, save/reload or original rendering. Synthetic Qt
peers are separate from actual campaign evidence. Broader Grimoire knowledge,
research costs/progression and the mission-end renderer failure remain pending.
