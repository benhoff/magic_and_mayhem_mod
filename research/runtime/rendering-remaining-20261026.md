# Remaining native rendering gaps — 2026-10-26

This batch builds on8c0c4ea4 in an owned worktree. It changes native packed
colour composition and offline World admission, plus bounded validation tools.
It makes no web UI layout or gameplay balance change.

## What is now proved

| Gap | Result | Exact boundary |
| --- | --- | --- |
| Natural colour modes2/3 | Eight later-game map1/fifteen-item World samples contain719 mode2,1183 mode3 and104 mode1 colour draws | All2006 observed shapes are2x2; original simulation and drawing remain active |
| Native rendering of those requests |14852 ordered draws/3840000pixels match independent unchanged original replay from the same owned-zero base | All eight original-before diagnostic replays also match live; native-zero/live retains one difference per frame |
| RGB555 World input |128 authored cases across RGB555/RGB565, cached/uncached,31587776pixel comparisons per backend;640 refusal checks and9024cache hits | Explicit offline colour-only input and `.555` preview; software Mesa and direct NVIDIA EGL both pass |
| Large RGB565 fills and colour blends on hardware |192 ordered World fixtures/38199120pixels and five refusals match unchanged original547d80 | Authored requests, not naturally observed large fills or physical presentation |
| Shared arithmetic/default formats |4352 CPU pairs plus384 boundaries;90011152physical WORDs and caller state match | Four modes, RGB565/RGB555, isolated original preparation/footer; no CPU splice live replacement |
| Retained busy regression |16 frames/19036draws/7680000pixels match original-zero, original-before/live and native-zero/live | Historical inputs freshly replayed on changed current sources; software headless |

Current sources and prospective contracts bind each new result. Historical
reports and source hashes remain unchanged. See
[natural-mode classification](rendering-remaining-results-20261026/natural-colour-index-02.json),
[natural-frame comparison](rendering-remaining-results-20261026/natural-late-comparison-02.json),
[software RGB555](rendering-remaining-results-20261026/rgb555-software-03.json),
[NVIDIA RGB555](rendering-remaining-results-20261026/rgb555-hardware-01.json),
[CPU and NVIDIA colour/fill comparison](rendering-remaining-results-20261026/colour-hardware-02.json)
and[retained busy comparison](rendering-remaining-results-20261026/busy-comparison-01.json).
NVIDIA reports identify TITAN RTX, vendor NVIDIA Corporation, OpenGL3.3 driver
610.57.04; software fallback is refused. These contexts have no window or swapchain.

## The new live-history boundary

Every late sample has exactly one native-zero/live difference at(548,433):
native0,live6338,original-before6372. Native and original both starting at zero
agree completely; original execution initialized from the diagnostic before
plane reproduces the live plane completely. Native never reads that plane.

The[first-frame original-only watch](rendering-remaining-results-20261026/natural-history-difference-01.json)
observes record1236/backend2 terrain copying6372to0 at that pixel, followed by
record1261/backend9 wave/distortion writing6338. Thus the trace propagates
prior destination history from another sample location. This identifies the
selected propagation, not the earlier source writer or complete history.
The late-frame test is accepted for bounded same-base original comparison,
not complete native/live parity. A progress statement claiming all three
comparisons matched was corrected after reading this separate result flag.

Recovering the owned producer history before the selected late interval is the
next requirement for native/live equality. Supplying captured original pixels
to native replay would not close that requirement.

## Implementation and scope

The default World decoder and existing live/producer transports remain
RGB565-only. The offline preview's explicit `--rgb555-colour` policy admits
header format1 only with operation7/backend12 and canonical fifteen-bit owned
sources. Unknown formats, sprites, additive rectangles/effects and high-bit
sources refuse. Late invalid draws preserve the previous completed scene.
Scene and colour-cache storage use RGB555 directly; the integer shader selects
mask0x3def for its half/quarter blends and retains0x7bef for RGB565. Cached and
uncached complete output WORDs, unused destination bits and preview exports
are independently compared with the unchanged original colour routine.

The original capture controller accepts an explicit60..180second finite
original-active observation budget. Defaults and replacement paths retain
their prior bounds. The successful late profile requests eight real samples
after600queue calls at eight-call intervals with180seconds; a larger budget
creates no samples and bypasses no source, manifest or success checks.

Static hash-checked[selected constructor/update review](rendering-remaining-results-20261026/colour-mode-static-02.json)
records initial mode2 stores and the gated548860 update's mode write at548a0d.
Its selected formula uses remaining lifetime and global5e1428. The54ab60 draw
wrapper forwards shape+1c to547d80. These selected instructions do not recover
the whole update, its providers, faults or object lifetime. Natural reachability
is established by the separate actual captures, not inferred from these stores.
The existing function inventory omits initializer547950. Eighteen separate
unchanged-original initializer cases across both formats validate its selected
mode2/extents/channel/null-plane/return contract and pin entry bytes for the
explicit547950..54798a range exception. The inventory gap and provisional
whole-update understanding remain; no native initializer is claimed.

## Remaining acceptance gaps

Large fills during active gameplay remain unobserved. The eight successful
World samples contain2x2 colour primitives only; this wire does not census
generic Surface fill calls. Large-fill arithmetic and ordering are covered by
authored fixtures, including hardware, without claiming natural invocation.
RGB555 sprites/effects, actual game RGB555 production, live format1 ingestion,
physical presentation, sustained hardware performance, normal Quit/journeys,
all-writer ownership and whole-renderer/live replacement remain unaccepted.

## Failures, accounting and retention

All setup/negative attempts remain retained: two pre-menu failures without the
complete private display/audio fixture; a successful-startup60second late probe
with no complete selected samples; the outside-namespace static manifest
refusal; a wrong CMake project selection; a strict reference compiler warning;
and the first natural comparison with its retired game directory absent.
The completed game's exact environment receipt was restored before its asset
catalogue was consumed. The strict resource binding rule was not weakened. The acquisition reused the
earlier natural-busy scenario declaration; that120/four-call profile receives no
acceptance from the600/eight-call acquisition. Natural counts are registered as
static classification of actual execution, with no native CPU implementation
or integration credit. The separate same-base native comparison prospectively
binds the correct new eight-frame headless scenario.
The first regression invocation omitted required output prefixes; its result
remains failed separately from the corrected batch.

Generated reports and raw execution provenance use SQLite references. Private
RAM holds temporary builds/captures; exact evidence, unique inputs, failed
attempts and verified local backups must be durable before RAM teardown.
The user explicitly allowed a temporary88GiB main-drive floor for this batch;
the default90GiB and secondary100GiB policies are unchanged. No source push,
evidence publication or private remote transfer is part of this work.

The[final full audit](rendering-remaining-results-20261026/audit-03.json) has
zero errors and indexes4609sources. Its inherited historical staleness and
unknown functions remain explicit. The[six current comparison fingerprints](rendering-remaining-results-20261026/final-source-contract-check-01.json)
match current source and behavior/scenario contracts exactly. Ten shared
renderer regressions pass. A stable481-object/129175552byte metadata input
store avoids concurrent reader locks; its exact payloads and catalog have
verified backups/restores in both configured local snapshot catalogs. This is
accounting/storage provenance, not additional renderer acceptance.


### Rendering integration with current master — 2026-10-27

Combine rendering `cb24376e` with current master `432d8b36`. Rendering code
has no overlapping implementation edits; merge the coverage register, receipt
journal and ledger without changing historical evidence hashes. Fresh
`RRG27.merge-rgb555-*` controls recompile the merged code and compare 128
colour-only RGB555/RGB565 World fixtures against unchanged original `547d80`:
31,587,776 pixel comparisons per software/NVIDIA backend, 640 refusals each,
cache controls, public offline preview and no leaked surfaces. Original manifests
pass before and after both runs. Raw inputs, outputs, binaries and logs survive
a complete SQLite round trip.

Intervening surface-wrapper and capture-producer edits change provenance in the
six older RRG26 comparison reports. Retain them as historical; the fresh colour
policy runs do not renew the complete CPU/large-fill or natural/busy capture
claims. The archived one-pixel native/live history difference, natural large-fill
reachability, live RGB555, RGB555 sprites/effects, physical presentation and live
replacement remain open. `merge-source-contract-check-20261027-01.json` and
`merge-audit-20261027-03.json` record the combined-tree boundaries. The first
two audits retain archival-provenance schema errors as negative evidence. Final
catalog03 records actual producer fingerprints copied from the fresh execution
declarations; storage evidence grants no additional engine milestone.

Use pinned, hash-verified materialized local SQLite backups for this closeout.
The cold archive remains below its unchanged100GiB reserve; this merge makes no
cold write or publication and removes no evidence. Preserve all unrelated main
edits through the exact owned reservation/restoration transaction.
