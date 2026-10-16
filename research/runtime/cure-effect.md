# Cure effect dispatch (casting milestone CURE01)

Scope: pinned No-CD `0x0048b0f0`, spell 41 only. The selected arm starts at
`0x0048b532`; its branches include deferred delivery, target lookup, health and
cleansing calls, ongoing-damage clearing, visible feedback, failure/refund and
effect release. This is a bounded effect-dispatch reconstruction. Full delivery,
lower health/status/resource effects, installed configuration, native World
composition and live replacement remain separate contracts.

## Instruction evidence and confidence

The existing hash-pinned creature-behavior export supplies the selected arm and
shared failure tails. A fresh read-only export under
`working/tests/cure-static-20261016-v1/` verifies the immutable manifest before
and after inspection. Confidence is high for the explicit branch/call ordering;
the selected arm's native projection now has a prospectively bound original
comparison, described below. Lower service internals remain separate.

- A missing target enters failure before checking delivery type. Types 6, 7,
  11 and 30 otherwise request effect state 77 without resolving the actor.
- Other types cache the bounded target lookup. Missing/dead actors fail; a
  living actor with nonzero `+0x7c` returns without a cue, refund or release.
- Success snapshots health, `+0x114` and `+0x8d5`, invokes signed health change
  with configured magnitude, attribution `effect+0x212`, contribution words at
  `effect+0x216`, and final argument 1; then calls `0x00516150`, clears `+0x8d5`,
  and emits cue `0x2dc` at the effect's grid coordinates.
- Feedback `0x0049a350` requires local owner 0..7, target visibility for that
  owner, and either increased unsigned health or a nonzero pre-call status.
- Failure emits cue `0x3f3`. A present source and delivery mode other than 4
  select the source, query its configuration with `0x00411e50`, and conditionally
  refund configured cost shifted left eight bits. Effect release follows.
  A nonsentinel out-of-range source would dereference null in the original;
  native admission must reject it separately.

## Remaining boundaries

Do not infer full Cure equivalence from this arm. Health, status/audio
ownership, feedback, state transition, refund and release are supplied lower
services until independently recovered/composed. Pointer caches become owned
logical indices; callbacks must not resize actor storage or alter the effect
cache out of band. Ordinary FS chain restoration is checked independently of
Windows exception/unwind behavior. The wider spell switch stays incomplete.

## Implementation and current comparison

`reconstruction/casting/cure_effect.*` implements only this selected dispatch
contract. `CureServices` keeps health, cleansing, feedback, transition, resource
mutation and release explicit. Caller-staged state/events are required for
rollback; the primitive can change its cache before a lower service throws.
Native preflight rejects other spell IDs, invalid source/cache identities and
more than 65,536 actors. Invalid target lookup follows the original failure
branch. Those deliberate native refusals are `NP.cure-effect-admission`.

The prospective run
`working/tests/cure-effect-bound-20261016-v1/report.json`, preserved as
`original-cure-effect-bound-20261016.json`, passes 32,768 original32/native64/
ASan+UBSan+LeakSanitizer cases and four separate native admission refusals.
The unchanged whole dispatcher enters through its real spell41 compressed
table arm. Full guarded actor, configuration, effect and spell bytes, ordered
lower-service arguments, contribution-pointer identity, body/table bytes and
per-call FS chain/guard restoration agree. Source/dependency hashes and all
2,927 original files remain unchanged. Standalone CMake build and CTest pass.

The corpus includes source/target aliases, missing/out-of-range targets,
preexisting cache entries, unsigned health changes and wrapping costs, extreme
signed magnitudes/positions, owner/visibility limits, deferred delivery and
feedback based on pre-call status even when supplied health does not increase.
Supplied cleansing may also change health, checking the final post-call feedback
decision. No configured magnitude, cost or gameplay balance value is invented.

The first development run was killed with SIGSYS while starting the 32-bit
reference inside the sandbox. Its failed report is retained under
`working/tests/cure-effect-development-20261016-v1/`; it supplies no comparison
credit. The authorized outside-sandbox development run and the subsequent
prospectively declared final run pass. No binary file was patched: redirects
exist only in the private mapped comparison image after its hash is verified.

`CAST.cure-effect` records scoped implementation/comparison. `SP.041` remains
partial, and native World integration/live replacement remain none. Next:
recover/compose the status-clearing helper's ownership and effect delivery/
release with real health/resource services before claiming a complete spell
lifecycle.
