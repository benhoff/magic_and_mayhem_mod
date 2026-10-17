# Native staged-mission wire extensions

These are native policies, separate from original campaign save layouts.
All scalar fields below are little-endian. Existing encodings are preserved.

## Campaign version3

The `MNMCAMP\0` campaign blob selects version3 only when an active authored
mission has objective continuation. It retains version2 fields and appends:

- stage uint32 (0: initial group; 1: reinforcement; 2: rival wizard);
- reinforcement-admitted byte, strictly0/1;
- initial-group count uint32 and slot/generation uint32 pairs;
- reinforcement-group count uint32 and slot/generation uint32 pairs.

Each group has at most8 actors; the initial group is nonempty. A handle must
have nonzero generation and be unique across both groups. Stage0 has no wave;
stage2 requires an admitted nonempty wave. A mission record requires an active
campaign battle. Content/session validation additionally checks exact group
sizes, living/dead stage conditions, creature role and hostile ownership.
Idle new campaigns still encode as version1; settled results encode as version2.
The outer native campaign checkpoint remains unchanged.

## Skirmish MNMARM03

Mission-controlled battle metadata uses the eight-byte `MNMARM03` header and
retains the existing rules hash. It inserts two uint32 flags after that hash:
custom mission present and victory enabled. Both are strictly0/1. If custom is
set, the existing MNMARM02 mission geometry follows; then the ordinary skirmish
fields continue unchanged. With custom clear the standard arena is selected.
Mission control suppresses the authored rival casting AI. The campaign supplies
the victory gate when admitting a tick. MNMARM01/02 retain their prior encoding.

## Creature CRB4

The creature continuation tag `CRB4` (uint32 0x34425243) has the same field order
as CRB3: owned profile, nonempty MTER02 terrain, RNG, controllers. It selects the
native creature-before-wizard acquisition and mixed-body wizard reach policy.
Controllers remain Crocodiles only; wizard handles may be targets, and receive
shared health/death without creature stun/knockback state. Profile resolution
requires agreement with the immutable driver policy. CRB1/2/3 do not gain wizard
targets or the new acquisition fallback.

## Authored JSON definition version1

Exact keys: `version`, `id`, `briefing`, `objectives`, `initial`, `reinforcement`,
`reward_spell`. Objectives has exactly three nonempty descriptions; group entries
are bounded integer XYZ arrays. Version is1 and the supported reward is `cure`.
The compiled definition's exact bytes join map/profile/mission bindings in the
campaign content revision. Continuation never chooses arbitrary installed files
or overrides its owning content's objective/reward definition.
