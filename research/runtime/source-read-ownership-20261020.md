# Private source-read ownership (2026-10-20)

`NR.source-read-ownership` is an opt-in native policy for the pinned Wine11.16
private single-subresource RGB565 boundary. `--wine-source-ownership` expands
its prerequisites; the runner admits only a complete physical-mode manifest.
`MNM_DRAW_WINE_SOURCE_OWNERSHIP=1` is strict; absence preserves diagnostics.
Other values, missing output/guards, incomplete installed hooks, skipped source
maps and COPY/worker calls after shutdown refuse before forwarding.

Original source pixels freeze only during the callout-free value copy. The
private staging allocation seals read-only before Wine sees it, stays read-only
after the exact worker/current-context fence releases its lease, and frees only
after original binding restoration. Resource and alias mutation, stage rebinding
and explicit staging free or VM mutation remain excluded throughout the binding lifetime. Texture
retirement publishes exclusion before its extra getter, then checks resolved
aliases or conservatively refuses an unresolved result. Disjoint retirement and
retirement after restoration remain allowed.

Negative-first owned PE32 experiments demonstrated four forwarded unsafe
retirements (exit 211) and three permitted staging write/protection changes
(exit 0). These are synthetic native-policy findings, not original game runs.
The original diagnostic reservation/version contracts do not provide this
combined ownership guarantee. The strict admission marker must join physical
handoff/fence proof; configuration flags alone cannot claim closure.

Shared/imported/drawable/DC/BO/unknown memory refuses. Arbitrary host/kernel or
direct-syscall interference, external direct GPU dispatch, unknown driver
aliases, original allocator/scheduler equivalence, global presentation, complete
gameplay and live replacement remain outside this finite policy. Current-code
physical original integration remains a separate milestone.

Current owned execution passes 44 strict/handoff/window/admission cases,
36 physical controls and 11 parser/typed-manifest tests. The native regression
suite passes 11 CTests and two ASan/UBSan CTests, 150 transform cases/17,550
pixels, trapped original row-body continuation controls, 91 SPR and 195 scene
frames, six guard suites, 59 offline controls and the actual V5/point/combined
surface backend checks. Qt/Mesa leak detection remains excluded; owned GPU
cleanup assertions run. Raw artifacts remain under the registered worktree
`working/worktrees/ownership-completion-20261020/working/tests/` and its pinned
`working/ownership-prototype/`.

The earlier retained attempts include an obsolete refusal expectation, an
explicitly superseded source tree during master reconciliation, and a private
fixture lock refusal. None supplies final success credit. A serial repeat
provides the final broader native result.
