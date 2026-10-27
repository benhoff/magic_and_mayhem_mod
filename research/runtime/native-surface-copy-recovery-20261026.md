# Native surface copy recovery composition

`recoverOwnedSurfaceCopy` joins the existing copy geometry planner to the
recovered retry scheduler and owned surface backend. The bounded service accepts
only opaque `0x58c4a0` and keyed `0x58c8a0` copy contracts. The existing one-call
copy API retains its separate contract.

The caller supplies native handles, source rectangle, destination position,
window-context snapshot, saved keys and explicit reload/report bindings. A lost
copy reports its draw result, restores the source and then the destination,
reapplies keys after each successful Restore, invokes that surface's selected
reload route and retries. Both surfaces are visited even when only one was lost.
Busy and other nonzero results retry within the supplied draw budget. Exhaustion
returns an incomplete result; it never manufactures an original successful return.
Missing opaque handles bypass; missing keyed handles and unsupported wrapper
kinds refuse. Exceptions from reporting, reloading or unknown pixel sampling
propagate. No background work or application installation is enabled.

Restore preserves identity and metadata and leaves lost pixels unknown. A reload
callback must supply actual bytes before those pixels can be read. A wrong-mode
primary destination remains lost while source recovery still runs. Keys, WAIT,
Blt/BltFast and destination-only window translation use the existing contracts.
Geometry, context and handles are snapshots for the bounded call: dynamic window
queries, original interface replacement/null mutation and destructive/reentrant
callback lifetimes have no recovered equivalence claim.

Validation compares all 64 combinations of opaque/keyed, Blt/BltFast, WAIT,
primary-window translation and neither/source/destination/both lost surfaces,
checking complete input-derived RGB565 destination planes and callback order.
Additional controls cover invalid options, missing handles, persistent/transient
busy, unknown restored pixels, reload failure and wrong-mode primary Restore.
The unchanged scheduler separately compares fresh original scripted fault traces;
these are original branch evidence, not physical driver-loss observations.
Release and ASan/UBSan runs retain their own results and source hashes in
`native-surface-copy-recovery-results-20261026/execution03.json` through SQLite.

This is a scoped native integration. Physical original lost-draw composition,
real original resource acquisition, arbitrary callback lifetime/reentrancy,
changing window/COM identities, Windows/hardware loss and live replacement
remain pending. Historical broader surface evidence retains its original source
hashes and becomes stale where shared wrapper sources changed; the fresh
composition does not renew every parent contract.

Reproduce with a new output/report identity under the storage guard:

```sh
python3 tools/test-native-surface-copy-recovery.py --main-root /path/to/main \
  --output working/tests/surface-copy-recovery-next \
  --build-root working/build/surface-copy-recovery-next \
  --report research/runtime/native-surface-copy-recovery-next.json
```
