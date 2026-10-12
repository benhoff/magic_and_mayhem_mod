# Native-menu save ownership groundwork

`tools/menu_save_store.py` keeps opaque original `.sav` bytes in a separate
owned store and copies them into each disposable installation. It seeds once
from the first installation, then treats its own files as authoritative.
Deleting a save therefore remains deleted even if the installation template
still contains it. This service is not connected to launch/exit yet.

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

Actual native Save/Load and launch/exit integration remain the next increment.
