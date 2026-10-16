# Cure delivery and native ownership

Work begins from CURE01's compared spell41 dispatcher. This follow-up owns the
bounded direct-delivery path, resource mutation and status cleanup, followed by
a headless native session and explicit checkpoint continuation. Recovery,
implementation, original comparison and native composition are separate stages.

## Initial boundaries

The common effect creator (`0x00493f20`) currently excludes creator ownership
and type11 destination placement. Type setup (`0x00494ab0`) and update
(`0x004970e0`) must establish how initial type11 reaches deferred type77 and
application/release. The status helper (`0x00516150`) clears creature status
`+0x114` and owns an associated audio reference at `+0x120`; its backend and
scheduler operations require explicit ownership. Resource mutation
`0x00507f00` and type predicate `0x00411e50` are lower admission/refund services.
The resource/status helpers and headless native composition below now own their
bounded state; complete native World/campaign integration remains pending.

## Confirmed instruction findings

The pinned NoCD build is `40209ca7…`. Read-only export
`cure-lifecycle-static-20261016.json` records exact function hashes and both
89-entry dispatch tables. Confidence is high within these instruction branches:

- Type11 setup at `0x49507a` resets its animation player; spell41 does not take
  the special spell58 continuation. Its update at `0x499a0e` calls the spell
  dispatcher, which selects type77 before applying health.
- Type77 setup at `0x494dbf` selects/binds an animation through metadata. Update
  at `0x4998f0` follows the target (`0x4896b0`), refreshes its cached actor and
  visible flag, then waits for animation result1. Only then does it call Cure
  and release. Inactive or dead actors branch straight to release without the
  dispatcher's failure/refund. Invalid target indices dereference null here;
  a native refusal must be recorded separately.
- Release includes cell unlink, high-water maintenance, reservation/attribution
  reset, animation reset and optional associated audio cleanup. Those operations
  are not implied by a callback that simply marks the effect inactive.

## Resource and status helper milestone

`cure_services.hpp/.cpp` implements `0x507f00`, `0x411e50` and `0x516150` as
owned typed projections, composing existing audio scheduler retirement
`0x571d20`. `original-cure-services-bound-20261016.json` records 32,768 resource
cases and 8,192 status/audio cases against all four unchanged original entries,
native64, ASan/UBSan/LSan, and standalone CMake/CTest. Immutable manifests pass
before/after. Full guarded objects and ordered UI/COM calls agree. Eight invalid
native graph cases refuse before writes or calls; they are not original cases.

Resource addition wraps first, clamps a signed-negative result to zero, then
caps against the DWORD-wrapped `maximum << 8`. Classes0/24/25/26 are chargeable.
UI notifications require an unblocked local actor and changed resource; the
second notification reloads resource after the first callback. Dirty `+0x92e`
is written even when resource is unchanged. These are baseline rules, not a
new mana economy.

Cleanup always clears status `+0x114` and drops audio reference `+0x120`.
Enabled/initialized manager and a buffer gate backend calls. Failed GetStatus
or playing bit1 triggers Stop and schedule retirement, even when Stop fails.
Successful Stop additionally resets byte position. Retirement preserves ring
membership, advances a retired head or moves a middle node to the tail, clears
the external output and resets scheduler fields. Wrappers/buffers are not freed.
The owned projection uses logical vector indices and reuses the recovered audio
retirement implementation. Malformed rings and absent associated scheduler nodes
are outside the original's safe domain; native preflight refuses them explicitly.

Lower UI/audio backends are supplied. Installed assets, complete effect creation,
animation timing, native World/campaign embedding and live replacement remain
separate milestones. The owned lab is described below; static delivery findings
are not lifecycle equivalence.

Use `tools/export-cure-lifecycle.py` under the storage guard for a fresh static
export with immutable-manifest checks. Do not infer whole-spell equivalence or
live replacement from static dispatch targets or supplied lower services.

## Owned native lab milestone

`apps/cure-lab/` now composes the compared admission/effect/ordinary-positive-
health/resource/status contracts on owned actors, effects, logical audio state
and learning/visibility records. The headless application runs without Wine or
original assets. Cast and update publish a staged candidate; generation handles
refuse completion input for reused slots. The portable version1 checkpoint
retains in-flight type11/77 state, all owned consequence state and pool identities.

CURE03's native evidence covers 4,096 parameterized cases on native32/native64
and sanitizers, standalone CMake/CTest, exact state/event continuation after
admission/deferral/waiting, and a separate-process checkpoint/resume demonstration.
The demo heals9→17 and debits20→17 resource units. Native refusal tests include
stale/duplicate completion inputs, absent audio replies, invalid casts, full pool,
insufficient resource, damaged/truncated checkpoints and valid-checksum malformed
payloads (including invalid version/count/Boolean/reference/ring fields).

The lab deliberately supplies animation results and audio replies. First-free
allocation, empty contributor chains, logical grid following, effect disposal
without spatial/render registration and staged publication are native policies.
Type77 dead/inactive target failure releases without refund; a living blocked
target releases at completion without health/status mutation. Original whole
creator/follow/setup/animation/release equivalence has not been executed as a
lifecycle. Projectile delivery, installed parameters/assets, negative magnitudes,
wizard/special target health, physical audio, native World/campaign-envelope
embedding and live replacement remain pending. The lab's native integration
evidence does not promote `CAST.cure-delivery` comparison or whole-spell `SP.041`.
