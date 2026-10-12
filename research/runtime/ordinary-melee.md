# Ordinary melee exchange (NS24)

No-CD SHA-256 `40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Preferred VAs are build-specific, never host pointers. High confidence applies
only to the executed projection below. The original still owns the campaign.

The independent model in `reconstruction/combat/melee.*` covers type17 against
type17, distinct hostile owners, no evasion (`+7c`), no charge (`+16c`), no cached
contribution chains or attached secondary animation. Configuration HTH at
`+1a0+targetType*4`, stun at `+210+targetType*4`, and caller-supplied response
classification/percentage enter the projection. Full reach, facing/sequence
selection, AI, diplomacy, stun state/effects, audio/XP/effect admission internals,
other creature types and live replacement remain pending.

## Recovered arithmetic and event ordering

`50c0d0` divides signed HTH by3 when only the target is physical, adds the
signed low32(power*base)/100 adjustment, doubles for Blood Lust, and adds
signed low32(modifier*base)/256. Modifier includes signed height bonus, supplied
response-table value and target response flags' +50. Negative adjusted base
becomes zero. It ALWAYS draws from unchanged `54e200`, masks the sign bit,
takes modulo257 and signed low32(draw*base)/128. A nonpositive result returns
before damage, stun or attack sound. Positive pre-rank results get signed rank
bonus/100 and the attacker's quarter flag multiplies by4, with DWORD wrapping.
The resulting request passes to unchanged defended health (`514860`).

For distinct hostile owners, the credit endpoint then receives the requested
amount. A second draw modulo100 selects the stun branch with an inclusive
`draw <= configuredThreshold` check. The supplied `521160` result selects
`521420`, or a third draw modulo20 plus20 passed to `50d060`. Attack sound
follows. Optional effect admission is a null-return boundary when a status
requests it. The ordinary cleanup/bar/attribution chain uses the NS23 health
model; pair-present contribution lookup with IDs -1/-1 is now explicit, and
lookup returns can be null. The lethal detach argument is the target slot index, read from creature+0;
this lab uses slot1 rather than the earlier isolated health fixture slot0. Contribution IDs do not establish experience semantics.

Main action `50e7a0` ticks the original controller `464ec0`. Event2 resolves
the target, clears engagement and decrements/clamps incoming engagement, then
calls melee unless `+855` blocks it. A surviving target records source slot+1.
Event1 requests action15 and consumes a SEPARATE modulo3 draw, setting cooldown
2..4 (+30 when `+77c` is nonzero). Event3 invokes an audio boundary. Event4
creature-specific audio is excluded. Missing target event2 returns safely;
engaged event1 with an invalid target is outside the safe original domain.

The shipped Crocodile ANI has one event2 in each sequence24..31. The complete
forward cursor (PC, displayed record, active, delay, elapsed, repeats and break)
is compared against unchanged `464cb0/464ec0`, including the repeat tail.
Animation does not hit when the order is issued or when the first frame starts.

## Native integration boundary

`apps/melee-lab` loads encrypted creature/HTH CFG through native asset decoders
and the Crocodile ANI. It selects facing0/sequence24 explicitly, maximum health
from CREATURE_17 and HTH/stun values for target17. The shipped values are
360 HP, HTH18 and stun threshold9. Original sequence/reach selection is pending.

`game/simulation/melee_exchange.*` owns a two-actor stationary native encounter,
slot order, retained dead identities, terminal stop, and a once-per-tick recovery
countdown/rearm policy. This does NOT reproduce original ready/behavior callback
reentrancy (`50f7c0`), footprint placement or whole-fight campaign equivalence.
The adapter supplies neutral response and a handled-stun endpoint; stun effects
are not applied. Native ordinary numerical health and independently owned
cleanup are used. This lab does not change authored skirmish balance.

The backend is immutable; animation and all250 RNG words/both cursors live in
owned checkpoint bytes. The profile includes all three raw source assets and a
version tag; SHA256-derived identity and health-rules identity bind continuation.
Fresh processes resume with no installation, original executable, Qt window or
Wine. The Qt window is a presenter around a lifecycle controller, with Play,
Pause, Step, Restart, Save and Load. Its50ms timer is presentation policy, not a
recovered original tick frequency. Vector figures are authored, not game sprites.

## Reproduction and evidence

Use `tools/working-retention.py run --reserve-gib 2 --` for builds and the
comparison runner. `tools/test-ordinary-melee.py` verifies the original manifest
before/after, pins the immutable PE hash, preserves target bodies, redirects only
explicit helper entries in private memory, checks source/config write boundaries
and every RNG word/cursor, and compares normal/i386/ASan+UBSan results. Initial
negative runs under `working/tests/melee-ns24/` are preserved. Final reports and
coverage declarations are registered separately; historical health/skirmish
results retain their original hashes.

The pinned Ghidra inventory has no discovered function containing `50e7a0`.
An explicit recovered-range exception `50e7a0..50eb24` binds the unchanged
executed body and entry anchor. The inventory/unassigned ranges are preserved;
this comparison covers the stated event branches, not the entire dispatch table.

Final isolated comparison:8,448 formula cases,4,368 main-action/composed-exchange
cases,18,432 animation ticks and262,144 RNG words match, with ASan/UBSan passing.
The independent health follow-up matches8,622 cases/36,895 callback events.
Native normal integration preserves49 fresh-process continuations and681
one-tick snapshot roundtrips, including strict refusal/rollback checks. The
production window timer and actual Play/Pause/Step buttons preserve the same
final checkpoint as headless execution. New immutable report IDs are NS24.*;
negative/development runs remain under `working/tests/melee-ns24/`.

The sanitized native exchange independently passes the same49 process resumes,
strict refusals/rollback and full outcome under ASan/UBSan/LeakSanitizer. Qt
window sanitizer execution remains pending. The sandbox ptrace restriction
prevented the initial LeakSanitizer attempt; the final bounded native run used
normal host thread inspection and passed.
