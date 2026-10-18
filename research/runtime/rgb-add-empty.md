# RGB-add empty horizontal clipping

Pinned NoCD original `0x545c10` tests vertical clipping before the production
splice `0x545ce9`. With positive prepared rows, signed right <= skipped-left
skips the pixel body. The tail leaves ECX zero, EDX skipped-left and signed right
in EAX before its AX-only clear. Empty work reads neither RGB fields nor pixels.
Confidence is high for the compared branch, not the whole function.

The [current original comparison](rgb-add-empty-default-original-20261018.json)
passes **512 native cases**: 256 missing-selector/default cases and 256 explicit
`MNM_PIXEL_EMPTY_RGB_NATIVE=1` cases, both RGB565/555 and DF states. All 1,064,960
physical WORDs match. Pixel backing is inaccessible; 256 source descriptors
expose only eight width/height bytes before a guard page. GPR/flags/FX, LastError
and producer counters match. No admitted empty call requests operation-0 pixel
history. Eight explicit `0` controls forward once, each preserving its history
request and callback LastError 1234 despite selector lookup error 203. The
positive pixel suite independently passes 5,028 native admissions.

Native admission is now the default for this bounded, original-compared branch.
Missing/empty selector or explicit `1` enables it; explicit `0` or other nonempty
values retain fallback. `test-raster-cpu-game.py --disable-empty-rgb` selects the
opt-out. Owner, extent and prepared-geometry checks remain required. The resolver
uses adapter scratch word9 to retain the original final registers. It reads no
RGB source or destination pixels, including for guarded eight-byte descriptors.
This is a native history/admission policy, not an original game API.

The [read-only caller observer](../formats/raster-rgb-callers-v1.md) samples
prepared scalar stack geometry at `0x545ce9`, in a fourth hardware breakpoint.
It neither edits inputs nor does file I/O inside game callbacks. Six real PE32
controls and seventeen decoder/selection tests pass. Synthetic RET-stub calls
receive no natural caller credit.

A successful explicit-enable original-game journey observed one eligible empty
call (sequence6, top0, bottom2, right2, skip2), matching its native RGB epoch with
zero forwards and normal Quit. That report predates the default-selector edit;
its source hashes remain historical. Current-source default journeys pass strict
acceptance in both CPU contexts but observed **zero empty calls**. Their traces
match 1,776 authored-CPU RGB calls and 708 physical-host RGB calls respectively.
The [accepted alternate record](rgb-empty-default-alternate-accepted-20261018.json)
and [physical record](rgb-empty-default-physical-accepted-20261018.json) explicitly
exclude unfulfilled empty-live claims. `RS.rgb-add-empty` and
`NP.rgb-empty-default-admission` retain integration **none**. Configuration and
isolated comparison do not establish current-code live execution of the branch.

The [acquisition catalog](producer-rest-acquisition-20261018.json) preserves all
attempts, including earlier setup failures, source changes, presentation stops
and camera-stress refusal. Two earlier RGB forwards remain unassigned; no empty
trace from them exists. Earlier [256-case evidence](rgb-add-empty-scoped-original-20261017.json)
and its [shared-source rerun](rgb-add-empty-followup-scoped-original-20261017.json)
retain their original default-off scopes and hashes. No campaign threshold,
43-route denominator, original GPU-history claim or gameplay policy changed.

## Finite-raster source renewal (2026-10-18)

The [current original renewal](finite-cpu-displacement-renewed-original-20261018.json) prospectively reruns512 default/explicit native empty cases,256 guarded descriptors,eight explicit opt-outs and5,028 positive pixel cases. Required contract sources and the actually executed compiler closure remain in the focused record; the immutable raw superset is retained. The [renewed caller witness](finite-cpu-displacement-renewed-game-20261018.json) observes1,776 RGB calls and zero empty calls with normal Quit/zero forwards/history refusal. It renews caller observation, not current empty-branch live execution. No historical source hash is refreshed.
