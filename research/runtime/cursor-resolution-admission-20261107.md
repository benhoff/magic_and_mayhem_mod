# Cursor resolution admission — 2026-11-07

The retained CP06 default/software low-resolution Campaign captures each contain
a failed save-under rectangle `(640,512)..(680,552)` from a 640x480 display and a
failed keyed cursor copy at `(640,512)` into that display. The original low-level
error remains correct: those rectangles leave the owned storage. No historical
capture, result or source hash is rewritten.

This batch reproduces a mismatch between the original cursor client's nominal
resolution and the live display surface. The original foreground body chooses
its clipping bounds from the unaligned original resolution flag at `0x6de6d5`.
The retained captures do not include that flag at the failed call, so exact
historical cause attribution remains separate from a controlled reproduction.

The unchanged original foreground body emits exactly those two unsafe requests
when the flag selects 800x600 and the surface is 640x480. The same pointer and
surface with the low flag emit no copy. Its full 849-byte body remains unchanged
in this controlled original execution. This establishes a sufficient mechanism,
not the unobserved flag value in each old capture.

## Native correction

The owning adapter checks the actual retained target backing before entering
the foreground or timer. If it cannot contain the original nominal clip extent,
the client waits for compatible backing. A larger desktop primary remains valid.
The saved-underlay flag is cleared and saved position is set to an explicit
repaint sentinel, so a stationary single-frame cursor retries after settling.

Inside the protected cursor paint/restore scope, actual source/destination bounds
are checked again before an original immediate copy or deferred publication.
This covers a mode change after entry admission and stale restore coordinates.
An invalid operation aborts the remaining cursor copies, releases temporary pins
and invalidates the underlay/repaint position on return. A failed save cannot be
followed by a visible sprite without a usable background. No unsafe job becomes
active. Already accepted valid jobs retain their existing ordering and ownership.

The policy applies to foreground, timer and back/front restore scopes. Other
copy callers retain the strict result and geometry policy, including
`0x88760096` for the exact invalid destination. No original file, general API
bound, pixel tolerance, wire layout or gameplay rule changes. Native status
word 26 (the existing counter slot 14) counts cursor geometry/mode refusals;
the lifetime journal layout and existing counters retain their meanings.

## Validation and evidence boundary

`before-03.json` records the pre-fix production owner failing the first independent
complete-plane check (exit 207): it paints despite nominal800/live640 mismatch.
Its exact native binary, test preimage and logs remain immutable. The expanded
current `after-02.json` passes four profiles: plain/protected backing and both
startup mutex orders. Each passes 27 complete independently constructed RGB565
planes (108 total), 8,640,097 assertions, balanced queued jobs and zero owners.
All 18 changed-anchor refusals also pass. Cases include the exact historical
off-screen geometry, stale immediate and deferred restores, stationary timer
retry, a controlled mode flip after admission, negative/right/bottom clipping,
resumed 800x600 output and unchanged general invalid-copy refusal.

The existing original-owner suite also passes all four profiles: 24 complete
planes in total, retained generations after wrapper reuse, busy writers, 128-slot
saturation, cancellation and Stop. Optimized and ASan/UBSan recovered-client
comparisons each pass 10,368 client, 1,280 queue, 12 disposal and six retry cases.
The test-library guard changes no normal client model or original instruction.

Preparation failures are retained separately: the first reference selected Blt
instead of the captured BltFast route; the next declaration omitted three native
compiler inputs; an old client runner required an absolute output path. The
initial corrected 12-plane controls remain historical after extending the test.
These are not engine failures or successful games. All original consumers verify
the immutable-media manifest before and after. Reports, binaries, exact source
preimages and raw diagnostics are SQLite references under
`cursor-resolution-results-20261107/`.

`NP.cursor-resolution-admission` gains scoped implementation and headless native
integration only. No game entrypoint or physical driver is exercised by this
batch. Current original-game display-transition execution, exact cause attribution
of each historical failure and full player/provider acceptance remain separate.
The frozen 52 accepted rows and both pending rendering decisions are not promoted
or renewed; historical source fingerprints remain unchanged.
