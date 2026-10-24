# Multiple observations and owners — MOO1, 2026-10-23

The bounded owner-observation lifecycle now covers multiple records and owners,
including erasure, cursor continuation, and task-source resolution after expiry
and reacquisition. Native storage already reconciled owner-local ordered values;
this work adds a checked `resolveOwnerTaskSources` value API and compares the
composed lifecycle against original instructions. No gameplay balance changes.

## Confirmed contract

Pinned NoCD SHA-256:
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Confidence is high within the compared domain.

* `4181d0` modes1/2 retain owner/cursor/matched state across the eight-work budget.
  Processing an expired record advances to its successor before erasure. Work
  continues across multiple calls, sparse enabled owners and an owner disabled
  during a partial sweep. Owner-local keys continue increasing after erasure;
  equal numeric keys in different owner trees identify different observations.
* Actual original red/black insertion, successor, erase and cleanup execute.
  Native maps preserve surviving value identity and reconcile the logical result;
  their tree topology and iterator ABI remain native representation policy.
* Snapshot construction/refresh clears owning708/8f9 and auxiliary6d0/6d4 while
  retaining borrowedcf3/8fd bytes. Actual5060a0 supplies callback/list generations,
  including hidden resets. Original52cbe0/52cc00 deletion and505f70 destruction
  execute. Snapshot expiry does not take ownership of live actor resources.
* `413330` creates owner-local bank1 caches and independent task-owned clones.
  Cache invalidation does not retire clones. Some clones resolve continuously;
  others first resolve after the underlying snapshot has expired.
* `407c10` looks up a source's key in the supplied owner's observation tree.
  A missing key or noncurrent marker sets activity89 to0 and returns a usable
  remembered source, preserving35 without reading the borrowed actor/snapshot.
  A current marker rebinds35 to the live slot before health/diplomacy admission.
  Dead/allied live targets set activity0; living hostile targets set activity3
  and contribute to the source root's incremental raw-coordinate average.
  Constructor byte25 remains the old snapshot-pointer tag after rebinding.
  Reacquisition under a new key does not retarget old-key task copies.
* `40a340` releases task-owned source storage after observation erasure without
  freeing or dereferencing35. All fixture allocations and216 callback generations
  are retired once. Checked allocation lifetime and suffix guards remain active.

The new API reads current live storage only after a current owner-local key match.
It rejects malformed forests/invalid live addresses as native admission policy;
those guards do not assert original error handling. Source traversal cursor bytes
are excluded from its value projection. Existing low-level traversal remains in
`OwnerTaskSources`; this API does not add reference counts or alias repair.

## Execution and reproducibility

`multiple-owners-trial-5.json` prospectively binds the implementation, fixture,
runner and transitive repository compiler inputs. Native64, original32 and ASan+UBSan+LSan agree
on215,540,764 checks and digest1154226086461827485. Eight episodes/896 updates cover:

| Witness | Count |
| --- | ---: |
| Record insertions |1264|
| Record expiries |528|
| Nonterminal cursor continuations |795|
| Task-root resolutions |6474|
| Missing-key leaf resolutions |54124|
| Stale-marker leaf resolutions |12616|
| Reacquired records |472|
| Retained rows while owner disabled |3120|
| Observations retained at age1024 |545|
| Missing/stale resolutions preserving a freed snapshot borrow |17748|
| Real callback generations destroyed |216|

Full packed bytes are checked against independently authored live inputs at
refresh, then against retained expected bytes through hidden ticks. Borrowed
pointer bytes and constructor pointer tags are checked against each process's
own allocator identities before normalization in the cross-process digest;
no original output seeds native inputs. Owner/cursor/sweep/matched state, ordered
keys, timestamps, value identity, activity, pointer rebinding and root centers are
compared. The original text is sealed RX and checked unchanged. Original artifact
manifests pass before and after each original-consuming experiment. The only
redirects are allocation/free/buffer-free and the explicit false line service.

Failed trials remain immutable. Trial1 hit an included-main warning and the
host temporary-directory quota; trial2 exposed reference-only compile errors.
Trial3 reached a cache-pairing path with an incorrectly declared fixture line
service ABI and crashed; bounded debugger evidence is retained. Trial4 passed
all per-run checks but its digest still included allocator-specific pointer tags.
Trial5 checks those identities locally and normalizes them only for comparison.
Compiler temporary files stay in the owned run directory after the quota failure.

Run from the owned checkout, using a new result name:

```sh
mkdir -p working/tests/compiler-tmp
TMPDIR="$PWD/working/tests/compiler-tmp" /home/hoff/swdev/magic_and_mayhem_mod/tools/working-retention.py run --reserve-gib 0.15 -- python3 tools/test-multiple-observation-owners.py --output working/tests/multiple-owners-next
```

All durable reports, declarations, failure records and history reviews use the
SQLite report writer; small research paths are compatibility references. Raw
binaries, logs and bounded runner/interchange JSON remain pinned under ignored
`working/tests`. No original artifact is modified.

Incomingb45abf10..9b83b3d7 review retains537 file transitions:493 watched source/
research transitions have exact receipts;44 immutable binary capture additions
are outside the text/source gate and retain separate payload identities. The
initial unclassified missing-receipt report is preserved alongside that review.
This accounting grants no past gate pass or new rendering validation.

## Remaining boundaries

The domain has twenty stationary ordinary actors, up to eight owners, strictly
increasing ticks and authored visibility, diplomacy and activation. The4f4ba0
line-reachability service explicitly returns false, so cache groups stay separate.
Arbitrary merged forest semantics and intermediate PE traversal cursor bytes,
mode2 nonempty task/notification consequences, exceptional allocation/Windows
SEH, arbitrary callback classes or invocation, stale callback/list consumers,
whole-brain4156b0 destruction and World/installed/live replacement remain open.
The Critical actor-services umbrella remains partial. The reusable lifecycle
fixture supplies bounded cases for later consumers without promoting them.

Completion storage: the main drive temporarily fell below90GiB while concurrent
work continued. The user explicitly authorized an89GiB reserve for this batch's
completion. A bounded working policy input supplies that override to the main
retention guard; the permanent policy stays90GiB. Coverage temporary trees use
RAM-backed storage. Source/evidence/SQLite backups and raw failures remain pinned.
