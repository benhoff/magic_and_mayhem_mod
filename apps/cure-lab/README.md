# Headless native Cure lab

This application-owned composition runs without Wine, Qt or original assets.
It connects the compared casting admission, Cure dispatcher, ordinary positive
health, resource and status helpers to owned actors/effects and checkpoints.
It is a bounded integration harness, not campaign spell replacement.

```sh
python3 tools/working-retention.py run --reserve-gib 0.2 -- cmake -S apps/cure-lab -B working/cure-lab
python3 tools/working-retention.py run --reserve-gib 0.2 -- cmake --build working/cure-lab -j2
ctest --test-dir working/cure-lab --output-on-failure
working/cure-lab/cure-lab --checkpoint working/cure-mid-delivery.bin
working/cure-lab/cure-lab --resume working/cure-mid-delivery.bin
```

The demo checkpoints type77 before completion, then restores and supplies an
explicit animation-complete input. Target health goes9→17, caster resource20→17,
and status/ongoing damage clear. The file retains the in-flight state so a second
process can resume it. The test runner performs native64, native32 and sanitizer
checks, exact checkpoint continuation and refusal tests.

`CureSession` owns actors, health/resource/status state, a fixed generation pool,
learning/observation state and logical audio schedules. Cast/advance stage their
candidate and publish only after validation. Returned events express cues,
feedback, UI notifications, health changes and supplied audio operations; the
lab does not play media. No application widgets or build-specific contracts are
added as dependencies of `game/`, `audio/` or `renderer/`.

Recovered dispatch orders are type11→77, then follow/visibility/wait, apply and
release on animation result1. Dead/inactive deferred targets release without a
refund; blocked living targets release at completion without healing/cleanup.
The source pays through real wrapped/clamped resource rules. Ordinary class1
targets use the recovered signed-health backend. Missing animation results wait.

Native policies remain explicit: first-free bounded pool, monotonically changing
generation handles, validated logical references, no spatial/render registration,
empty contribution chains, supplied animation/audio replies and staged publication.
Grid target following represents current owned target position; the original
follow/creator/animation-binding/release functions have not been compared as a
complete lifecycle. Costs/magnitudes are fixture inputs, not installed spell
configuration or balance changes. Projectile types6/7/30, wizard/self/special
target health, negative magnitudes, audio playback, native World integration,
campaign checkpoint embedding and live replacement remain outside this lab.

See [delivery findings](../../research/runtime/cure-delivery.md) and
[checkpoint format](../../research/formats/native-cure-session.md).
