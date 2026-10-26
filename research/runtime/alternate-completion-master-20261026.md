# Alternate canvas: current-master integration

This batch joins recovery `5757e548` with landed master `d5e08213`. Execution
was acquired prospectively by composing the committed `1ffeb53e` and `3544f60a`
source changes onto the recovery. Every declared source and behavior claim in
the six final reports matches the resulting `5757e548`/`d5e08213` tree.
The separate native-player merge in the shared checkout is outside this frozen
source validation. Its pending index and source edits are not changed here.

## Fresh bounded execution

[Headless controls](alternate-completion-results-20261026/native-controls04.json)
pass both video modes and padding 0/0, 4/3 and 96/0: 54 frames, 144 prepared
replays, 94,112,736 pixel checks, all backing guard words, six explicit context
checkpoint/rebuilds and zero remaining surfaces. Ordinary frame readbacks remain
zero; recovery checkpoints use explicit diagnostic readbacks.

The [asynchronous journey](alternate-completion-results-20261026/native-async07.json)
completes 302 Worlds and copy-backs with padding 4/3. The
[Save/Load/rebuild journey](alternate-completion-results-20261026/native-storage07.json)
completes 515 Worlds and copy-backs with padding 96/0 and two publications.
Both verify normal Quit, unchanged source declarations and private Wine cleanup.
These fresh results supplement the retained 303/737-World recovery-batch runs;
old records and fingerprints remain unchanged.

[Original raster comparison](alternate-completion-results-20261026/original-comparison04.json)
checks four complete World checkpoints: 1,920,000 RGB565 words and zero
mismatches. Unchanged original raster entries consume diagnostic native entry
histories. Two authored baseline histories remain separate controls. This is
selected World raster equivalence, not independently acquired original presented
sessions, simulation/save-state equivalence or a general baseline-independence
proof. Explicit context rebuilding does not establish arbitrary in-flight
physical device-reset recovery. No fixed performance acceptance is asserted.

## Original ownership stays separate

The original [low-mode](alternate-completion-results-20261026/copy-low02.json)
and [high-mode](alternate-completion-results-20261026/copy-high02.json) error
observations remain source-current. At caller `4b2e1c`, the original full-source
736/896-wide rectangles exceed the ordinary 640/800-wide campaign capture
surface by 96 pixels. The original `Blt` returns `88760096` before World.
Both alternate pointer slots stay zero. Neither executable rectangles nor
pointer slots are corrected by this batch.

Original alternate allocation, natural publication, initialization, base/source
relationship, computed/bulk/external writers and retirement remain unresolved.
The recovered failed ordinary capture call does not identify an alternate owner
or prove global dormancy. Native centered padding is an intentional owned-storage
policy, separate from original alternate semantics.

## Retained failures and integration accounting

The first refreshed asynchronous attempt reached World but failed an original
Mini transition after a witnessed Hermes dismissal. It is retained as
[negative evidence](alternate-completion-results-20261026/native-async05.json).
The next run restored the established process setup and passed; the cause of the
first input failure is not established. An initial comparison was interrupted
to restore its required launcher guard; the next comparison refused an inherited
read-only temporary directory before original raster execution. The final run
uses a private writable temporary directory and pinned Mesa software rendering.
No failed attempt receives successful journey or equivalence credit.

An uncommitted N29-only merge passed its scoped audit, then was superseded by
the direct merge with landed master. Its exact review cache, journal exports,
source census and audit remain retained; no past commit or gate pass is asserted.
Historical file transitions receive exact receipts separately from fresh engine
execution. New evidence IDs preserve each earlier result and source fingerprint;
no new comparison or replacement milestone is promoted for the native owner.

[Raw integration retention](alternate-completion-results-20261026/retention02.json)
verifies every member of the 1,380-file archive through SQLite. Unchanged game
and prefix installation copies remain reproducible inputs. The first batch's
already-backed temporary archive is relocated byte-for-byte into RAM while its
original path and both durable SQLite payload copies remain available. Required
pins and active consumers are preserved. The user-approved temporary 88 GiB
main-drive reserve applies to this batch; the checked-in 90 GiB policy remains
unchanged. Source, build and current execution staging use a private RAM
namespace. Final local completion requires verified durable SQLite backups and
the normal staged coverage hook. No evidence publication is requested.
