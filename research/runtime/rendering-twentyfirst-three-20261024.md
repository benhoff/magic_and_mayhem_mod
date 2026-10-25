# Twenty-first bounded rendering batch (2026-10-24)

The existing forward animation, placement and selected mode-one attachment models
now have current-source execution proof for their bounded offline implementations.
`AN.forward`, `AN.placement` and `AN.attachment` move implementation from partial
to scoped. Their understanding remains partial, comparison recorded, and
integration/replacement none. No production behavior change was required by these
controls; this batch closes the missing boundary validation and review work.

- `AN.forward`: 22 bounded original/native forward and phase-preserving switch cases match 2134 complete controller states. Native full-width delay continuation and atomic invalid-restore refusals pass.

- `AN.placement`: 2400 synthetic boundary and 88512 installed Effects2 helper results match the four original placement helpers. All 4608 original body cases match, including 2304 fallback starts and 2304 retained displays. Native failed body candidates preserve dormant phase and owned records.

- `AN.attachment`: 32 original configured mode-one cases match 2080 child states, same-mode no-op, removal and reentry. Native all-eight-facing DWORD endpoint rebinding, source ownership, health extremes and failed-candidate preservation pass.

## Native and original boundaries

The new standard CTest checks a delay reaching UINT32_MAX without wrapping,
full controller state preservation on invalid restore, dormant body phase/owned
source preservation after malformed or cyclic fallback failures, and eight
explicit attachment rebindings to exact DWORD endpoint selections. Callback
arguments and counts, copied records, retained display snapshots, invalid modes,
overflow, same-mode short-circuit and signed health extremes are checked. A
stop-only child is valid for mode one; a body fallback without a displayed record
is refused. Raw forward dispatch safety failures can retain intermediate control
state: discard or restart such a player. This batch does not change that policy.

Original controls privately map the hash-pinned, unchanged no-CD PE32 routines.
They use owned bounded fixtures and no Win32 calls, Wine prefix or live game.
Forward controls cross delays 0/1/2/7, repeats 0/1/3, signed event endpoints,
unknown-opcode skipping, a UINT32_MAX delay, stop, break and six switch phases.
Four placement helpers cross all 25 signed DWORD metadata endpoint pairs over
four footprints and six views, plus 922 installed Effects2 sprite records. The
body helper includes real original start calls, all eight facings, inclusive SPR
bounds/frame-zero fallback and both existing/missing display branches. Mode-one
controls exercise configured base40 with assets0/2/12 on synthetic sequences and
the installed Effects2 asset2, eight facings each. Original file/configuration
loading, real resource identities and health-gated original drawing are not
executed by these attachment fixtures. Fixture metadata and no-CD linkage do not
establish complete game rendering equivalence.

## Evidence and reproduction

[Execution 2](rendering-twentyfirst-three-results-20261024/execution-2.json) stores
prospective contracts, complete declared source/compiled header identities,
commands/log hashes, five passing Release CTests, five passing ASan/UBSan CTests
and the original comparisons. The original manifest passes before and after the
experiment; the PE SHA-256 is
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
The installed ANI is a read-only input. Historical installed-wide/image proofs
retain their original source hashes and do not become fresh through this result.

Run `tools/test-rendering-twentyfirst-three.py` with a new checkout-local
`--output` under `working/tests`, a new `--report` under `research/runtime`, and
the existing installed `--asset-root`. It chooses a private TMPDIR, generates
bounded ANI interchange fixtures and writes both success and failure directly
through the SQLite report writer. The failed [first execution](rendering-twentyfirst-three-results-20261024/execution-1.json)
stopped before compilation because the installed filename was lower-case;
[the exact prior source](rendering-twentyfirst-three-results-20261024/failed-source-variants.json)
is retained. Both manifest bookends passed for that failure.

The user [approved an 89 GiB reserve for this batch](rendering-twentyfirst-three-results-20261024/storage-approval.json)
after the required retention inventory found no eligible cleanup. A separate
batch-only guard policy implements that approval; the tracked normal90 GiB
policy is unchanged. The isolated source checkout, build/log output and pinned
SQLite payloads/backups are on the Videos drive. No installation or full source
snapshot is created. Binaries, ANI interchange and build/log directories are
retained execution outputs; SQLite reports and exact failed source are durable
evidence. Local completion retains verified backups; no publication is requested.

The [planning UI controls](rendering-twentyfirst-three-results-20261024/ui-controls.json)
pass all 38 tests. The [coverage audit](rendering-twentyfirst-three-results-20261024/coverage-audit.json)
has no errors and recognizes current implementation/comparison support for the
three selected contracts. Historical gaps remain preserved. The
[incoming history review](rendering-twentyfirst-three-results-20261024/incoming-history.json)
finds matching exact receipts for all 55 latest-merge file transitions.

## Remaining boundaries

`AN.forward`: Reverse modes, full effects/event consumers, wall-clock/global cadence, complete entity lifetime and live integration/replacement remain separate. Native safety exceptions do not recover original unsafe failure behavior.

`AN.placement`: World coordinates/callers, SPR storage identity/resource lifetime, full scene production and live integration/replacement remain separate. Native source resolution and malformed/null fallback refusal are intentional policies.

`AN.attachment`: Other numeric modes, automatic stopped-child restart, global cadence, original file-loader/configuration I/O, complete effect/entity lifetime and live integration/replacement remain separate.
