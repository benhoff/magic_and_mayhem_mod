# Native-menu save ownership groundwork

`tools/menu_save_store.py` keeps opaque original `.sav` bytes in a separate
owned store and copies them into each disposable installation. It seeds once
from the first installation, then treats its own files as authoritative.
Deleting a save therefore remains deleted even if the installation template
still contains it. Action staging and normal exit now use this service;
[bounded live Save/Load and fresh-process restore](campaign-save-load-live.md)
have separate original-game evidence.

Linux file locking serializes store changes. A completed session compares its
final files against its imported baseline, checks all changed names for
concurrent store edits before changing files, then copies each changed file
through an fsynced temporary file and rename. Conflicts preserve both store and
session files. Cancellation with no actual file changes does not rewrite saves.
The store has a `.retain` pin; cleanup is never automatic.

The policy bounds file count at the original menu's 100-row enumeration limit
and individual files at 256 MiB, rejects symlink files/directories, zero-byte
saves, invalid names and case collisions. Atomic copying is per file, not a
crash-safe multi-file transaction. The module does not parse the save format,
validate original serializer success, or establish compatibility/durability.
First-use seeding is deliberate native policy; later legacy saves require an
explicit future import mechanism instead of silently overriding native saves.

Four generated-byte unit tests pass for seed/write/delete/restart, cancellation,
whole-merge conflict refusal, and selected symlink/alias/zero/case failures:

```sh
python3 tests/test-menu-save-store.py
```

Actual Save/Load and launch/exit integration are validated only for the bounded
Wine-window embedding scenarios above. Native command presentation, overwrite,
write failure and full restored-state comparisons remain separate.

## Restart persistence repair, 2026-11-14

Hard restart used to kill the launcher before its clean-exit merge. A completed
`Ben.sav` remained in the failed installation while the next session imported an
empty persistent store. The save was recovered byte-for-byte into the configured
store without removing the session copy; the hash-bound recovery record is
`save-restart-persistence-20261114/ben-recovery.json`.

After the launcher's process group and selected Wine prefix have stopped and the
wait barrier succeeds, `LiveMenuSession` now runs the save-store helper before
reporting successful force-stop. Recovery is asynchronous, bounded to 30 seconds,
and holds launch admission until completion. A copy/conflict failure refuses the
restart and preserves the files with a diagnostic. Cancelling staging does not
recover an incomplete installation. The ordinary launcher also merges files on
an abnormal unbounded exit; bounded diagnostic runs retain their prior policy.
Recovery preserves missing persistent files instead of interpreting interruption
as deletion. Clean Quit still propagates explicit deletes. Repeating a merge
accepts an already identical destination, while differing concurrent edits still
refuse the whole merge before copying.

Filesystem and Qt lifecycle tests cover recovery, next-session import, conflict
refusal, a locked store without blocking the event loop, duplicate force-stop,
and destruction regressions. These use generated save bytes and process fixtures;
they do not establish original save decoding, interrupted-serializer validity,
power-loss durability or a fresh original-game Save/Load acceptance milestone.
