# Campaign selection and order admission — 2026-11-03

This batch separates original selection, command-buffer submission and later
simulation effects. It uses the unchanged original checkpoint
`checkpoint-camulos-05.sav` (SHA-256
`9ec512d4c05a75c20e7cc7befec8398e112c2ca700917f34e93f5501099ee7c1`).
It does not establish nominal progression or rendering equivalence.

## Static contract

The NoCD build is `40209ca7…b7168`. Inspection in
`campaign-order-admission-results-20261103/static-input-contract-01.json`
confirms the bounded instructions at `0x537bd0`: manager `+0x24` is a table,
`+0x28` its capacity, `+0x30` its cursor and `+0x34` its enabled flag. The
chosen node index is `(cursor ? cursor : capacity) - 1`, with stride 21.
Node fields are buffer pointer `+0`, capacity `+4`, write pointer `+8` and
used bytes `+12`. Submission copies the incoming record's length at `+8`,
advances the write pointer and used bytes by that length, and increments
the buffer header's command count at `+16`. Confidence: high within these
instructions; producer rotation, lifetime and delivery are separate contracts.

Existing selection research remains scoped to raw creature fields `+0x68`
and `+0x70`. Spell-bar receiver identity is not creature selection. A combat
target field is not a command-buffer acknowledgement.

## Passive observer

`runtime/menu/campaign_order_observe.h` is opt-in through
`MNM_MENU_ORDER_ADMISSION_OBSERVE`. It reuses the existing guarded Region
callback and World-return observer. It installs no producer hook and issues
no simulation commands. Region observation preserves the existing FPU and
LastError forwarding. The original callback executes exactly once.

`MNMCOA01` version 1 has a 16-byte header and 256-byte records, at most 8192.
Kind 1 is sampled at most every 250 ms; kinds 2/3 bracket each actual Region
callback with an ordinal. A double-collected pool snapshot records primary
and secondary selection bitmaps. These checks do not establish allocation
generations or atomicity against every possible writer.

A callback return records up to 96 appended bytes only when both snapshots
refer to the same node and buffer, the write/used increments agree, command
counts are monotonic and length-delimited records consume exactly those
bytes. Buffer changes, insufficient readability or unmatched lengths cannot
be used as submission proof. Overflow, foreign thread, oversized appends,
inconsistent counters and malformed records are refused by the strict reader.
An empty valid append means no witnessed submission. Submission does not prove
delivery, engagement, movement completion or a successful battle.

## Bounded scenario

After staging the verified checkpoint and preparing current sources:

```sh
python3 tools/run-rendering-progression.py \
  --case victory --software --size 640x480 --skip-intros \
  --checkpoint restore --checkpoint-name checkpoint-camulos-05 \
  --checkpoint-reference research/runtime/rendering-progression-results-20261101/checkpoint-seeded-software-low-05.json \
  --checkpoint-probe orders --deadline-seconds 180 \
  --preparation <new-preparation-reference> --report <new-progression-reference>
```

Run this under the storage guard. The default diagnostic capture policy
retains no viewport, command-history or World-source pixel captures. The
physical helper selects/centres the wizard, presses X, checks the sole living
owned Zombie's selection bitmap, samples five real pointer motions and
requires the existing original-callback/later-World hover gate before one
ordinary right-click move. Opcode 5 submission and subsequent original
movement are independent assertions. A negative probe remains negative while
the Qt owner attempts normal Quit and resource retirement.

## Results and remaining boundaries

All three `order-admission-probe-01.json` through `order-admission-probe-03.json` in
`rendering-progression-results-20261101/` passed original checkpoint restore,
normal Quit, stopped Wine, unchanged saved bytes and complete artifact
retention. The first two are historical after the final hover source fix. The third
matches the current player, compiler closure and observer/driver sources.

In all three runs, X left primary selection `[3]`, with one living owned Zombie.
The original callback appended a 21-byte opcode 5 record for slot 3, destination
`(26,59,4)`, mode/extra zero. Its buffer changed from 20 to 41 used bytes and
from zero to one command. Independent gameplay samples then showed the Zombie
move from `(24,57,5)` to `(25,58,4)`, still alive with 89 HP. This is subsequent
movement, not completed arrival. All ten callback pairs in each stopped run
bind to the independent existing Region envelope by ordinal, mask, thread and
tick ordering. See
[the binding](campaign-order-admission-results-20261103/observation-binding-02.json).

The tested five real motions, including a one-pixel displacement, reached the
original callback. This does not reproduce or explain the earlier stalled
`(470,230)` callback. It does rule out treating that earlier failure as a
universally applicable small-motion threshold.

Review found that `fresh_hover` returned the first matching World row before
examining newer callback/World records in the same prefix. It now examines the
whole prefix and cancels a match on a newer callback or unready World sample.
The four new prefix/pending/nested-callback regression cases fail against the
exact retained previous function and pass the current function. Two of them also
fail the exact intermediate source from the second probe; that source was
reconstructed and verified against its original execution hash. A callback
entry cancels prior readiness, including while nested callbacks remain pending.
Only a subsequent complete return and later World update can restore it.
The final repeat's hover witness is sequence 26 at `(320,310)`, after the last tested motion; the earlier run
accepted sequence 10 at `(321,310)` under the one-pixel tolerance. The tolerance
and original-callback/later-World requirement remain unchanged.

Current focused validation passed 156 Python cases and seven Qt driver cases.
No viewport, rendering-command-history or World-source pixel captures were
requested. Six stopped execution trees passed two-copy custody and exact real
restoration before retirement to pinned stubs; see
[the first four](campaign-order-admission-results-20261103/execution-custody-01.json)
and [the final two](campaign-order-admission-results-20261103/execution-custody-02.json).

Attack command admission and delivery, actual support engagement, mana/rearm
handling under combat, live route suspension, fresh nominal victory through
Realm/region 2, independent rendering comparison and the twelve-case matrix
remain separate pending steps. The old three-second combat-target-field
assertion has not been relaxed or promoted into an order-submission claim.
