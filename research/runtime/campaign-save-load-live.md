# Native Qt campaign Save/Load and persistent restoration

The [source-stable save/load run](campaign-save-load-embedding-scoped-20261012.json)
creates `native-menu-b8fd540a.sav` through the original writer (351,734 bytes),
selects it through the original Load list, returns to World/Mini, answers the
original Quit confirmation and reaches normal Main Quit/launcher exit0.
The owned store retains identical SHA-256 bytes after completion.

The [fresh-process restore](campaign-restore-embedding-scoped-20261012.json) starts
another private prefix/game installation and loads the earlier persisted
`native-menu-live.sav` through native Main Load. Original Load succeeds,
World/Mini returns and normal Quit completes without changing the stored file.
The [refreshed Main Load/Cancel run](main-load-persistent-live-20261012.json)
passes twice with the persistent rows available. All reports retain prospective
source/claim snapshots, compiler dependency hashes, original callback traces,
staged input hashes and immutable-media verification before/after.

This evidence uses **Wine-window embedding with native Qt menus**, original
graphics and original audio. Explicit native-command attempts
`run-lbq9j81t` and `run-c2vb1cej` stalled before menu callbacks in this isolated
checkout, including the software/original-audio control. They remain negative
startup evidence under `working/tests/live-menus/`; they do not validate native
command presentation. The first embedding test incorrectly required native
frames and expired after successful original campaign ingress. Its evidence is
retained separately. No renderer, audio, balance or simulation replacement is
asserted by these menu runs.

## Observed hierarchy and actions

Original Save/Load from campaign Mini is a child of Mini screen17 at **depth6**,
with Main/Realm/World/Mini below it. The initial assumed World parent/depth5 was
rejected in `run-75lj58lg`; an initialized Save diagnostic recorded depth6.
The adapter now requires the actual fixed stack, Mini mode2/context5, original
receiver/vtable, control types and callback identities before dispatch.
Main Load remains ID12/parent3/depth3. Save is ID13. A corrected private adapter
fixture and Qt wire/controller/session check pass for these guards.

The successful fresh-game original callback sequence is:

```text
Main New Game → Region Enter → Mini Save → Save
→ Mini Load → Load → Mini Quit → defeat OK → Main Quit
(3,0),(18,0),(17,1),(13,0),(17,0),(12,0),(17,3),(6,21),(3,4)
```

The restoration sequence is `(3,1),(12,0),(17,3),(6,21),(3,4)`. Both keep one
original engine thread. No scripted memory changes or synthetic game saves are
used. Original return flags, actual save bytes, selected original list row and
subsequent World/Mini/normal Quit supply bounded successful-loading evidence;
this is not an independent comparison of every serialized/restored field.

## Persistent ownership and remaining limits

Action staging calls the [owned save store](menu-save-store.md). It seeds once
from the installation template, then copies its authoritative saves into each
private game. Normal unbounded exit0 merges changed files under a lock with
baseline conflict checks. Bounded/unclean termination preserves session files
and leaves the store unchanged. Synchronization failures return an error and
retain the source session. Store `.retain` pins prevent automatic cleanup.

The source store is `working/user-saves/native-menu`; original media and the
installation template are not modified by saving. Per-file atomic copying is
native policy, not original serializer/durability equivalence. Failed writes/loads,
more realms/parents, long play and
full restored-state comparison remain separate validation cases. The original
overwrite/error dialogs remain active. Native command/audio startup and live
save/load on that route remain unresolved.

The later [confirmation run](storage-confirmations-live.md) validates original
overwrite No/Yes, native Delete No/Yes and refreshed Save/Load rows for a unique
test-created save. It also fixes retained original admission between distinct
native requests. Deletion of a save present in the persistent baseline remains
a separate file-store integration case.

```sh
python3 tools/working-retention.py run --reserve-gib 3 -- python3 tools/test-live-campaign-storage.py
python3 tools/working-retention.py run --reserve-gib 3 -- python3 tools/test-live-campaign-storage.py --restore
```

Fresh save tests use unique safe stems and preserve prior evidence. Restore uses
the earlier `native-menu-live` fixture, or the name selected through
`MNM_CAMPAIGN_STORAGE_NAME`. `MNM_CAMPAIGN_STORAGE_NATIVE=1` selects the separate
native-command/original-audio diagnostic route; its startup is not a passing
milestone here.
