# Legacy DirectDraw dependency validation, 2026-10-20

The missing DIB Engine dependency is resolved for this probe, but legacy
DirectDraw admission remains blocked on Wine 11.16. Supplying the DLL is not
sufficient to reach `DirectDrawCreate`, surface creation, Lock, Restore or Blt.
Confidence is high for the measured loader outcomes; the exact underlying
Win16 compatibility defect and genuine Windows 98 driver outcomes remain unknown.

## Controlled execution

| Input | Observed native NE modules | LoadLibrary result | Surface available |
| --- | --- | --- | --- |
| Supplied DirectX 6, no DIBENG | Neither DDRAW16 nor DIBENG | NULL, error 1114 | No |
| Same runtime plus archived DIBENG | DDRAW16.DLL and DIBENG.DLL | NULL, error 998; probe-thread access violation `c0000005` | No |

Both final runs complete their owned probe and preserve the original manifest
before and after. Both private display servers remain alive after the probe.
Report `success: true` means the diagnostic completed, while
`legacy_surface_available: false` records the failed admission. No original game
wrapper executes and no native fallback, scripted HRESULT or original instruction
patch supplies a successful result.

The dependency is Windows 98 DIB Engine version 4.10.0.1998, 202,800 bytes,
SHA-256 `b7c0b676cd8c9960da4c53deae8de846d1251e9534ef4e4b2f05c988306dd2bd`.
Two independent extractors produced identical bytes from archived `WIN98_27.CAB`.
The archived file's publisher authenticity is not established. Its NE import
table names only KERNEL; supplied DDRAW16 imports KERNEL, TOOLHELP, DIBENG, USER
and VER. Download-time provenance is preserved unchanged; its original
unexecuted/uninstalled fields describe that earlier download stage.

The supplied CD archive remains SHA-256
`5f29f38abc8a9cd3ebb90812f377067d401565fdf044e82e5923f6490608839b`.
The runner verifies it, extracts DDRAW.DLL, DDRAW16.DLL, DDRAW.VXD and DDHELP.EXE,
and accepts the optional dependency only at the hash above. Native-only overrides
forbid builtin ddraw/ddraw16/dibeng fallback. A private Wine prefix has the probe's
Windows 98 compatibility setting. The original game image remains read-only;
the owned executable uses the existing loader/Create/offscreen admission fixture.

The outer runner has a private network and PID namespace. Wine's nested
filesystem sandbox permits writes only to the owned output, prefix and the
output's private temporary directory mounted as `/tmp`. The controlled manifest
checker can create its required verification temporary file beside the manifest;
Wine cannot write to research or original artifacts. Nothing is installed into a
shared game prefix or host system directory.

## Reproduction and evidence

The exact outer sandbox commands are retained with the evidence. Invoke them
through `tools/working-retention.py run --reserve-gib .8 -- ...` for a new prefix,
or reserve .15 GiB for new bounded outputs with the existing owned prefix. Use
new output and launch-report paths; retain the historical results. The runner
accepts `--dibeng <path>` for the hash-pinned dependency and omitting it performs
the matching control. Its source hash and prospective behavior declarations are
inside each report before execution.

- [Dependency-assisted report](legacy-ddraw-dibeng-results-20261020/with-dibeng-final-report.json)
  and [loader/exception log](legacy-ddraw-dibeng-results-20261020/with-dibeng-final-legacy-execute.log).
- [Missing-dependency control](legacy-ddraw-dibeng-results-20261020/without-dibeng-control-report.json).
- [Confined launch command](legacy-ddraw-dibeng-results-20261020/launch-with-dibeng-final.json)
  and [download provenance](legacy-ddraw-dibeng-results-20261020/download-provenance.json).
- Raw attempts, build outputs, failure logs and the owned prefix are pinned at
  `working/tests/legacy-ddraw-dibeng-20261020/`.

Earlier attempts remain retained. Attempt 1 stopped before original consumption
because the manifest verifier could not create a temporary file on read-only
research. Attempt 2 verified originals before/after but Wine could not create
its private socket directory. Attempt 3 loaded both modules but its X server
crashed inside the host NVIDIA EGL path; it is not the clean display control.
The subsequent runner disables the unused X GLX extension and checks server
liveness. Attempt 4 and the final dependency run reproduce error 998 with a
surviving server. The final no-dependency control reproduces error 1114 using the
same final runner and compatibility settings.

## Remaining boundary

The missing-file condition is no longer the remaining blocker. The observed
blocker is a fault during native legacy runtime initialization on this Wine
configuration. No evidence here identifies which module instruction or Wine
implementation defect causes the fault, or proves every Wine configuration
would fail. Further admission requires investigation of that Win16 initialization
fault or a compatible legacy Windows/display-driver environment. Until admission
succeeds, the actual legacy-driver loss/failure/recovery comparisons remain
unvalidated. All five broad recovery parents and their other ownership,
reachability, composition and live-replacement gaps remain open.

The first-parent rendering integration since the previous REC8 review was also
checked against exact active/archived receipts: 323 committed file transitions,
zero missing receipts. See the separate
[committed-history report](coverage/legacy-dibeng-incoming-history-20261020.json).
That review does not confer new engine validation or describe the dirty tree.


## Current validation freshness

The REC9 diagnostic reports match the current probe sources. The normal coverage
audit passes with zero errors. The integrated rendering sources separately make
older REC6–REC8 comparison fingerprints stale; their original reports and receipts
remain unchanged. A full change-gate check still rejects the earlier workstream's
recorded-current receipts and scoped status promotions when those stale results
are used against the new master. This retry does not renew them or assert a full
master gate pass. Their separate comparison reruns remain pending.

The dashboard's old process also held Python modules from before the rendering
merge, causing Refresh to reject the newly reviewed Low-priority cohort. A fresh
process builds the current snapshot successfully. The existing importance unit
test incorrectly expected project-wide counts from its one-behavior fixture;
its assertion is restored to that fixture's single Critical item without
changing production ratings or contract statuses.
