# Campaign transition validation — 2026-11-01

The bounded preparation is ready: selected Celtic map/configuration bytes and an
unchanged save control are hash-pinned; fresh-campaign victory, defeat and
region-2 progression drivers have strict natural-report, return and cleanup
assertions. [Current preparation](rendering-progression-results-20261101/validation-inputs-13.json)
and [105 controls](rendering-progression-results-20261101/validation-guards-11.json)
pass. The save is not a verified near-outcome fixture and never supplies a
progression result.

Live validation is partial. One default 800×600 natural defeat and its complete
independent pixel comparison pass at their retained source phase. An earlier
diagnostic reaches original victory but fails Realm return. Current complete
victory/Realm/region-2 entry, the 640×480/software matrix and populated lifetime
closure remain pending. A nominal victory run also exposes an unresolved native
CPU-history refusal. None of these boundaries is waived by preparation success.

This owned batch starts from master `08f813e1d` and integrates campaign commits
`4e4a3d416` / `460906605`, including the existing capture-worker repair
`d0bf7a551`. Both coverage journals and ledger additions are preserved. Earlier
campaign failures and source hashes remain historical; merging them does not
renew their validation. Runtime outcome, pixel comparison and populated lifetime
proof remain independent.

[Incoming committed-history review](rendering-progression-results-20261101/validation-committed-history-01.json)
checks the five non-merge commits from common base `65095aee6` to `08f813e1d`
and `460906605`. All watched parent/current file hashes match exact receipts
in their own committed journals; 6,076 committed file changes are enumerated,
with no unresolved watched versions. Current uncommitted changes are a separate
review. This accounting does not assert a past gate pass or fresh validation.

## Lossless long-session capture

[Version 2](../formats/campaign-viewport-capture-v2.md) is an opt-in viewport
diagnostic encoding. It preserves every paint/swap, using complete RGBA keys and
XOR deltas against the preceding owned paint. Every 128th paint is a full key;
dimension changes also require a full plane. Reconstructed hashes, base indices,
dimensions and bounded policy fields are checked before admission. The worker
retains its 256 MiB/256-job queue; the viewport storage ceiling remains 1 GiB.
The campaign decoded-byte ceiling is 64 GiB with 131,072 rows. Exceeding any
budget still produces an incomplete proof. No original pacing, simulation,
rendering output, health or mana is changed by the codec.

The Python reader retains compressed payloads and metadata, then decodes planes
on demand for the existing independent comparator. Decoded image storage uses a
fixed number of plane buffers rather than retaining a whole mission's RGBA
planes. Backward access replays from a full key at most 127 paints away.

[Codec controls](rendering-progression-results-20261101/validation-codec-controls-01.json)
pass optimized and ASan/UBSan builds, ownership/FIFO/blocked-compressor controls,
bounded queue refusal, compression failure and first-error retention. The long
control preserves all 2,300 source snapshots: 9,646,899,200 decoded bytes in
118,112,433 stored bytes, with 18 keys and 2,282 deltas. The independent fixture
decoder's cumulative digest matches the source. The production Python reader
validates every paint, retains lazy plane handles and reproduces the final source
hash. The sanitized case separately preserves 192 snapshots / 805,306,368 decoded
bytes in 11,936,904 stored bytes. These are diagnostic transport controls, not
original-game equivalence or a general workload compression ratio.

[Current source/input pins](rendering-progression-results-20261101/validation-inputs-01.json)
retain the canonical No-CD build, selected maps, unchanged save control and real
launcher manifest. [Driver guards](rendering-progression-results-20261101/validation-guards-01.json)
pass 5 capture, 9 progression, 10 pilot and 14 normal Python checks plus five
actual Qt control cases (43 total), with Release player/fixture links. Independent
history and 32-bit original-reference binaries are built in the owned checkout.
No original process is executed by these tooling checks.

The collector retains source corpora up to the producer's existing 16 GiB stored
bound, replacing its former 2 GiB import ceiling. Successful acquisition now also
requires complete continuous World/source capture and closed, valid viewport/
command streams before separate independent comparison. Semantic result/return
facts remain in the flow if transport admission refuses the acquisition.

## Calibrated physical-input candidates

[Named waypoint calibration](rendering-progression-results-20261101/validation-waypoint-calibration-01.json)
decodes the pinned installed EVT records and the unrotated fixed Celtic region 1
recipe. Three retained original spawn anchors independently agree on 20-cell
section translation: the opening Redcap, bridge Redcap and Camulos deployment.
This yields `CF52 PoP = (23,57,6)`, `CF52 Bridge = (31,52,4)` and
`CF54 PoP = (43,27,5)`. The first point is a candidate mana-recovery waypoint;
its capture/recovery and path reachability require fresh live evidence. This
calibration does not establish a complete assembled map, RNG choices or general
rotation/trigger semantics.

The new physical geometry helper separates clamped ground navigation from exact
visible-target projection. Clamping an aim can select different ground or a
different creature, so offscreen/occluded aims are refused. Native-frame HUD
anchoring and 640×480 coordinates have pure policy controls and still need live
confirmation. Opening anchors are checked by role/observed position, with no
cross-launch slot-number assumption. These are input-driver policies, not new
creature orders, recovered AI rules or gameplay balance changes.

## Source-history transport

[V4 source capture](../formats/campaign-source-capture-v4.md) preserves every
owned source/request byte with a previous-record prefix, periodic independent
keys, exact ordinals and reconstructed SHA-256. The existing record, queue and
16 GiB accounting limits remain. Legacy formats keep their existing encoding.
[Optimized and sanitizer controls](rendering-progression-results-20261101/validation-source-controls-03.json)
each preserve 317 records / 323,511,217 body bytes in 12,420,093 stored bytes;
independent C++ and Python decoding agrees throughout. Malformed and legacy
controls pass. The prior level-1 compression guard failure is retained as
`validation-source-controls-02.json`, with exact codec/producer preimages. It
decoded correctly but missed the synthetic storage-ratio requirement. A prior
Qt include-order build refusal is also retained. None establishes original
simulation or pixel equivalence.

## Original report callback boundary

The [first dedicated nominal defeat](rendering-progression-results-20261101/validation-defeat-default-high-01.json)
played both enabled intros, entered the actual 800×600 World and reached natural
player death. The original report contained all 21 strings, including "You were
killed." Original OK and fresh parent return ACKs were observed. Its automatic
wrapper still failed: the helper incorrectly waited for a Report-active World
pop, which does not precede original OK. The session returned to Main but held
cleanup waiting for the helper. A separately recorded manual public Main Quit
retired the original normally; this is not automatic scenario acceptance.

[Exact retained callback diagnosis](rendering-progression-results-20261101/validation-report-boundary-diagnosis-01.json)
identifies report callback begin 25 and return 26, on the original owner thread.
World pop 31 occurs afterward, with Realm active. The helper now stops on the
original report OK dispatch; the comparator separately requires its return,
natural outcome, fresh parent return and final Main Quit. A real helper-process
regression supplies callback begin without World pop/return/gameplay and checks
that the helper exits successfully without claiming semantic acceptance.

The failed run retains 1,880 World records and 25 source chunks. Its V3 source
capture is complete/untruncated: 4,137,182,061 accounted raw bytes and
2,233,937,250 stored bytes. No independent pixel comparison is claimed for this
failed automatic acquisition. Historical acquisition/source hashes are unchanged.

[New prepared pins](rendering-progression-results-20261101/validation-inputs-02.json)
and [52 current guards](rendering-progression-results-20261101/validation-guards-02.json)
bind the callback correction and V4 source codec. The independently reviewed
clipped-distortion oracle correction is copied exactly from master `e5fc9a0eb`;
its skipped-top-row phase adjustment changes only the oracle setup, not native
rendering.

The [fresh dedicated default 800×600 defeat](rendering-progression-results-20261101/validation-defeat-default-high-02.json)
passes automatic acquisition in 445,463 ms. Original natural defeat has outcome
0 and the Forest title; original OK ACK 5 is followed by Main return ACK 6 and
automatic Main Quit. The helper observes report dispatch and retires without
manual input. The original process exits 0, retirement is complete and all final
resource counts are zero. Source/compiler closure and player binary remain
stable, and both original manifest scans pass.

All 1,820 accepted Worlds are written, complete and untruncated. V4 retains
3,920,724,742 accounted raw bytes in 200,866,229 stored bytes, with a 3,123,592-byte
peak queue. Closed viewport V2 has 11,586 rows and 10,518,486,000 decoded bytes
in 283,645,365 stored bytes; every decoded plane hash passes. Commands V1 has
6,399 rows and 119,215,024 decoded bytes. The acquisition retains 290 artifacts.
This actual viewport history exceeds the former 8 GiB decoded limit.

[Independent comparison](rendering-progression-results-20261101/validation-defeat-default-high-02-comparison.json)
passes: 1,820 original Worlds / 2,055,857 draws reconstruct continuously from
owned inputs and original raster results. All 3,007 captured native presentations,
including 2,949 World swaps, match 2,629,621,500 pixels with zero differences.
All command surfaces/palettes retire, source/input hashes stay stable and both
original manifest checks pass. Native player pixels and captured original
destinations never seed this oracle. No GPU-to-CPU read operation occurs in this
case, so none is claimed. Movie planes without native leases, hidden Qt widgets
and physical scanout remain outside this comparison.

## Next ordinary-input phase

The following input-only source phase preserves the successful acquisition and
comparison hashes above. Their exact execution phase remains historical when
the shared driver/build files change; no old hash is refreshed. Transport
implementation and codec-control sources remain unchanged. Input preparation
03 and its two subsequent source preimages are retained before the explicit
observer option; preparation 04 pins the final option and corrected policy.

The policy first checks two original opening spawn anchors, approaches the
calibrated Place of Power, summons through ordinary input and requires an
original caster mana debit plus living owned Zombie. It then requires observed
mana recovery before approaching the bridge and Camulos. One nearby combat
support is preferred over repeatedly spending mana on distant summons. Ground
orders are bounded; visible body aim remains exact. Fireball effects on Camulos
are recorded separately from other recipients. These are candidate input
policies; successful power occupation/recovery and victory still need live proof.

`--observe-input-admission` explicitly enables the existing passive original
mouse/World prefix and marks acquisition diagnostic. Before a right press, that
mode requires original motion dispatch and a later World hover; cast mode must
also be ready. An unready order sends no button and is recorded as refused,
allowing the main loop to inspect reports or dismiss a stalled conversation
before reselecting. The existing prefix bound is unchanged. A diagnostic result
cannot silently become nominal acceptance. Without the flag, physical inputs
retain fixed-delay candidate delivery, with original cast/mana/outcome checks.

Victory, later-map entry and the other mode/provider executions remain pending;
final zero owners do not establish populated acquire/release lifetime closure.

The [first optional-hover progression diagnostic](rendering-progression-results-20261101/validation-progression-hover-high-01.json)
verifies both opening anchors but stops at the movement-refusal budget. The new
guard incorrectly requires mode 0 after fresh original hover. It records 21
selected-wizard ground hovers with mode 1 and four unready-hover timeouts; all
25 orders have `button_sent=false`, and there are zero physical right presses.
Thus this is a driver-guard failure, not a failed original movement command,
power capture or mission outcome. Source/compiler/binary stability, complete
artifact retention, private Wine stop and original manifest scans pass. Its
exact helper preimage and diagnostic prefix remain retained. A corrected live
probe must establish actual right-input movement and subsequent boundaries.

## Power recovery and bounded combat continuation

The [second optional-hover probe](rendering-progression-results-20261101/validation-progression-hover-high-02.json)
confirms selected-wizard ground mode 1, physical movement to `(23,57,6)`, an
original Zombie cast costing 17 mana and a new living owned Zombie. Its 60-second
refill deadline expires at 49/50 mana. The [boundary diagnosis](rendering-progression-results-20261101/validation-power-refill-boundary-01.json)
preserves that negative result; the policy window becomes 90 seconds.

The [third probe](rendering-progression-results-20261101/validation-progression-hover-high-03.json)
then observes full recovery after 77,636 original tick milliseconds. It reaches
Camulos, whose health falls to 20 through owned-Zombie and player Fireball damage,
but the wizard dies first. The [retained tactical diagnosis](rendering-progression-results-20261101/validation-camulos-tactics-01.json)
contains the original cast/debit and defended-damage rows, including one player
Fireball hitting an owned Zombie. Bridge combat and a boss-area summon consume
mana; repeated camera/spell selection also spaces player casts widely. This is
negative victory evidence, with complete artifact retention and clean private
Wine retirement, rather than progression or a separate accepted defeat case.

The next [pinned policy](rendering-progression-results-20261101/validation-inputs-08.json)
uses an authored bridge-bypass candidate `(35,47,4)`, prioritizes Camulos, keeps
combat mana for Fireballs, and sends bursts of at most five shots. Every repeated
shot requires an original accepted cast/mana debit before the next; a missing
acceptance returns to the bounded reconciliation path. An owned living ally in
the firing segment refuses the cast and requests a flank. Low mana requests
retreat. No original simulation values, scripts or spell parameters change.
[93 controls](rendering-progression-results-20261101/validation-guards-06.json)
pass, including firing-line geometry controls and the five Qt public-control
fixtures. These controls alone establish no original mission completion.

Shared evidence-writer contention required another documented clean checkpoint
of the background archive sweep after its current case. The previous run's final
acquisition and exact preimage were retained before changing the driver. The
checkpoint is temporary; the unchanged sweep must be resumed after the bounded
collection. Generated reports remain SQLite-backed references.

## First natural victory and Realm tutorial boundary

[Diagnostic 04](rendering-progression-results-20261101/validation-progression-hover-high-04.json)
reaches the original `Victory!  ( The Forest of Pain )` report. Its 21 strings
include the original 10 points for talking to Camulos, 40 for defeating him,
and 50 total. Original report OK callback begin/return and subsequent World pop
to Realm are retained. The complete progression case still fails its 780-second
deadline at stage 5; it has no region 2, normal Quit or independent comparison.
No simulation-memory, health, mana, experience or campaign-progress writes manufacture that
intermediate victory. The original script owns Camulos's retreat and portal.

[Retained diagnosis](rendering-progression-results-20261101/validation-realm-tutorial-01.json)
links the exact callback/pop rows, final channel and recorded native frame. The
frame displays the original "The Realm Map" tutorial. Settled Realm admission
correctly remains screen 0 / not ready, with campaign handoff 3. The adapter
already excludes tutorial overlays; those guards are unchanged. The earlier
camera-basis suspicion did not justify a projection change: the run subsequently
closed casting range, defeated Camulos and entered the original report flow.

The [next pinned driver](rendering-progression-results-20261101/validation-inputs-09.json)
waits for original report return and World pop to the exact Realm owner/depth,
then permits at most 12 ordinary tutorial clicks in the unsupported campaign
return window. Owner/status, acknowledged request, even/stable wire sequence and
handoff are checked. Only settled original Realm readiness completes the helper;
the Qt driver then selects region 2 after the helper exits. This is a bounded
authored input policy rather than a recovered tutorial-state API. Unknown states
wait or refuse. [98 controls](rendering-progression-results-20261101/validation-guards-07.json)
pass, including a real isolated helper process which clicks the authored tutorial
fixture and stays alive until the fixture publishes settled Realm readiness.
Fresh original execution remains necessary for the complete transition.

## Original returned-pointer callback omitted by the readiness policy

[Diagnostic 05](rendering-progression-results-20261101/validation-progression-hover-high-05.json)
dies before the tutorial step, after only two accepted Fireballs with mana still
available. [Its exact diagnosis](rendering-progression-results-20261101/validation-motion16-01.json)
finds ten no-click readiness refusals. One retained original callback return has
mask `0x10`, no button flags and the requested `(624,230)` position. A strictly
later World sample is cast-ready at that same position with the selected Fireball
and validated player receiver. The old `mask & 0x60` predicate excludes the
callback and waits five seconds without sending a button. This is a driver
policy omission; it does not establish a general original event-mask taxonomy.

The new `0x70` predicate includes that observed return while retaining exact
position, no-button, active input, strict later-World, selected-spell/receiver and
ground-bounds checks. [Replay from immutable captured rows](rendering-progression-results-20261101/validation-motion16-replay-01.json)
admits sample 1239, and refuses sample 1238 whose World counter still equals the
callback's counter. [99 controls](rendering-progression-results-20261101/validation-guards-08.json)
pass against [new source/input pins](rendering-progression-results-20261101/validation-inputs-10.json),
including original no-button `0x10` with equal/later Worlds and held/changed-button
refusals. Live victory, tutorial dismissal and full progression require another
fresh run; no former negative result or source hash is rewritten.

## Re-run the bounded scenarios

The owned validation checkout has a physical canonical `working/game-nocd`, the
current prepared map/save/source pins, a linked Release Qt build and independent
history/original-reference binaries. Existing report names are immutable; choose
a new name for each execution. From that checkout, a nominal fresh campaign run
(no skipped intros or extra input observer) is:

```sh
python3 tools/working-retention.py --root "$PWD" run --reserve-gib 6 -- \
  python3 tools/run-rendering-progression.py --evidence-mode full \
  --case progression --size 800x600 \
  --preparation research/runtime/rendering-progression-results-20261101/validation-inputs-13.json \
  --report research/runtime/rendering-progression-results-20261101/user-progression-default-high-01.json
```

Use `--case victory` or `--case defeat` and a distinct report name for the other
bounded scenarios. `--size 640x480` and `--software` select the other frame/backend
variants. An unchanged retained save is a reload/control input only; these
primary cases start fresh and never count a reload as progression. An optional
held `--prefix-template` may reduce staging while retaining an exclusive lease.
The default path creates an isolated prefix.

After a successful acquisition, keep its source/build phase unchanged and run:

```sh
python3 tools/working-retention.py --root "$PWD" run --reserve-gib 6 -- \
  python3 tools/check-rendering-progression.py \
  --acquisition research/runtime/rendering-progression-results-20261101/user-progression-default-high-01.json \
  --report research/runtime/rendering-progression-results-20261101/user-progression-default-high-01-comparison.json
```

Source drift refuses the old preparation: rerun `prepare-rendering-progression.py`
and `review-rendering-progression.py` under new report names after any driver or
engine edit. Acquisition, original pixel/history comparison and populated
object/vptr/generation lifetime proof are independent. A helper success, static
guard pass or final zero owners does not establish the remaining milestones.

## Named bridge and documented keyboard controls

[Diagnostic 06](rendering-progression-results-20261101/validation-progression-hover-high-06.json)
reaches full mana recovery, then remains in the bridge-bypass phase until its
540-second helper deadline. The wizard stays alive with 196 health; no combat,
tutorial or progression result follows. [Exact route diagnosis](rendering-progression-results-20261101/validation-bridge-loop-01.json)
retains repeated movement/order samples and the captured river/bridge frame.
The authored direct `(35,47)` bypass is not reliable. This does not establish an
original pathfinding fault. Portrait appearance alone also did not establish the
earlier suspected selection/camera mismatch; that hypothesis is withdrawn.

The [pinned original game manual](rendering-progression-results-20261101/validation-manual-controls-02.json)
provides candidate controls on printed page 15: W selects the wizard and a second
W centers the view; Return advances a conversation. The next driver uses those
ordinary keys and restores the original named `CF52 Bridge = (31,52,4)` waypoint,
which had been reached in earlier probes. It retains all cast/debit, exact-aim,
allied firing-line, original report and bounded tutorial checks. It changes no
original pathfinding, script or simulation values. Build-specific control
consumption and the complete progression route require fresh execution.

[99 progression controls](rendering-progression-results-20261101/validation-guards-09.json)
pass against [source/input pins 11](rendering-progression-results-20261101/validation-inputs-11.json).
[Separate production Qt controls](rendering-progression-results-20261101/validation-key-input-controls-01.json)
exercise W and Return key-down/up, correct Windows key state and exact synthetic
XCB delivery at normal and 1.5 DPI, alongside existing focus/repeat/drag/retirement
checks. These execute fake windows, not original game keyboard handlers. The
manual source bytes are retained in SQLite with their SHA-256 and source URL.

## Actor-state quiet during the opening Hermes conversation

[Diagnostic 07](rendering-progression-results-20261101/validation-progression-hover-high-07.json)
stops before power capture, exhausting 25 no-click readiness refusals. The
[recorded diagnosis](rendering-progression-results-20261101/validation-opening-dialogue-01.json)
contains the native frame showing Hermes's opening conversation, repeated
unchanged wizard state and advancing sampler sequence numbers. The diagnostic
field named `world_counter` remains 101 in that window; this does not recover a
general original simulation clock. The old detector interprets every new sampler
row as gameplay change and therefore never requests conversation advance.

The isolated `campaign_dialogue.py` policy compares actual actor fields and
ignores sample sequence numbers. Six seconds of unchanged nonempty state permits
one ordinary Return attempt; actual actor/role/set changes restart the window.
A global 48-attempt budget, finite/monotonic clock and empty-state controls bound
the policy. A hint is not an original dialogue-state API. The original handles
the key, and all natural report, fresh Realm, later-map and Quit requirements
remain unchanged. The record parser itself is unchanged.

[Immutable-row replay](rendering-progression-results-20261101/validation-quiet-replay-01.json)
produces the hint at 6.812 original tick seconds in the visually confirmed Hermes
window. It also verifies the old diagnosis/preimage still matches source pins 11,
despite a temporary shared-writer wait during another batch's commit closeout.
[105 progression controls](rendering-progression-results-20261101/validation-guards-10.json)
and [two production Qt input/DPI controls](rendering-progression-results-20261101/validation-key-input-controls-02.json)
pass against [source/input pins 12](rendering-progression-results-20261101/validation-inputs-12.json).
Fresh original Return consumption and complete progression remain pending.

## Optional callback contract and nominal native-history refusal

[Diagnostic 08](rendering-progression-results-20261101/validation-progression-hover-high-08.json)
advances beyond the first frozen actor state after Return attempts. It later
exhausts the optional callback guard. [Exact retained analysis](rendering-progression-results-20261101/validation-hover-contract-01.json)
shows advancing World samples with the requested Qt position and original pick
fields, but no new matching Region callback. The guard withholds buttons under
its stated contract. Per-motion callback semantics remain unestablished; this
diagnostic admission limitation is not an asserted original-engine input fault.
Nominal scenarios do not enable that extra observer/predicate.

The [first nominal victory attempt](rendering-progression-results-20261101/validation-victory-default-high-01.json)
plays enabled intros, enters the actual default 800×600 World and makes the
original power Zombie cast/debit. During refill it stops on native rendering.
[The exact refusal](rendering-progression-results-20261101/validation-native-history-refusal-01.json)
is the version 2 native CPU-history authority record at generation 466, with
family/route 0 and original process exit 124. The subsequent command interruption
is not a scenario deadline or natural defeat. The process-local caller remains
unnormalized; no loaded-module base or cross-launch pointer identity is assumed.
The exact reader/admission/lease cause remains unresolved. No authority guard is
weakened, and no nominal victory/progression or independent comparison is claimed.

## Observed recovery allowance

[A separate no-hover, skip-intro diagnostic](rendering-progression-results-20261101/validation-progression-nohover-high-01.json)
reaches the same power candidate with exactly one accepted 17-mana player cast.
Mana increases monotonically from 33 to 46 before the 90-second driver refill
allowance expires. [The retained boundary](rendering-progression-results-20261101/validation-refill90-extended-01.json)
contains the original cast rows and first/last recovery samples; extra player
spending is absent. These are position, summon/debit and recovery observations,
not proof of PoP ownership or a recovered regeneration-rate formula.

The source policy now allows at most 180 seconds for actual recovery of the
observed debit. The 540-second helper and 780-second scenario bounds remain,
with no simulation pacing, health, mana or balance changes. [Source/input pins 13](rendering-progression-results-20261101/validation-inputs-13.json)
and [105 current controls](rendering-progression-results-20261101/validation-guards-11.json)
pass. The earlier two Qt key/DPI control receipts retain their exact source-phase
fingerprints; the input C++ implementation is unchanged. Fresh complete original
progression and the native-history barrier remain separate work.

For explicit transition isolation, `--skip-intros` produces a diagnostic receipt
that is ineligible for nominal acceptance. `--observe-input-admission` also marks
its run diagnostic and retains the extra callback contract described above.
Neither flag bypasses natural outcome, report/return, later-map or Quit checks.

## Current bounded execution result

The [source-13 no-hover diagnostic](rendering-progression-results-20261101/validation-progression-nohover-high-02.json)
finishes with an original natural defeat at 322,994 ms; the required victory
discriminator refuses the scenario. The collector retains 290 artifacts,
unchanged source/compiler/binary closure, both original manifest checks and
stopped private Wine. Its enabled skip-intro flag remains diagnostic.

[Exact combat evidence](rendering-progression-results-20261101/validation-refill180-combat-01.json)
confirms the original power summon/debit and full 50-mana recovery, followed by
bridge/combat entry. Eleven player casts have original acceptance records. One
later Fireball damages Camulos from 100 to 80 HP before the player dies; another
enemy receives two earlier hits. Accepted target records alone do not prove
impact. Remaining combat/impact causes are unresolved, with no recovered camera,
collision or mana-regeneration formula inferred. The final current-source
victory/Realm/region-2 path and 640×480/software runs remain unvalidated.


## Reviewed preparation boundary

The [final coverage audit](rendering-progression-results-20261101/validation-audit-closeout-04.json)
passes with no errors, no indexed-source drift and no changed focused imports.
The three prepared scenario entries bind the already executed native Qt
controller fixtures separately from static input preparation. These controlled
service/state fixtures do not execute the original game or promote any behavior
integration status. The two rejected metadata audits remain immutable historical
records. Historical source staleness and unresolved recovered-range/validation
boundaries remain visible.

[Storage verification](rendering-progression-results-20261101/validation-storage-pointer-closeout-01.json)
resolves 7,943 new generated report references and verifies their original payload
hashes. The [storage review](rendering-progression-results-20261101/validation-storage-review-04.json)
records retained unique inputs and completed execution roots. The [sweep resume](rendering-progression-results-20261101/validation-sweep-resume-after-mission-01.json)
releases this task's clean checkpoint after collection; it claims coordinator
startup only. Local review/backups and the normal commit gate remain independent
of original engine acceptance and evidence publication.
