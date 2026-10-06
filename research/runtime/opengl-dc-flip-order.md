# DC handoff and flip ordering

This extends the existing continuous-only opt-in native scheduling policy to
application GetDC, ReleaseDC and Flip callbacks. It is not recovered original
DirectDraw ordering. The historical flag name `MNM_RENDER_ORDERED_COPIES=1` now
covers copies, CPU Lock/Unlock, DC handoffs and flips. Default scheduling and
wire formats remain unchanged; no resource, queue, tracker or recovery budget grows.

## Ownership boundaries

GetDC admission includes the original acquisition and recording the returned
application-held bitmap identity. ReleaseDC includes the validated pre-call DIB
snapshot, original release and successful commit. The shared gate serializes
these callbacks with copies and CPU handoffs, while no tracker is held during
an original operation or a wait. Its 50ms tick-based budget and same-thread
reentry behavior are retained. Original arguments, return values and LastError
are preserved; scoped cleanup releases admission on ordinary failures as well
as success.

The lease does not span application painting between GetDC and ReleaseDC. Existing
thread owner, selected bitmap, generation/epoch, layout, format and complete
checkpoint guards remain authoritative. No observer GetDC, Lock, GetDIBits,
bitmap replacement or COM reference is introduced. GDI metadata queries, flush
and GetBitmapBits remain part of the existing admitted DIB handoff, not individual
native text/GDI replay. Properties and lifetime callbacks outside the gate can
still invalidate a pending handoff. A timed-out GetDC is forwarded without
recording a producer bitmap lease; a later release never invents that ownership.

Flip admission and commit span the same gate, preserving observed two-buffer
chain, layout, active-lock and generation checks. A timed-out original Flip can
swap both buffers even though the producer could not observe it. The forwarding
branch therefore invalidates the metadata/pixel epochs before and after that
original call, as well as the existing target misses. It does not retain an
unobserved back-buffer checkpoint. All original calls still run exactly once;
uncertain histories terminate rather than publishing guessed pixels.

Timeout forwarding keeps the existing legacy-history incomplete flag. An
original failure does not become a successful mutation. Abnormal owner-thread
termination, complete original callback dependencies and other callback-family
concurrency remain outside the scheduling contract.

The front's successful flip diagnostic origin is corrected from 10 to 11, matching
the back's flip origin; 10 remains attachment observation. Historical traces are
unchanged: front origin 10 in older source could mean a flip and must not be
reinterpreted as an attachment-only proof. This private diagnostic field is not
wire resource identity or an original baseline finding.

## Independent fixture validation

The [fresh eleven-case matrix](opengl-dc-flip-order-native-isolated-current-20261006.json) compares
228 complete native frames across six valid and five refused sessions.

- `cross-dc`: a real Wine worker GetDC waits behind a pending original copy. The
  application owns an actual selected RGB565 DIB; independent fixture writes are
  committed into its offscreen fixture source by original ReleaseDC and poisoned
  afterward. This case checks ordering and primary outputs; it does not present
  the changed back surface afterward. Gated `dc32` independently compares the
  admitted DC update pixels in presented primary frames.
- `dc-timeout`: both DC handoffs forward under finite gate refusal while the
  outer original waits for the worker. Original pixels/results/counts remain
  independent; the interrupted native stream publishes only its initial frame.
- `cross-flip`: a worker Flip waits for a pending partial primary copy. Independent
  full pixels distinguish the initial front, partial-copy result and subsequent
  swapped front; all three native frames match, including final DELETE/END cleanup.
- `flip-timeout`: an original worker flip swaps distinct front/back contents while
  the outer original waits. Epoch invalidation refuses the interrupted history;
  only the initial native frame publishes. No guessed back-buffer state is retained.
- Existing CPU handoff, CPU timeout, BltFast, copy timeout and same-thread source
  write cases retain their results. Mixed RGB16 and RGB32 DC regression now run
  with the gate enabled, including failed original copy/flip/CPU calls and a
  failed ReleaseDC followed by successful retry.

Original arguments/results/LastError and exact call counts are checked in the
fixture, independently generated native pixels feed expected GPU frame hashes,
and consumer storage must retire to zero. The source guard verifies the final
fingerprints. No actual original-game driver or font equivalence follows from
these synthetic COM objects and actual Wine DIBs.

Reproduce with the existing optimized probe and Wine/Xvfb prerequisites:

```sh
xvfb-run -a python3 tools/test-render-mutations.py working/build/renderer-live --case cross-dc --case dc-timeout --case cross-flip --case flip-timeout --case cross-lock --case lock-timeout --case cross-fast --case copy-timeout --case source-write --case mx16 --case dc32
xvfb-run -a -s '-screen 0 1800x1000x24' python3 tools/test-live-render-routes.py working/build/renderer-live --mode campaign --ordered-copies --require-world-summon --world-seconds 30
```

The second command also needs Tesseract. It checks original Zombie tutorial
prompts/control count, forced World reader recovery and independent unsynchronized
stable native regions. It is not synchronized moving-actor, effect, full-frame,
movie or hardware driver equivalence. The original files are verified before
and after the isolated run; executable staging remains scripted.

Remaining: palette/property/lifetime concurrency outside the gate, reentrant
DC/flip-specific fixtures, borrowed-interval concurrency, prolonged driver calls,
repeated scene/action coverage, timeout with legacy archive replay enabled,
animation/full-frame/driver comparison and default enablement. The exact requester
in the historical tracker miss remains unproven. No live replacement or gameplay
balance change is claimed.

Validation records from the initial and rebuilt workspace probes are retained
with their original fingerprints. Those probes also saw independent uncommitted
renderer/DIB changes; they do not establish validation of the committed renderer.
The isolated records use the committed renderer plus only this scheduling chunk.
The initial runs passed within their recorded source scope. None of those
workspace records is relabeled as fresh committed-tree evidence.

Committed history `4c01686..4273de1` was reviewed against exact parent/current
file and behavior receipts; the retained report finds no unresolved receipts.
This history review does not assert past gate execution or new validation.

The first isolated observation completed all17 region comparisons and793 frames,
with zero terminal resources, but its final source-preferences guard failed
because staging copied a symlink to the shared working installation. The
shared executable was restored to the hash-verified supplied no-CD input, and
preferences were restored to their exact pre-staging SHA256
`9e57225c21366a4f06fb0e5b3af234c08f3b76ac84800fa9ffc138da2a638dbc`. Its unchanged failure record is retained
separately. Final isolation copies the installation and includes the subsequently
committed renderer dependencies.

After the dependency commit, the additional exact history review covers
`4c01686..b9ed3a9`, including23 behavior changes in the new renderer chunk,
with no unresolved file/behavior receipts. The earlier history report is retained.

The [final private-installation original observation](opengl-dc-flip-order-live-isolated-current-20261006.json)
passes with792 native frames,487 additional frames after forced
World recovery over40904ms,17 matched independent stable regions and zero terminal
resources. Both complete source fingerprints and unchanged private source
preferences pass;2,927 immutable artifacts verify before and after.
