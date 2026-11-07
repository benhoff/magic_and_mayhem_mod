# Ten FPS completion conditions reconciled — 2026-11-07

Ten registered component gaps no longer carry a fixed FPS target as a mandatory
native-rendering completion condition. The existing user decision of 2026-10-25
already made FPS diagnostic. This review applies that decision to older component
entries; it changes no engine code, benchmark outcome, acceptance requirement,
behavior contract or milestone. These are ten scoped requiredness closeouts,
not ten new feature or player-acceptance passes.

The authoritative [performance policy](../../docs/native-rendering-closure.json)
requires functional actions, output, forward progress, response deadlines and
resource bounds. None is waived. Original timing/cadence fidelity and source-floor
reliability remain within their existing contracts. Experimental 60FPS,25/20.5FPS,
16.67ms and19FPS outcomes remain recorded at their original source phases; a
missed benchmark is still a missed benchmark.

| Registered item | Functional/recovery work retained |
| --- | --- |
| `NR.scene-production-repetition` | Nested/map/visibility/timer behavior, independent scheduling, original helpers and pixel/lifecycle proof. |
| `NR.scene-draw-budget` | Four prediction misses, interpolation, default activation and unclassified module/map/nested/pause/network behavior. |
| `NR.world-owned-snapshot-delivery` | Source progress/reliability, full public functional Quit, loss/Restore and original equivalence. |
| `NR.native-prepared-world-replay` | Public HUD/presentation acceptance, interpolation, original/driver parity and retained live failures. |
| `NR.native-prepared-command-frame` | Live output/driver equivalence, physical scanout/interpolation and real copy-order history timeouts. |
| `NR.masked-update-scratch-reuse` | GUI response cost, original comparisons, interpolation, missing branches and default-route regression checks. |
| `NR.native-colour-rectangle-cache` | Original output/replacement, unobserved failures, providers, movie/World integration and historical source limits. |
| `NR.masked-displacement-admission` | Original raster/callers, additional ownership/failure routes, hardware output and historical failures. |
| `NR.native-retained-world-image` | Copy-order GAPs, OS-stall resilience, driver/pixel/scanout equivalence and interpolation. |
| `NR.native-world-owner-costs` | Actual busy live workload, original simulation CPU, GPU attribution, scanout and historical source limits. |

The [immutable review](rendering-fps-scope-results-20261107/reconciliation-01.json)
pins each exact prior/revised gap, unchanged behavior contract/stages, the existing
policy bytes and the executed controls. Both existing completion and acceptance
guard suites pass, including rejection of a new FPS floor, removed response or
resource bounds, a reduced requirement denominator and combined final decisions.
No new fixture or test was introduced.

The accepted pack remains52passed/104pending and historical for its six changed
native files. Both final decisions remain pending. This review closes only the
ten FPS requiredness conditions. Pixel mismatches, prediction errors, copy-order
failures, missing branches, unknown ownership/callers and historical source
limitations remain explicit; none becomes a performance-only diagnostic.
