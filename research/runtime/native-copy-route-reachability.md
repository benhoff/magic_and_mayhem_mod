# Application copy route reachability

Reviewed/executed 2026-10-17 UTC, supported NoCD build
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
The [fresh camera/HUD/Mini/Quit journey](native-copy-route-campaign-scoped-20261017.json)
passes. This narrows the copy gap: **keyed application copies execute, using
CPU-backed sources; keyed GPU-source plans remain unexercised live.** No renderer
pixel mismatch or unsupported copy was reproduced in the passing interval.

The former GPU-only census could count admitted GPU plans but could not explain
other copy calls. `NP.native-copy-route-census` now records a bounded scalar
snapshot before each active application Blt/BltFast: actual caller, canonical
tracker identities, generations, CPU/GPU source state, attached clipper, source
key, preparation route and actual HRESULT. It reads no pixels, makes no extra
COM queries and changes no graphics admission. Missing active observations or
unknown alias/clipper metadata invalidate branch-absence claims. See the
[wire contract](../formats/native-copy-routes-v1.md).

## Confirmed live routing

All **4,075 active calls** have complete metadata and successful HRESULTs:
2,380 CPU-prepared copies, 1,666 GPU-prepared copies and 29 fills. The GPU rows
have an exact bijection with the existing GPU admission census. Actual return
PCs below belong only to this pinned build; module base/extent are observed per
process, and no pointer stability across launches is assumed.

| Application return PC | Observed source/route | Calls |
| --- | --- | ---: |
| `0x0058c279` in whole keyed wrapper `0x0058c140` | CPU-backed, keyed | 916 |
| `0x0058c9af` in keyed rectangle wrapper `0x0058c8a0` | CPU-backed, keyed | 302 |
| `0x0058cbb2` in keyed rectangle wrapper `0x0058ca90` | CPU-backed, keyed | 610 |
| `0x0058c05d` in opaque whole wrapper `0x0058bf40` | GPU-backed, opaque; includes 3 bootstraps | 246 |
| `0x0058c488` in opaque rectangle wrapper `0x0058c360` | GPU-backed, opaque | 1,208 |
| `0x0058c5be` in opaque rectangle wrapper `0x0058c4a0` | GPU-backed, opaque | 212 |

Thus all **1,828 keyed calls** use the CPU-prepared path. No active call has an
attached destination clipper or aliases its tracked source/destination. These
are bounded observed facts, not proof that other game paths are dead or that
unknown physical/COM aliasing is globally solved. Shared wrapper return PCs do
not identify which higher-level direct or indirect caller invoked a wrapper.

The original camera switch's three selected native bodies all execute:
`camera0=41`, `camera1=133`, `camera2=39`; camera3 remains absent from that reviewed
dispatcher. All 213 World completions, imports and verified returns balance;
there is no pending work and normal retirement/normal game Quit/private Wine stop
pass. The six optional producer families record 4,143 native calls and zero
forwards/history refusal. Their other 24 unexercised routes remain open.

## Output proof and its boundary

The separate native-host observer checks **1,788 World-ancestry COPY commands /
447,772,800 complete destination words**, with zero mismatches against its
independent CPU transforms. All 1,666 admitted GPU-source calls match their exact
command ranges/resources. Sixteen retained source/before/actual/expected cohorts
receive an additional Python recomputation over 3,374,400 destination words.
Expected/captured pixels never become drawing inputs or CPU history.

**No keyed command belongs to this observer's World-ancestry selection.** The
1,828 CPU-backed keyed calls establish routing, not new independent keyed output
or original-driver equivalence. This result does not expand the diagnostic selector or independently compare
all application copy destinations.
Clipped/self/keyed GPU-source plans retain the separate fresh headless evidence.

There are 5,364 explicit diagnostic native readbacks / 5,786,611,200 transferred
bytes. Ordinary World/native/RGBA readbacks and producer CPU materializations are
zero. These test costs are separate; the llvmpipe run meets its declared 5 FPS
functional floor and makes no sustained hardware/performance claim. No new
unchanged-original World comparison or 545-frame portable rerun is claimed here;
those immutable earlier results retain their recorded source boundaries.

## Original caller and clipper findings

The [fresh static review](native-copy-route-static-20261017.json), backed by the
[unchanged full export](native-copy-route-static-raw-20261017.json), retains all
156 direct call candidates for the seven selected wrappers, including 55 with
unassigned discovered owners. Keyed candidates comprise 17 whole-copy, 19 keyed
rectangle/restore, one keyed rectangle and 11 explicit-source/destination-RECT
calls. These are candidates, not 156 executed scenarios; indirect reachability
and unknown owners remain unresolved.

The selected original creation block tests `0x006f68d8` at `0x0058b0f1` and skips
SetClipper when it is nonzero. When zero, `0x0058b106` calls Surface2 `+0x70`,
passing `0x006f98d0` to the selected surface. This is recorded separately as
`DRAW.surface-clipper-attachment`. Existing rectangle research identifies the
zero/nonzero windowed/fullscreen Blt/BltFast split. The live copy sites above
exercise the unclipped paths. Actual original windowed creation, clip-list/HWND
contents and mutation, and a genuine clipped GPU-source caller remain unexecuted.

The pinned earlier timer-cursor review shows distinct wrapper arguments:
`0x004a4afc` copies `0x00641fc8` to `0x006a4fe0`, `0x004a4c21` copies back, and
`0x004a4d2a` copies keyed `0x00642000` to `0x006a4fe0`. This supplies genuine
cursor-copy candidates and explains why save/restore need not be a self-copy.
Distinct wrapper addresses alone do not prove physical/interface non-aliasing;
that distinction comes only from the bounded runtime tracker observations.
No dispatch or game state was changed to manufacture a missing branch.

## Terminal boundary, controls and preserved attempts

The existing command lease intentionally bypasses observation after terminal
RenderStop, when owned tracker metadata may already be destroyed. The final
census records **seven such excluded callbacks**, with no active missed callback.
Their branch metadata/output remains unknown. They do not contribute to the
4,075 active rows or a complete-renderer claim. An active skipped callback, unknown
metadata, missing closure, capacity exhaustion or non-bijective GPU rows still
refuse proof. Retired-tail counter exhaustion also refuses without wrap.

[Final observer controls](native-copy-route-controls-20261017.json) pass the
65,536-row boundary, publication/counter exhaustion and active-miss versus retired
exclusion checks, ten decoder/caller/alias/clip/history negative controls, the
previous GPU census controls, actual GL incorrect-output/overwrite controls and
eight independently corrupted/incomplete pixel/caller cases.
[The fresh production facade/bridge controls](native-copy-route-plan-20261017.json)
pass all 23 clipped/self/keyed cases, 25 prepublication refusals, three publication
failures, legacy private baseline/CPU restore and command/resident compatibility.
These execute with observer mode disabled and retain their headless scope.

[Preserved attempts](native-copy-route-attempts-20261017.json) include the first
30-second camera journey: initial rotations pass, but resumed rotations after
Mini Cancel fail the required camera-route assertion. The shorter final ten-second phases did not close that longer scenario.
The [current-master follow-up](native-camera-resume.md) reproduces it with
active delivered input and identifies a blocking original Hermes modal. Its
bounded ordinary-input repair also handles the final-readiness/Escape race;
the historical failed acquisition remains unchanged. The second
short journey completes the GPU checks, then its initial census rejects a missed
callback without distinguishing terminal bypasses. The final census records
that boundary explicitly; it makes no retrospective claim about the old missed
callback's exact origin. A fresh-prefix Wine bootstrap also times out before
copy-plan execution; the final controls use a separately cloned, stopped owned
prefix. Intermediate positives receive no final-source credit. Six initial
uncommitted source versions are retained with exact hashes matching the old
acquisition inventories; no historical fingerprint was refreshed.

## Reproduction and next boundary

Use an isolated supported installation and matching launcher manifest, immutable
original-manifest bookends, and the main checkout's storage guard. The final
journey is:

```sh
python3 tools/test-native-campaign.py --command-wire-version 3 \
  --resident-world --profile-resident --capture-resident \
  --gpu-copy-diagnostics --other-pixel-producers --pixel-camera-scenarios \
  --stress-seconds 10 --normal-quit --difficulty 1 --min-fps 5 --timeout 600 \
  --skip-movies --legacy-audio --source-game working/source-game
```

The registered checkout is `working/worktrees/copy-reachability-20261017`;
its final campaign is `run-xrna9tap`, observer experiment `run-90vbp0t6`.
[Provenance](native-copy-route-provenance-20261017.json) pins all 80 actual
artifacts, including the 335,990,765-byte resident source capture, both scalar
censuses, staged executable/DLLs and 64 raw pixel planes. All 2,927 original
files verify before/after. The c3d6055f..1d53022e history review retains one commit,
one parent transition and zero missing exact receipts; it establishes accounting.

The [actual-game injected-failure and public Hard restart boundary](native-live-restart.md)
now passes both World owners after a confirmed terminal null-access repair. Clipped/self/keyed GPU-source callers, independent
CPU-keyed pixels, windowed/other-map reachability, complete action-bound presentation, physical loss and performance
remain separate. Complete replacement and its 14-journey/21-operation denominators
remain false.

The [complete raw acquisition](native-copy-route-campaign-20261017.json) retains
its source inventory unchanged. The scoped result excludes only blanket native
skirmish/game inventories that the original-engine shell neither compiles nor
executes; concurrent changes there receive no rendering validation credit.
