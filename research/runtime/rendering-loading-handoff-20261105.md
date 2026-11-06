# Native Campaign and Quick loading handoff closeout — 2026-11-05

Confirmed within scope: `LH05.loading-handoff-closed01` closes Campaign loading
and Quick Battle loading for both explicit player profiles at actual 640x480 and
800x600. The selected pack passes **28 of 156 rows**: all 24 accepted parent rows
remain unchanged and four loading rows are new. The other 128 rows and both final
decisions remain pending. This is analysis of retained evidence, with no new game
run, pixel comparison, fixture or renderer/runtime change.

## Evidence and confidence

Eight accepted cases bind the original Campaign confirmation or Quick
setup/map/spell/start callback chain to a native loading-to-World handoff. The last
actual swapped pre-World surface has a native GUI owner, source lease and complete
GPU readback. The first swapped World is source generation 1; actual generations
2 and 3 follow with increasing presentation clocks and command progress. Each
import has an owned RGB565 source surface of the actual target size, a recorded
CREATE sequence and a live presentation surface. Its command sequence precedes
the presented handoff. Complete command histories rederive exactly the accepted
resource ledger, including normal final count and logical-byte conservation.

All four selected presentations per case (pre-World, then Worlds 1, 2 and 3) bind
to the earlier independent complete-output comparisons: **32 presentations and
27,984,000 pixels**, zero mismatches. Those comparisons reconstruct continuously
from owned initial and intervening history, without native player pixels or
original destination snapshots as oracle inputs. The native progress checkpoints,
original menu acknowledgments and single original thread corroborate the entry.
The selected map/default initial region and provider identities remain those of
the accepted parent; both profiles report llvmpipe. Hardware support is not inferred.
Confidence is high within these finite loading boundaries.

89 current controls pass: 50 normal-acceptance tests (including eight new loading
guards), 30 coverage UI tests and nine client tests. They reject missing/duplicate
entry domains, absent callbacks or native progress, missing source generations,
wrong owners/sizes, incomplete or mismatched output, dependent history, broken
resource/import order and parent-row aliasing. An initial test setup error and a
report-namespace restriction in the new glue were corrected; their exact failed
logs remain retained. Analysis02 is the accepted result.

`LH05.PR05-verification-boundary01` preserves the unchanged resume analysis/test
closure across these passive helper/test additions. The earlier LM05, CS05 and
SW05 scope reviews remain applicable with no source actions. Raw historical
source drift stays visible; no historical hashes, evidence definitions, statuses
or failures are refreshed. Presentation dependencies remain outside native proof.
No browser preview is claimed.

SQLite references:

- `rendering-normal-results-20261031/loading-handoff-closeout-20261105-02.json`:
  retained-data analysis, exact handoffs/import owners and all 156 acceptance rows.
- `rendering-loading-handoff-results-20261105/current-closeout-binding-01.json`:
  current sources, accepted parent, controls and unchanged negative evidence.
- `rendering-loading-handoff-results-20261105/controls-final-01.json`:
  the 89 affected controls and retained exact logs.
- `rendering-loading-handoff-results-20261105/candidate-review-01.json`:
  reviewed closure boundary and all eight explicit preparation limitations.

The bounded analysis is ready to run with a fresh report name:

```sh
python3 tools/join-rendering-normal-acceptance.py --loading-handoff \
  --parent-evidence PR05.preferences-resume-closed01 \
  --report research/runtime/rendering-normal-results-20261031/loading-handoff-LOCAL.json
```

## Remaining work

Every retained first-World comparison explicitly records
`complete_caller_preparation_proved: false`. Loading ownership and first-generation
handoff do not supply that missing caller state. Both first-composed-World leaves
remain pending. Loading-art/original spinner equivalence and real later-map
progression loading are also separate; reload is not natural progression.

There are 66 pending journey rows, 42 operation rows and 20 gate rows. Per-World
exit retirement, direct Mini Apply/pause timing, ready Main focus, movie output/
skip-return, camera/picking/HUD/state, natural results/progression and remaining
ABI/font/DC/provider/fault boundaries remain open. The known CS05 pre-Save
admission/reliability failure is preserved. Neither original recovery nor
supported-player completion is promoted.
