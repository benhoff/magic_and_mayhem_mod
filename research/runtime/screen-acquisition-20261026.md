# Screen acquisition and controlled ownership composition

Pinned No-CD build: `40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
The independent DWORD-memory models live in
[`screen_acquisition.hpp`](../../reconstruction/rendering/screen_acquisition.hpp).
Confidence is high within the admitted normal-return caller scope.

| Caller | Recovered local ownership | Explicit lower boundary |
| --- | --- | --- |
| Main layout `4a6f70..4a7024` and prepare `4a7110..4a756d` | Seven layout records and cookie; allocated canvas and surface descriptor; optional canvas callback; six buttons with immediate sixth retirement; label, vector membership, restriction path and initialized guard | CRT allocation/array construction, layout parser, canvas/asset/label/button/vector providers |
| World entry `46a460..46a92e` | Owner flags, timer selection and service ordering; three sprite loads with continuing failure warnings; roster, optional logging/countdown/network publication; initialized state | Simulation, sprites, audio/other services, OS/timer/network implementations |
| Report layout `474a50..474b03` and prepare `473d70..4745c8` | 23 layout records and cookie; embedded surface, sprite, text buffers, callback, 21 labels and button; conditional label retirement, score records and campaign totals | Config/layout/asset/control/score/heap/CRT providers |

Each provider event compares the complete owned byte state as a lossless delta
from the initial snapshot, ordered receiver/arguments, all 27 surface descriptor
DWORDs, normalized stack strings and network packet contents. The final owned
state is compared directly. The original local caller bytes remain unchanged;
only named lower endpoints and fixture virtual slots are adapted.
`4d2970` explicitly loads only the byte at stack argument `+4`, so unused upper
caller-saved bits are excluded from the semantic color argument.

The corpus covers both resolutions, zero and nonzero initial bytes, existing
canvas callbacks, repeat Main preparation, failed bitmap/sprite/config loads,
network/roster/countdown branches, positive and negative nonzero timer divisors,
zero through four score records and provider mutation of resolution, control
arrays and World mode flags. The composed schedules execute Main construction/preparation, Main
release, World entry, report preparation/full exit, World exit and Main
preparation/enclosing destruction again. Existing release/retirement models are reused; their
source hashes and previous isolated comparisons remain unchanged.

The composed sequence uses admitted screen state and controlled providers. It
does not prove the real Main/Realm/World/report routing, recover World
construction, or validate a whole installed player session. In particular,
World service-owned simulation/bitmap storage is explicitly normalized to the
empty admitted exit fixture before its local exit. No live hook or replacement
is added. Heap failures, SEH unwinding, arbitrary reentrancy, asynchronous
aliases and transitive providers remain separate contracts.

## Negative findings and admitted limits

The first attempt found a fixture adapter signature mismatch at compilation.
Attempts 2–5 also retain an oversized-score failure. Attempt 5's debugger trace
shows score count 3, a valid three-record score provider, but the loop-count
stack slot contains ASCII from `-2147483648`; the caller subsequently accesses
an out-of-domain score record and faults at `47446e`. The local report uses a
seven-byte decimal scratch region around `474434..474458`; its CRT conversion
has no capacity parameter. This supports a bounded original-code overflow
finding, not a claim about naturally reachable game scores.

The normal-return comparison admits at most six decimal characters including
the sign, plus a terminator, up to four score records and bounded input
strings. Larger values and their failure/recovery behavior remain unimplemented
outside this scope. The independent model's string storage is safe, but no new
gameplay score policy is introduced. Allocation providers return writable
admitted storage; lower constructors/parsers/destructors are authored endpoints.
Zero timer divisors and exceptional allocation/SEH outcomes are outside scope.

The durable reports and failed diagnostics are immutable SQLite references in
`screen-acquisition-results-20261026/`. Bottom-up comparison, controlled
composition, top-down installed acceptance and live replacement remain separate
milestones.

## Current result

`RSA26.acquisition` binds `screen-acquisition-results-20261026/acquire07.json`:
768 schedules and 192 composed lifecycles pass identically on optimized32,
ASan/UBSan32 and ASan/UBSan64, including complete Main construction/destruction
and report exit. The source fingerprints, three behavior contracts, 14 closed
local CFG ranges and before/after original manifests are retained. Attempt6
is a successful historical result for the earlier release-only composition,
not validation of the subsequently extended source.
