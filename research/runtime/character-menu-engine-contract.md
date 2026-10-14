# Campaign Character action contract

Pinned No-CD executable SHA-256:
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Character singleton `0x006c5148`, ID20, vtable `0x005c66b4` retains the identity
established by the earlier [screen discovery](grimoire-engine-contract.md).
This increment investigates original callback `0x004b5470`, independently of
the [native preview's supplied costs](character-screen-qt.md).
[Read-only control export](character-controls-static-20261012.json) retains the
initializer, drawing, tutorial/input/control-processing candidates and reviewed
vtable prefix, with artifact and source hashes. Static interpretation remains
separate from the original action execution below.

## Selected original action branches

The callback uses the original wizard-selection helper `0x005908e0` and wizard
stride `0x93a`. Draft experience is receiver `+0xa7`. Three stat-bar records
start at the pointer `+0x7f`, stride `0x2c`: opening value `+0x24`, edited value
`+0x28`, maximum `+0x20`. Three talisman records start at pointer `+0x83`, same
stride: opening value `+0x18`, edited value `+0x1c`, maximum `+0x14`.

| Action | Branch inspected in original code |
| --- | --- |
| 0, OK | Commit draft experience and all six rows to the selected wizard, advance control baselines, fade and return |
| 1, Cancel | Restore draft experience from the selected wizard and all rows from opening baselines, fade and return |
| 25/26 | Decrease/increase row0 (Mana) |
| 27/28 | Decrease/increase row1 (Health) |
| 29/30 | Decrease/increase row2 (Control Limit) |
| 31/32,33/34,35/36 | Decrease/increase the three talisman rows |
| 2..24,37 and UINT_MAX | No edit/return branch; still refresh costs/UI |

Increment fields are receiver `+0xab/+0xbb/+0xcb`; all three talisman rows use
`+0xdb`. Six decrease/increase price pairs are `+0xeb/+0xef` through
`+0x113/+0x117`. Decreases require the resulting value to remain between its
opening baseline and maximum, then refund the original decrease price.
Increases require sufficient draft experience and resulting value within those
bounds, then deduct the original increase price. Rejected actions retain the
draft. Every callback requests original refresh `0x004b5f90` once, including
ignored actions. Only successful OK/Cancel requests fade and return `+0x43`.

Selected wizard fields are experience `+0x58`, stat-bar rows `+0x3c/+0x40/+0x44`
and talisman rows `+0xf0/+0xf4/+0xf8`. OK's experience check rejects negative
draft experience in ordinary bounded arithmetic but does not reject a value
above the opening wizard balance. A native adapter must validate its transaction
against the opening authoritative state, not rely on the callback to detect
forged grants. No experiment modifies a real wizard or campaign.

## Pricing and integration boundary

Original refresh invokes calculator `0x004b5d10` and updates displayed costs.
Static instructions divide the edited value by its configured increment, use
the resulting integer and configured coefficients through original x87 helper
`0x0059bcc0`, then convert through `0x0059bee0`. The exact formula, rounding,
failure/overflow behavior and replay/refund schedule require their own original
comparison; this investigation does not substitute the preview's sample prices.

The private action fixture uses deterministic supplied decrease/increase prices
7/11 and stubs wizard selection, fade and the price/UI refresh. It executes the
complete original callback and verifies its complete bytes remain unchanged.
It checks two wizard indices, all six edit rows, opening/max values and budgets,
OK/Cancel commit/rollback, ignored indices and negative/above-opening experience.
The test verifies the executable hash and immutable-original manifests before
and after execution. It does not prove disk persistence, input/control admission,
full ABI preservation, actual cost schedules or native/live integration.

[UI48 original execution](character-actions-original-20261012.json) passes329
cases:288 edits,10 OK,6 Cancel and25 ignored actions. The declaration and complete
source closure were checked before execution, stayed unchanged, and are retained
in the report alongside unchanged original callback bytes and both manifest
checks. Confidence is high within these selected branches; the supplied7/11
prices deliberately do not establish the real pricing/refund schedule.

```sh
python3 tools/working-retention.py run --reserve-gib 0.1 -- python3 tools/test-character-menu-contract.py --claims research/runtime/coverage/planned-character-actions-20261012.json
```

Fresh Celtic region1 disables Character in the original controls. Those controls
remain disabled. Next recover pricing and authoritative snapshots, then observe
Character from a genuine progressed Realm or Region caller and validate original
return ownership before enabling the native editor.
