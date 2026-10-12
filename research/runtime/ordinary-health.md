# Ordinary creature damage, health and death (NS23)

This increment recovers a bounded No-CD health contract and connects its numerical
projection to the native arena. Original melee RNG/timing, targeting, spells,
leader/illusion/explosion/scenario deaths, whole callback effects, experience
semantics and live replacement remain pending. The arena still uses authored
spell, AI, mana, profile and presentation policies.

## Evidence and confidence

The immutable input is `original/Arcane_Nocd/Chaos.exe`, SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
The working executable has been patched by other runtime work and its hash-gated
export was refused. Preliminary disassembly of that working input supplies no
confirmed finding. Fresh read-only pinned exports and isolated execution are
retained under `working/tests/health-ns23/`; original manifests are verified
before and after every comparison, including failure.

**High confidence within the declared projection:** mapped unchanged x86 code
for `0x005075f0` maximum, `0x005076f0` signed health, `0x00514860` defense,
`0x00514990` damage/lethal branching and `0x0051f5e0` ordinary cleanup. The
fixture is class1/type17, rank0 for damage/lethal, empty original creature/effect
pools, null cached contribution links, zero mana/multiplier and no special death
flags. Maximum arithmetic separately exercises ranks0/1/2. Source addresses are
build-specific preferred VAs, not persistent host pointers.

The comparator executes these five original functions on Linux/i386, without
Wine. Eighteen lower helper endpoints are redirected only in private mapped
memory after whole-image verification; the five compared bodies remain byte
identical and are checked after every case. Endpoints capture ordered arguments
or return explicitly supplied status, response and feedback values. They do not
implement the original helper effects. Object changes outside the declared
field projection fail; configuration and target text guards are checked.

## Confirmed behavior

| Contract | Finding |
| --- | --- |
| Ordinary maximum | Configuration `+0x18`; rank `creature+0x124` values1/2 add low32(base×global bonus)/100 using unsigned arithmetic; other admitted ranks retain base |
| Defended admission | Signed amount<=0 returns; status lookup returning8 and `+0x8c5` gate refuse before health mutation |
| Mitigation | `+0x90` enables low32((100-global percent)×amount)/100, unsigned; rejected/absorbed difference is sent to feedback before remaining-positive admission |
| Quarter reduction | `+0x81d` reduces positive remaining damage with integer division by4 |
| Absorption | Modes1/2 query the response helper; a true response replaces amount with current health; false mode1 reports absorption and returns, false mode2 reports absorption then calls signed health directly |
| Signed health | Global byte `0x006e9858` or already-zero health returns; negative delta negates its DWORD bits and calls damage; nonnegative delta uses wrapped DWORD addition and unsigned maximum cap |
| Incoming display fields | Nonzero owner fields `+0x24+i*4` receive attempted damage at `+0xdeb+i*4` and reset age at `+0xe0b+i*4`, before nonlethal/lethal selection |
| Ordinary nonlethal | Subtracts damage; optional selected-owner pain callback sets `+0x660=250` when its signed cooldown is nonpositive |
| Ordinary lethal | Clears `+0xb8b`, calls scenario/kill tracking and the unchanged cleanup body, clears `+0x104`, requests behavior5 and repair; active slot flag remains set |
| Selected cleanup fields | Health, `+0x9c/+0x778/+0x780/+0x124/+0x889/+0x8bd` become0; target `+0x64c` becomes-1. This is not full lifecycle equivalence |
| Refresh | Admitted signed changes compute min14(unsigned low32(health×15)/maximum), or0 for zero maximum, notify the bar and set dirty `+0x92e=1` |
| Attribution | Source attribution callback receives requested damage and target owner; supplied contribution lookup records receive the supplied feedback metric. Experience meaning and the metric calculation remain unrecovered |

The lethal feedback callback receives post-cleanup health zero in the declared
zero-mana fixture. The first differential run caught the native model's incorrect
pre-cleanup-health assumption; that negative run remains retained. Zero-maximum
healing can numerically reduce health to zero without taking the lethal branch;
that original edge case is reproduced in the model and refused by the native
adapter's positive-maximum domain. Direct health changes and defended damage
remain separate APIs; healing does not pass through defense.

## Native integration

`game/simulation/health_rules.*` defines a stateless native backend interface,
without Qt or executable addresses. `apps/skirmish/health_rules.*` owns the
build-specific adapter to `reconstruction/combat/health.*`. Its bounded domain is
health<=maximum<=1,000,000 and signed change within ±1,000,000. The arena uses
neutral defenses and the ordinary numerical projection for its fictional units;
this does not reproduce original wizard/leader death branches or introduce
balance changes. Native ownership still performs motion cancellation, occupancy
removal, retained identities and match outcome handling separately.

`CombatSession::heal` uses direct signed health, reports actual gain, caps at
maximum and refuses dead/stale/invalid targets. No healing spell is added.
Damage and healing stage the whole candidate before publication. Invalid or
throwing backends cannot publish partial damage, healing or an advanced tick.

New sessions bind backend identity `0x4e53323348454131` in combat-v3 checkpoints.
The app explicitly resolves that immutable contract or legacy identity0; unknown
identities fail. Legacy v1/v2 saves keep their authored backend, and same-session
restore refuses a mismatched backend. Pending commands and every existing
movement/cast/projectile phase remain serializable. See the
[wire extension](../formats/native-combat-subsystem.md) and
[arena instructions](../../apps/skirmish/README.md).

## Reproduction and remaining boundaries

```sh
python3 tools/working-retention.py run --reserve-gib 0.05 -- \
  python3 tools/test-ordinary-health.py --output working/tests/health-next \
  --claims working/tests/health-ns23/claims-original-final.json
```

The prospective declaration must match current sources; prepare a new one with
`draft-coverage-claims.py` after any source/scope changes. The runner compiles
native, native ASan/UBSan and private i386 reference programs, checks compiler
closure and original/source stability, and refuses retained output overwrite.
It needs installed i386 C++ runtime support; the restricted syscall sandbox does
not support native i386 execution. This is an isolated-original comparison,
not execution of the game or live bypass.

The scoped corpus covers 8,614 cases and 36,803 ordered endpoint events, plus
literal numerical/admission controls and a continued damage/heal/death sequence.
Native integration tests cover heal cap, owned death/occupancy cleanup, legacy
and bound checkpoint resolution, unknown/wrong rules, invalid/throwing backend
rollback and pending-order continuation. Fresh execution records and final
window/headless continuations are registered separately after completion.

Open work includes real installed creature profiles/status lookup, nonempty
world cleanup, cached contribution chains, the feedback metric and callback
implementations, original leader/illusion/explosion/scenario branches, melee
formula/RNG/event timing, full spell/mana/AI behavior and live replacement.
`GP.combat` retains its broader static/partial scope; new `CB.ordinary-health`
and `CB.defended-damage` account only for the confirmed projection, while
`NP.ordinary-health-adapter` owns the native integration policies.

## Final execution records — 2026-10-12 UTC

- [Original/native numerical and callback projection](ordinary-health-original-20261012.json)
- [Native arena Debug integration](ordinary-health-arena-normal-20261012.json)
- [Native arena ASan/UBSan integration](ordinary-health-arena-sanitized-20261012.json)
- [Production Qt window journey](ordinary-health-window-20261012.json)
- [Automatic v1 melee Debug regression](ordinary-health-combat-normal-20261012.json)
- [Automatic v1 melee sanitizer regression](ordinary-health-combat-sanitized-20261012.json)

All declarations precede their executed results; source hashes remained stable.
The original model run compares8,614 projected cases/36,803 events and includes
native and sanitizer controls. Forty-two headless regressions pass; eight focused
sanitizer CTests pass. Each arena build passes32 exact fresh-process continuations
and18 malformed/CLI/binding refusals, with separate native health backend contracts.
The real timer window wins at31 ticks and loses at29; five checkpoint phases
restore to identical native/headless state and canonical final pixels. Debug
and sanitizer automatic-only v1 encounters each retain32 continuations,7 hits,
1 death and15 refusals. Qt/window sanitizing is not part of this execution.

Full logs, prospective declarations, build commands and raw negative/development
runs remain under `working/tests/health-ns23/`; none of the initial failures is
registered as successful evidence. The original working-copy hash refusal and
restricted i386 syscall failure are also distinct from successful execution.
The [committed-history review](coverage/ordinary-health-history-20261012.json)
checks122f5c8..0144cb6c:354 file/789 behavior transitions. Thirteen missing
intermediate movie-startup dependency receipts were appended as pending
retrospective accounting, without promoting original/movie validation.

Incoming commit `d94bd66f` was also checked after the initial review:26 file and46
behavior transitions have exact receipts, with no missing versions.
[Incoming history review](coverage/ordinary-health-incoming-history-20261012.json)
remains accounting only, separate from health execution and concurrent media work.

## NS24 contribution/identity follow-up

The melee caller supplies a contribution-pair pointer even when its two IDs are
UINT_MAX. The model now exposes both IDs and each lookup return as explicit
boundaries. Null returns do not update contribution records. Ordinary lethal
cleanup passes the target slot read from creature+0 to the detach endpoint;
this is independent of contribution-pointer presence. The standalone original
comparison includes null/non-null combinations and slots0/31/62/93. Historical
NS23 reports retain their original sources and scope. This does not recover
lookup/experience internals or whole entity-pool cleanup.
