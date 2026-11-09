# Stretch and alias/overlap COPY readiness — 2026-11-08

Neither `operation.copy-stretched` nor `operation.copy-alias-overlap` can be
closed from the retained CP06 evidence. Both profile rows for each requirement
remain pending. The accepted parent remains **112/156 passed, 44 pending**;
both final rendering decisions and every existing attachment remain unchanged.
This investigation adds no game run, game fixture, native implementation change,
original-driver comparison or acceptance promotion.

The [SQLite assessment](rendering-two-copy-results-20261108/assessment-03.json)
redecodes the original version2 route streams and checks their exact GPU census
bijections in all eight retained Quick/Campaign cases: default/software,
actual640×480/800×600. Acquisition/comparison hashes and raw artifact bytes are
verified through their immutable records. Historical native/compiler source
phases are preserved; this is current analysis of retained captures.

| Observation | Calls |
| --- | ---: |
| Closed active application denominator | 37,337 |
| Source-less calls, excluded from COPY geometry | 467 |
| Prepared COPY geometry | 36,866 |
| Prepared stretched COPY | 0 |
| Tracked same-backing COPY, including observed aliases | 0 |
| Refused calls with unknown geometry | 4 |

Prepared geometry is one-to-one in this bridge's admitted domain. Consequently,
zero prepared stretch calls cannot prove that stretching is globally unused.
The four refused geometries are not classified as unscaled: early preparation
refusals can leave default or partial rectangles. Tracked identity is also
separate from arbitrary physical/COM alias discovery. None of these counts is
an independent pixel comparison or proof that a positive branch is unreachable.
The four unknown records are the two preserved Low storage refusals in each
profile: original return PCs `0x58c488` and `0x58cbb2`, both HRESULT
`0x88760096`. Their existing invalid-rectangle failures remain unresolved.

## Proof gaps and bounded next work

`runtime/render/lock_surfaces.h::game_blit_before` refuses ordinary CPU COPY planning at
`blit_reject_self` and unequal source/destination dimensions at
`blit_reject_stretch`. `NativeDrawGpuCopyPlanV1` refuses unequal dimensions;
`NativeDrawCpuCopyPlanV1` also admits only one-to-one COPY geometry. The CPU
surface facade in `runtime/native-draw/surfaces.h::ns_draw_route` has stretching
and whole-alias snapshots. Importantly, `game_blit_after` can fall through to
`runtime/render/owned_session.h::game_native_blit_checkpoint`, which publishes
successful native CPU-transform output through CREATE/UPDATE. Thus the COPY
plan refusals do **not** establish missing rendering support. Those checkpoint
observations still need caller-bound independent transform/output proof;
uploaded producer output alone cannot supply it. Existing backend overlap
corpora retain their separate original Wine/control scope.

The retained hash-checked original wrapper review identifies eleven direct
call instructions targeting explicit-RECT wrapper `0x58cbc0`. Ten are in the
discovered owner `0x4c6030`, one in `0x4c3a40`; all exact instruction contexts
are retained in the assessment. Independently forwarded rectangle dimensions
make these candidates for tracing. They are not eleven natural stretch
witnesses or exhaustive indirect reachability. Separate existing
[Grimoire research](grimoire-engine-contract.md) identifies `0x4c3a40` as the
book initializer. Book initialization/page transitions are therefore a specific
follow-up lead, without inferring that an unequal rectangle actually executed.

The smallest next acquisition is an actual required caller/argument observation
on those explicit-RECT paths, before selecting any complete pixel capture.
An alias acquisition must establish a real shared backing/lifetime and
overlapping arguments; distinct cursor save/restore surfaces do not qualify.
After a positive required path is identified, inspect its ordinary COPY or CPU
checkpoint route, then check its actual flags/result, source and prior destination authority,
complete independent output and retirement in both required profiles/domains.
If a precise supported-domain exclusion is proposed instead, it needs a separate
requiredness review; these eight bounded zero counts do not supply one.

No accepted journey or whole rendering matrix needs to be repeated for this
accounting result. All existing evidence and pending source boundaries remain.

Nine new classifier controls cover alias-versus-coordinate identity, overlap
edges, translated versus stretched geometry, signed32/wide extents, nonpositive
geometry, unknown refusals, source-less fills, wire version, and refusal to turn
positive metadata into output/acceptance credit. Ten existing route-integrity
controls pass unchanged. The initial direct invocation of the non-executable
storage-guard script returned126 before any assessment; explicit `python3`
invocation succeeded. These results are retained in the controls receipt.
The final assessment removes an unnecessary dynamic import of the full CP06
closeout tool and reads the same retained streams through the existing strict
decoder directly. Assessment01, its exact hash-verified analyzer preimage and
the original controls receipt are preserved; assessment02 has the final source
phase and identical counts. Assessment03 corrects the earlier overly broad
claim about production support by reviewing the complete checkpoint call path;
it retains the same census counts and does not assert a new runtime result.
Assessment02's exact source is preserved in commit `96e11d042`; assessment01's
preimage remains in SQLite. Superseded static-source staleness is explicit and
does not reopen any accepted engine milestone.

The first commit snapshot stopped with ENOSPC while materializing its Git base
under `/tmp`; the failed log/source phase is retained as
`finish-refusal-01.json`. This setup refusal precedes commit review and earns
no validation credit. The retry uses task-specific `/dev/shm` scratch with
checked byte and inode headroom. No accepted rendering check is repeated.
