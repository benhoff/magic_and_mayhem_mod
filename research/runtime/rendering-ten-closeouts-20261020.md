# Ten native rendering policy closeouts — 2026-10-20

All ten selected existing contracts now have scoped implementations supported by
the fresh [TEN2 execution](rendering-ten-closeouts-results-20261020/native-controls-sentinel.json).
Their scopes were retained. Only implementation advances from `partial` to
`scoped`; understanding, original comparison, integration and replacement retain
their independent milestones. These are intentional native policies, not newly
recovered original behavior. No original artifact or game process was consumed.

| Behavior | Completed controls |
| --- | --- |
| `NP.surface-final-word-read` | Actual PE32 facade: twelve DirectDraw1/4 descriptor cases across 1×1, odd-pitch, aligned and page-boundary allocations; final payload and final logical pixel DWORD reads; twelve outside-row Lock refusals; five invalid-size refusals; 512-surface capacity and zero remaining owners. The existing 64 native caller/pixel cases also execute; this is not a fresh comparison with Wine DirectDraw. |
| `NR.native-draw-status-session-file` | 1,024 independently checked stubbed snapshots, startup/map/flush/unmap failures, collisions, forbidden-module refusal and repeated teardown; actual PE32 mapped 96-byte file retains 512 peak surfaces, 3,214 snapshots and zero final resources; an invalid path terminates with code 100. |
| `NR.native-status-outside-drawing` | Optimized and ASan/UBSan nested/latest/no-request deferral, balanced release, underflow and terminal publication checks; actual PE32 `status-flush` schedule admits a competing copy while the publisher stalls after ownership release. |
| `NR.minimap-continuous-read-window` | Fresh per-request bounds, 8,192 per-cell versus two region queries, lower/end/guard/hole/overflow/new-admission controls, six readable modes, four malformed modes on direct and spanning paths and three malformed region answers; optimized and ASan/UBSan execution. |
| `NR.world-source-region-epoch` | 10,000 queries, owner/non-owner dispatch, reset/revalidation, 64-region eviction, content mutation copied freshly, six readable and three malformed protection modes; optimized and ASan/UBSan execution. |
| `NR.readonly-description-continuity` | Actual PE32 cross-thread, copy-timeout, tracker-timeout and exclusive-gate read-only positives; changed-descriptor, copy-timeout and Lock-timeout mutation negatives. Exact forwarding/results/LastError, epochs and independently calculated complete GPU frames remain checked. |
| `NR.masked-update-scratch-reuse` | 100 reused uploads, unknown-word preservation, stale coverage rejection, empty-mask no-op with full input validation, oversized patch refusal before allocation/upload, opposite-aspect growth bound, source/HUD ordering and zero teardown surfaces. |
| `NR.native-colour-rectangle-cache` | Seven independent 12,288-word cached/serial/scalar comparisons, source mutation, ordering/batching, 143 hits, 67 evictions, zero warm source uploads, two atomic refusals and zero teardown surfaces. |
| `NR.native-palette-row-cache` | Six independent pixel checks in each cached and baseline mode; 1,200-row pressure, 1,024-row bound, 242 evictions, an untouched sentinel that exposes queued-reader row reuse, changed palette/coverage, zero warm cached uploads versus one baseline upload, refusal and resource cleanup. |
| `NR.displacement-finite-edge` | Fourteen mask/proof controls and eighteen scalar/GPU high-water edge checks, preserving valid taps and row wrapping, retaining invalid taps, and preserving strict caller refusals after scratch growth. No captured-input or original comparison is claimed by this run. |

The aggregate PE32 schedules pass five valid sessions and three expected
refusals, with thirteen complete independent frame comparisons. Shared scene,
upload and atlas regressions also pass. All 358 declared/compiled/nested source
fingerprints remain unchanged through execution. Prospective behavior/scenario
claims, executable hashes and compiler dependencies are in the immutable report.
The earlier TEN1 result remains unchanged. TEN2 reruns every control after
strengthening the palette fixture; TEN1's source freshness is now historical.

## Confirmed fix

`readable_extent.h` accepted malformed low-byte protection combinations such as
`6` because they shared bits with readable constants. A new regression fails
against the earlier helper. All direct, window and spanning admissions now accept
only the six readable modes, reject guard pages, and reject region answers that
start beyond the requested pointer or wrap their extent. Cache permission does
not establish content identity or safety against excluded external writers.

Surface and masked-scratch fixtures now cover the additional boundary/refusal
cases above. The palette fixture can exercise the existing baseline selector and
independently checks its output instead of assuming the cache is enabled.

## Reproduction and retained attempts

Use an idle marked prefix under `working/tests/drawing-fixture-prefix-suite`;
the runner acquires its exclusive lock, refuses live clients and owns only that
prefix's shutdown. Run:

```sh
python3 tools/working-retention.py run --reserve-gib 3 -- \
  python3 -B tools/test-rendering-ten-closeouts.py \
  --output working/tests/rendering-ten-closeout-next \
  --wine-prefix working/tests/drawing-fixture-prefix-suite/<suite>/wineprefix
```

The successful host run used a private `/tmp` mount through Bubblewrap because
the host temporary filesystem had exhausted its per-user quota. The runner uses
explicit Mesa software dispatch and its own Qt runtime directory. The initial
quota/display failures, private-device setup failure and omitted probe imports
are preserved under `working/tests/rendering-ten-closeout-20261020`. Exact
executed uncommitted source variants are preserved and hash-verified; no failed
report or old evidence fingerprint was refreshed. `--prepared-run` verifies
compiler inputs and executable hashes and reruns every control; it inherits no
successful execution claim.

For the private temporary-filesystem workaround, create an empty directory in
`working/` and run the command above inside:

```sh
bwrap --die-with-parent --unshare-user --uid 0 --gid 0 \
  --ro-bind / / --dev /dev --bind "$PWD/working" "$PWD/working" \
  --bind "$PWD/working/tests/<private-tmp>" /tmp --setenv TMPDIR /tmp \
  -- python3 -B tools/test-rendering-ten-closeouts.py <arguments-above>
```

Keep the outer `working-retention.py run` guard. The namespace exposes writable
execution storage only under `working/` and its private `/tmp`.

The durable evidence is the report here and its linked PE32 result. Local build,
logs, minimal unique-source copies and prefix activity are execution artifacts;
they are retained, not cleanup candidates in this batch. The storage inventory
found no eligible cleanup candidates. Native execution retained over 130 GiB
free. A separate registered review checkout and temporary Git-base accounting
exports also remain guarded by the 90 GiB reserve.

## Remaining independent boundaries

Full public gameplay, physical driver equivalence, unknown external writers and
aliases, hardware performance and live replacement remain outside these bounded
policy closeouts. Existing public failures and their original fingerprints stay
retained. The new implementation evidence does not renew historical live results
or assert that any previously observed public crash, GAP or FPS miss is resolved.

The [committed-history recheck](coverage/rendering-ten-closeouts-incoming-history-20261020.json)
verifies all 323 recorded parent/current file transitions through `5fc819d5`
against their exact receipts, with no unresolved transition. This is accounting,
not a past gate-pass or execution claim.
