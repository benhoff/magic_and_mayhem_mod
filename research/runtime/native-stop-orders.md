# Native scene stop and cancellation (NS15)

`NP.stop-orders` is an intentional native policy, separate from original input,
stop semantics, balance and live replacement. It extends the owned simulation and
production scene controls without original artifacts or build-specific pointers.

## Contract

**Cancel queued moves** synchronously removes only pending `move` operations for
the selected full slot/generation handle. Other actors, target/cleanup/release/stop
operations and their relative order remain intact. It returns the removed count,
advances no tick, and leaves the active route, animation/fine position and selection
unchanged. A repeated cancellation with no matching moves changes no bytes. The
world validates identity, eligible motion state, reentrancy and the resulting state
before committing. Failure leaves the world unchanged.

**Queue stop** adds native operation 5 with no target, destination or motion payload.
Existing queue/storage limits apply. It leaves active motion untouched until an
admitted Step. Pending commands apply FIFO before maintenance/decisions: a stop
following moves wins, while a later move may restart planning on the same tick.
A stop resets action to cancelled, clears route/progress, goal, fine motion,
previous-segment history and segment tick count, and anchors origin to the last
committed logical cell. It retains driver flags, search budget, entity identity,
cleaned flag and independent entity target. Fine movement therefore snaps back to
that logical cell rather than finishing or geometrically preserving a segment.
This is the existing native cancellation policy used on goal release/cleanup,
not evidence of original stop behavior. Subsequent stops remain valid.

Stale stop subjects, cleaned creatures and entities without motion are rejected
and counted at tick admission. Selection synchronizes before both actions; a
released/reused handle clears selection and never orders the replacement. Empty
selection disables both buttons. The QWidget emits semantic callbacks; session,
identity and command handling remain outside the widget. Application callbacks
refresh the controls and show either the queued-stop message or removed count.
Target fields and selection persist on ordinary refresh. Cancellation does not
remove queued stops; the separate button labels and tooltips explain this.

## Checkpoints

A pending stop selects native snapshot **v7**. Its layout is the v6 superset,
including optional animation binding and terrain-policy fields, with operation 5
newly admitted. Navigation binding is required. Decoders reject operation 5 under
versions 1–6, rather than silently expanding their contracts. Existing checkpoints
continue decoding, and a state without pending stops retains the prior minimum
version (1–6); applying/cancelling moves does not force a permanent migration.
Selection/control state is transient. Save preserves queued stops and cancelled
pending queues. See [format](../formats/native-world-snapshot.md).

## Validation and limitations

`tools/test-stop-orders.py BUILD/world-stop-test NEW_OUTPUT` uses owned synthetic
multi-creature navigation and the production QWidget/controller/world. It verifies
selected-only stable queue filtering, pending cancellation while motion stays
active, real button callbacks, no implicit tick, inadmissible tick preservation,
partial fine-segment stop/reset, FIFO restart, repeated stop, stale-generation and
cleanup refusal, malformed payloads, command budget, tick rollback and forbidden
in-tick queue mutation. A pending v7 stop resumes in a fresh process to exactly the
uninterrupted checkpoint bytes; changing its header to v6 is refused. Existing
motion/checkpoint, mouse-picking and composition tests run as regressions. Both
component runs, all 107 normal CTests and ten focused sanitizer suites pass.
Original manifest verification brackets the regressions. Reports and logs are
pinned in [evidence](native-stop-orders.json).

[Committed-history review](native-stop-orders-history-review.json) checks exact
receipts for versions since the preceding milestone; it asserts neither a past gate
pass nor runtime validation. Historical evidence retains its original fingerprints;
affected older evidence can be stale, with explicit pending reviews. Only the new
native stop policy gains current validation. Original input/stop equivalence,
installed whole-window interaction, group/faction/playback policies,
commander/summoned gameplay and live replacement remain pending.
