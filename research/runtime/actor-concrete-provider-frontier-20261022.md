# Concrete actor, resource and surface providers

The twelve supplied roots left by the owned actor/source batch now have raw
value-address implementations in `actor_frontier_providers.hpp`. Its public
`FrontierComposition<M>` forwards these twelve entries and the prior 171 routes
through one backend. Caller-owned memory, allocation ownership and meaningful
calling-convention inputs are retained; widgets and host pointers are absent.

This is bounded recovery, with high confidence within the declared inputs.
It does not establish recursive provider closure, original planner cadence or
live replacement. The broad Critical actor downstream contract remains partial.

| Original NoCD entry | Recovered behavior | Lower boundary retained |
| --- | --- | --- |
| `4644d0` | ANI 3/4/5 reload, conversion, descriptor publication, short-read/error retention and cleanup ordering | File/size/read/close, path/dialog/window endpoints; associated sprite loading is composed |
| `488330` | Head/interior effect unlink and empty-column flag admission | Valid owned cells and acyclic manager indices; empty-column predicate is embedded |
| `493f20` | Effect description copy, coordinate clamping/wrapping, owner lookup, placement, chain append, particle admission and publication | Creator metadata, subtype setup, special placement and particle construction remain named ends |
| `4eb820` | Frame bound, old/new terrain-class flags and zero-frame cleanup | Map cleanup callback; the affected flags are at cell+10 |
| `4f4330` | Single-cell passthrough and multi-cell wrapped support aggregation | Lower support query; direct width-one coordinates and whole return value are preserved |
| `5050d0` | ANI/profile bank transform validation for initialized counts >=8 | Concrete transform/reflection helpers; lower counts read an original uninitialized local |
| `505220` | Eight movement banks, optional flight banks, cumulative-delta redistribution and attack ANI statistics | Profile postprocessing; sequence access and controller scanning are composed |
| `50d060` | Stun admission, duration/status branches, target references, behavior transition and effect retirement | Selected task handlers, target lookup and selection notification |
| `50e400` | Large-creature attack banks, wrapped relative positions, target-size tables, facing/attachment control and fallback | Alternate animation notification; original table bytes are checked before fixture initialization |
| `57d310` | SPR v4 old ownership cleanup, file/layout publication, palette admission and frame palette-pointer rewrite | File imports, palette builder bodies and non-paletted conversion |
| `57d690` | Modes 0..9 append at each palette-chain tail, including partial failure retention | Ten palette-table builder bodies; count is ECX, tail slot EDX and palette pointer is on the stack |
| `58c8a0` | Rectangle forwarding, window translation, slow/fast copy, error notification, source/destination restore, key/reload and retry | Actual window/COM/driver endpoints; retry remains unbounded as in the original |

The stable behavior is `RS.actor-concrete-provider-frontier`. The independent
headless scenario is `actor-concrete-provider-callers-20261022`. The executable
matrix includes 64 variants for each root and the real `414890`, `4637c0` and
`5055a0` callers: 960 normal-return cases and 1920 before/after teardown snapshots
per build. The final run compares 515,692,604 bytes per stream.
Native64, pinned original32 and native ASan+UBSan streams agree.
The original root bodies and reused callees are byte-guarded before execution
and after every case; trampoline extents and every entry/IAT input are checked
before any patch is installed. Adjacent IAT guards are checked together.

Inputs are independently authored ANI 3/4/5 programs, SPR v4 buffers, map/actor/
effect ownership and finite provider outcome schedules. Streams include selected
globals, seeded storage, every live/freed allocated byte, file cursor/open state,
meaningful returns, ordered callbacks and shared state at each callback. Borrowed
stack/scratch addresses are normalized with initialized payloads retained. Void
EAX is excluded. No original execution output supplies a native seed.

SPR tests include cleanup of old chains/backing, missing/short/bad-version files,
allocation failure, palette counts 0..5, zero shade count and failed chain append.
ANI tests include conversion and reload ordering, short reads, associated SPR
loading and path failure. Support inputs include negative and multi-period XY;
width-one passthrough is separate. Surface tests include fast/slow flags,
window translation, non-loss errors, restore failure/success, key restoration,
null/default/virtual reload and eventual successful retry. Attack tests retain
all 25 authored spatial positions, including rejected/fallback inputs, both target widths, fallback and facing paths.

Original failed ANI path lookup can leave its file open. A failed associated SPR
can close the ANI handle, notify/destroy the window, then continue the original
body. The fixture supplies an explicit continuation outcome for that named file
endpoint. This is control-flow evidence, not a claim about using a closed CRT
`FILE*`. Partial state is compared before explicit caller teardown; teardown
then closes remaining fixture handles and frees every live allocation.

Excluded domains remain explicit: effect diagnostic file logging, malformed
buffers/strings and invalid/cyclic manager chains, unknown/outdated ANI formats,
indeterminate `5050d0` counts below 8, arbitrary imported backing, throwing/SEH
cleanup, actual file/platform/palette/task/subtype implementations, arbitrary
installed profiles, original owner/planner scheduling, and physical driver or
live acceptance. Scripted restore success does not validate a legacy driver.

The current [execution record](actor-concrete-provider-results-20261022/frontier-final-1.json)
and [routing reconciliation](actor-concrete-provider-results-20261022/reconciliation.json)
are immutable SQLite payloads. Failed and superseded attempts keep their exact
prospective sources and results; their histories do not receive current proof.
The [incoming history check](actor-concrete-provider-results-20261022/incoming-history.json)
reviews 34 parent/current file transitions in `18baf155..d48226f4`, with no missing
exact receipts or rewritten prior receipts. It does not assert past execution.

All 43 historical semantic leads are retained as 24 concrete routes and 19 embedded
scoped helpers. This closes that named list only within its documented scope.
The newly visible lower frontier includes 37 endpoint specifications (31 real
file/profile/palette/subtype/platform ends and six authored COM tokens), leaving
124 supplied endpoint specifications across the whole composed harness. Some
are inherited and unexercised by this batch. Font rasterization, task/planner and
scenario subtype bodies, installed-session acceptance and unresolved indirect
targets remain work; no total-engine completion percentage follows from counts.
