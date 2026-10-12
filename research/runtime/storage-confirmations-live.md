# Campaign Save confirmation boundaries

Build: No-CD `40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
This work uses native Qt menus with Wine-window embedding and original graphics,
audio, simulation, serializer and overwrite dialog. Native command presentation
is a separate unresolved route.

## Recovered admission lifetime

The [negative live run](storage-overwrite-admission-negative-20261012.json)
created a unique save and answered the original overwrite dialog No. Its bytes
and modification time stayed unchanged. The next native Save dispatched the
original writer without another modal; the harness expired waiting for a second
confirmation. That failed run and its original save remain retained. It is not
a successful complete confirmation journey or a normal-exit persistence result.

Original confirmation callback `0x004abb20..0x004abb7f`, thiscall(receiver,
answer), explains the result. Answer0 (Yes) calls deferred request `0x004ab870`
when admission DWORD `+0x5f` is nonzero. Answers0/1 close/free the modal, clear
modal pointer `+0x3b` and enable the list. Answer1 (No) **does not clear admission**.
Other answers leave the modal untouched. The writer tick consumes admission on
the next actual writer call, rather than when No dismisses the dialog.

The [private original confirmation fixture](save-load-confirmation-contract-20261012.json)
executes all three branches, preserves the full RET immediate and checks stack
balance and callee-preserved registers. Disk-space/deferred request, modal,
serializer and list helpers are stubbed. Confidence is high within those
executed callback branches; this does not compare complete file durability.
The pinned Ghidra inventory does not discover an owning function at `0x4abb20`.
Accounting therefore uses the documented recovered-range exception
`save-overwrite-confirmation-4abb20`, anchored to the private fixture's pinned
entry bytes and full tested return boundaries. The discovery inventory, other
unassigned bytes, indirect flows and incomplete functions remain unchanged;
this exception covers only the stated confirmation branches.

## Intentional native policy

Each new native Save request clears retained admission after ownership, control,
name and destination guards succeed, before invoking the original Save callback.
Admission values outside 0/1 reject the request. Invalid and repeated requests
preserve admission. Original Yes continues the already-pending write without
dispatching another native request, so its original approval remains effective.
This ensures that declining an overwrite does not approve a subsequent Save.
It is native transaction policy, separate from recovered original admission
lifetime; no original executable file is patched.

The [private adapter fixture](storage-fresh-admission-fixture-20261012.json)
checks retained/unknown admission, invalid names, new request reset and duplicate
polling alongside existing parent/row/control/stale/modal/retirement guards.
Its disk-space, serializer and deletion dependencies are stubbed.

## Bounded live scenario

The [source-stable live result](storage-confirmations-live-20261012.json) passes
overwrite No/Yes, native Delete No/Yes, refreshed original/native Save rows,
Save Cancel, deleted-row exclusion in Load, Load Cancel and normal campaign/Main
Quit/launcher exit0. Initial and replacement SHA-256 differ. The original callback
trace is `(3,0),(18,0),(17,1),(13,0),(17,1),(13,0),(13,0),(17,1),(13,2),`
`(13,1),(17,0),(12,1),(17,3),(6,21),(3,4)` on one engine thread. The final
owned-store merge reports no changed baseline files and the test save is absent.
Three focused Qt Save/Load/storage checks also pass. The earlier Xvfb socket
startup failure and original-admission timeout remain separate negative evidence;
no failed result was overwritten.

`tools/test-live-storage-confirmations.py` creates a unique `native-confirm-*`
save, checks original overwrite No preserves bytes/time, requests Save again,
answers original Yes, then exercises native Delete No/Yes, original row refresh,
Save Cancel, Load absence/Cancel and normal campaign/Main Quit. Original callbacks
must match the exact expected sequence on one engine thread. The final merge
must leave every pre-existing owned save unchanged; this scenario deletes only
its newly created test save, so persistent deletion of a baseline save remains
a separate case. Physical helper success alone is not accepted as game evidence.

Read-only input manifests run before/after original consumption. Prospective
source/claim snapshots and compiled menu dependencies are checked. The original
writer's initial and replacement bytes are fingerprinted; hashes/time are bounded
write evidence, not independent serialized-state or crash-durability comparison.
Failed writes/loads, other callers/realms and native-command presentation remain
pending.

```sh
python3 tools/working-retention.py run --reserve-gib 3 -- python3 tools/test-live-storage-confirmations.py
```

From an isolated worktree, invoke the main checkout's retention tool and pass
the absolute worktree test path: the guard launches commands in its main root.
