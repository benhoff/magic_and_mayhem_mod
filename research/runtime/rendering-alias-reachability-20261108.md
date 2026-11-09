# Natural alias/overlap COPY investigation — 2026-11-08

Both `operation.copy-alias-overlap` profile rows remain pending. The accepted
stretch rows and the 116/156 accepted parent requirements stay closed under their
recorded source phases. This investigation changes no renderer, runtime hook,
game input, acceptance requirement or original instruction.

The bounded inspector checks all 156 retained direct-call candidates targeting
the seven NoCD COPY wrappers against a fresh hash-checked original disassembly.
It tracks only straight-line argument expressions. Every direct control-flow
target, discontinuity, call or unsupported instruction discards earlier state;
stores invalidate cached memory/stack relationships. Results identify leads,
not complete function semantics, executed callers or allocation identities.
Distinct wrapper constants never establish disjoint backing.

The [source-bound assessment](rendering-alias-reachability-results-20261108/assessment-02.json)
retains 29 distinct constant-wrapper pairs and 127 unresolved expression pairs;
none is a same-expression candidate. The 127 unresolved pairs are not rejected
aliases or recovered nonaliasing callers. The exact candidate set matches the
older retained static inventory, including its unknown ownership boundaries.

The selected deferred queue writer at `0x58d5b0` stores the incoming source
receiver at entry offset `+8` and destination argument at `+12`. The drain at
`0x58d660` loads these separate fields into the source receiver/destination
argument for calls at `0x58d6db`, `0x58d6f0` and `0x58d709`. Queuing therefore
does not by itself create a source/destination alias; equality depends on the
original publisher. The selected queue gate and 128-entry capacity are static
observations, not a complete queue lifetime or overflow-recovery proof.

The generic image draw at `0x572e90` loads its destination from owner `+8` and
uses its embedded source wrapper at owner `+12`. Setter `0x572f20` assigns that
destination without a local equality check. The two-piece bar draw at
`0x4dd9b0` similarly takes its source through owner `+4` and its destination
through the caller argument. These remain concrete publication/argument leads;
neither field layout proves actual backing identity or an overlapping rectangle.
All three direct setter callers (`0x4b539f`, `0x55176b`, `0x57907b`) explicitly
publish destination `0x6a49e0`. Both bar callers remain concrete: `0x4b506c`
passes the bar's field `+0` as destination, and `0x4c8351` passes owner field
`+0x84c` to the bar at owner `+0x95c`. Their source field `+4` publication and
actual allocation relationships remain unresolved. The inspector also retains
the 67 direct queue-publication contexts, without relabeling them as executions.
The cursor save/restore paths retain their separately documented wrapper pairs.
The pinned executable imports no BitBlt; the standalone native DC-copy controls
are not natural original-game reachability evidence.

Two retained original-active API journals are also inspected using exact
hash-verified journal/interval metadata only. QueryInterface joins take effect
at successful original return and must precede the copy's entry; failed queries,
attached surfaces and future queries do not establish a copy alias. Unlinked
generations retain unknown backing relationships. No historical execution hash
is refreshed, and no full pixel capture is restored or reacquired.

The journals supply 2,060 and 1,752 source-copy calls respectively: 3,812 total,
zero same-generation pairs and zero successful prior-query alias pairs. All
3,812 retain unknown physical backing relationships. They are historical
original-driver observations, separate from the native eight-case CP06 census
and the four Grimoire domains; counts are not summed across those scopes.

[Thirty-three focused controls](rendering-alias-reachability-results-20261108/controls-01.json)
pass: eight new argument/timing/corruption guards and 25 existing temporal
alias/interval guards. A malformed initial test timeline was correctly refused;
its exact sources and negative output remain retained. The subsequent claim
bookkeeping omitted the original-build map after all controls passed; that
refusal is also retained, and its correction reuses the unchanged passing test
outputs. The first assessment's raw source phase remains historical after the
publisher/provenance additions in assessment02. No tested renderer or runtime
source changed, no original game was launched and no attempt cap was reset.

Closing the rows still requires a naturally invoked shared backing and
overlapping rectangle, the original flags/result and traversal history, and
complete independently checked native output and retirement in the required
profiles/domains. A supplied self-copy, syntactically compatible setter or a
finite absence count cannot substitute for that evidence. The next targeted
trace is publication of bar source field `+4` against its known destination
callers, plus the generic image's embedded source creation/lifetime; launch
only after identifying a concrete public action that can make those values
share storage. The accepted stretch and unrelated journey matrices need no rerun.
