# Tranche 3 issue work — 2026-10-19

The requested three workstreams were executed: finite producer handoff,
storage-parity diagnosis, and DirectDraw compatibility reconciliation. The finite
handoff passes. The facade's bounded comparisons pass after repairing a stale
mode-enumeration fixture. The unmodified storage reference still fails; the
diagnosis does not convert an expected negative into parity acceptance.

Results are retained in `rendering-tranche3-results-20261019/` and summarized in
`rendering-tranche3-issues-execution-20261019.json`. Fresh evidence has new IDs;
older reports, claims and hashes are unchanged. Confidence is high for the
measured results and supplied services below. Full retirement, unknown routes,
physical driver internals and broader live replacement remain unresolved.

## Finite handoff

Current sources were frozen and built using `test-world-producer-handoff.py` with
prospective claims and the closed tranche-2 original-active producer capture.
All **1,061 checkpoints / 474,235,312 pixels** match their separate original
oracles. All **16 World completions / 7,680,000 pixels** match independent
unchanged-original raster replay. Native input contains owned producer requests
and assets, never original destination pixels. The four native handoff/history
controls pass, with stable sources/inputs and original-manifest bookends.

This renews the selected headless finite integration only. It executes no new
game capture or live bypass; zero bypass requests were selected. It does not
validate continuous delivery, native-only launch, hard restart, hardware/FPS or
whole-game replacement. Implementation and replacement stages remain unchanged.

## Storage parity and its diagnostic limit

The new isolated drawing build compares every required frame at the unchanged
one-unit channel tolerance, excluding alpha:

| Reference profile | Frames | Combined GPU/physical pixel differences | Result |
| --- | ---: | ---: | --- |
| Unmodified storage | 238 | 252 | FAIL: ordinal 170, 126 in each cohort |
| Explicit corrected-driver storage | 222 | 0 | PASS, including 222 source/default/physical witness sets |
| Quick Battle | 323 | 0 | PASS |

Every physical frame retains same-image ordinal admission and paint-before-swap
receipts. No frame was dropped, renamed, translated for acceptance or removed
from the denominator. This is software replay; it does not renew the full
hardware suite. The hardware task retains the same reference negative as a
separate acceptance limitation, not a second diagnosed native defect.

The first downsized baseline frame changes from 800x600 to 640x480. Its original
RGB image is entirely black; the native image contains 126 nonblack cursor
pixels. A comparator-only search finds that every downward translation from 84
through 120 pixels would hide those pixels and match that black image. Therefore
this baseline alone cannot identify a unique 120-pixel translation or a specific
Wine/Mesa internal cause. None of these diagnostic transformations enters native
rendering or its acceptance result.

Independent retained source/default-framebuffer observations place the older
discrepancy after correct pre-swap pixels. The fresh corrected profile has one
explicit crop packet at ordinal 160 with structured source/image/window identity;
its different ordinal is run-local. Fresh actual owned XCB controls reproduce
307,196 incorrect pixels with stale upload geometry and zero with the scoped
correction. Five upload cases and six unsupported-route refusals pass. Seven
owned EGL modes across seven resize frames pass, including 35 destination-plane
checks, framebuffer/read-buffer/viewport restoration and unchanged library bytes.

These observations support the existing opt-in stale-image policy. They do not
prove universal driver behavior, missing native raster arithmetic or parity with
the unchanged failing reference. Full unmodified-reference parity remains FAIL.

## DirectDraw service crosswalk

The old pending-descriptor and blanket palette/clipping TODOs are stale. The
bounded facade and narrower services already supply these mechanisms:

| Domain | Supplier / current boundary | Evidence renewed here |
| --- | --- | --- |
| Factory identity and independent references | `factory.c`; Draw1/2/4 and canonical aliases | 128 lifetimes, 2,176 compared caller fields, zero final owners; native ddraw disabled/module absent |
| Offscreen descriptors, pitch, pixels and default placement | `surfaces.h`; 108/124-byte descriptions, selected 510x180 policies and guarded caps/formats | 64 original/native cases, 2,048 caller fields and 37,440 pixel words; default/refusal policies retained |
| DC/clipper ownership and selected primary/flip policy | `dc.h`, `clipper.h`, `surfaces.h` | 64 original/native cases, 2,048 caller fields and 22,464 pixel words; 16 startup callers / 256 fields; three terminal unknown-method refusals |
| Five-format storage, palettes, aliases and current regions | `NP.native-resource-state` | 408 retained independent clipping cases / 78,336 word checks, two partial-error cases, 80 format cases, 12,288 palette-word checks and six original/native HWND-region cases; zero final owners |
| Ordered GPU sources and transformed copies | `NP.native-gpu-copy-plan`, `NP.native-copy-transforms`, `NP.owned-dc-copy` | Supplied narrower contracts; this change does not renew every route or claim universal original conversion equivalence |
| Unknown methods and unsupported domains | Explicit HRESULT or terminal family/interface/slot refusal | Refusal is implemented; absence from one journey does not prove unknown routes unnecessary |
| Physical driver loss, complete route/writer/alias coverage, semantic retirement and sustained performance | Outside the bounded service proofs; aggregate recovery/acceptance remains open | No full-retirement, physical-driver, restart, 19/60FPS or arbitrary-scheduling promotion |

The aggregate therefore remains **Unresolved aggregate**, with supplied and
guarded domains enumerated. No newly reproduced required missing COM branch was
identified. A future implementation task must name a required encountered request
and its independent failing fixture; neither the old viewport-harness failure nor
the stale enumeration test identifies one.

## Validation fixture repair and retained setup failures

The services fixture failed at exit 196 because an unfiltered native enumeration
was still expected to return only two RGB565 modes. Current native policy offers
two sizes for each of RGB565, indexed8, BGR24 and BGR32. The fixture now requests
RGB565 explicitly and separately checks all eight modes, cancellation and a
width/height filter: 13 callback observations in total. The runner checks an exact
sidecar receipt. This changes validation code, not native supported formats or
the unchanged original DC/clipper caller matrix.

The EGL destination runner now accepts an isolated observer library and verifies
that its bytes remain unchanged through execution. All seven actual modes passed
with that option. It avoids replacing the shared default build during this work.

Two early facade attempts were refused by the global Wine reservation. Verified
three-hour-old orphan test prefixes, an abandoned WineDbg fixture and their
orphan displays were stopped gracefully after their process identities and logs
were retained; no files were deleted. A further transient reservation refusal and
the actual exit-196 fixture failure remain recorded. Later passes do not erase
those attempts. Initial handoff/storage and factory/surface reports retain their
earlier broad source hashes after the services-fixture-only edit; their changed,
uncompiled fixture/runner inputs remain an explicit freshness limitation rather
than refreshed evidence. The final finite handoff is executed again against its
actual dependency closure and the final source snapshot before registering its
headless integration milestone.

The initial drawing CTest command found no tests at the top-level build
directory. The corrected command uses `composition/`, requires a nonempty test
set and records all five passing scene/session/history controls. Sixty-seven
existing Python component methods also pass. These checks do not grant full
storage parity or whole-renderer acceptance.

## Storage and accounting

The user explicitly authorized going below the former 100 GiB floor for this
work. The existing 90 GiB floor remains enforced by the main checkout's retention
guard, with bounded output estimates. No storage deletion or archive was performed.
Referenced output is retained and pinned. All 2,927 immutable original files pass
the final verification; original-consuming runners retain their own bookends.

The reviewed HEAD remains `0dc91365aed634b16c61f31cc8f34b2746eee62a`; the prior
committed-history review still applies. Coverage audit, source census, exact
receipts and local-gate limitations are retained under
`working/tests/rendering-tranche3-issues-20261019/`. New finite integration
evidence is separate from implementation completeness and live replacement.
