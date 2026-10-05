# Cross-launch Main Preferences persistence (UI24)

The [V6 Main Preferences bridge](preferences-engine-bridge.md) retains the original
writer and callbacks. UI24 adds an application-owned store of its seven exposed,
accepted settings; this is deliberate native persistence policy, not replacement
of recovered engine work. The [store format](../formats/engine-preferences-store.md)
defines the units, importer, readback, concurrency and atomic-write rules.

Implementation boundaries:

- `EnginePreferencesStore` is a Qt Core application adapter with no widget,
  runtime pointer or wire-offset dependency. It validates original file readback
  and atomically stores the accepted semantic values.
- `LiveMenuSession` waits for an accepted OK and ready Main, then asks the adapter
  to save. A persistence failure is logged without retiring menu ownership.
- `tools/menu_preferences_store.py` imports that store into a fresh disposable
  installation. It preflights both plain/encrypted config copies and leaves
  unrelated keys and preparation overrides alone.

Targeted synthetic checks cover missing/corrupt/wrong-build/oversized stores,
integer/enum bounds, exact selected-key overlay, comments/unexposed values,
encoded-container agreement and all-copy preflight. The Qt Core checks cover
original readback mismatch, duplicate/missing or invalid profile values,
concurrent-session conflicts, nonblocking locks, repeated OK, corrupt-store
repair and failed directory creation. No original artifacts or user config are
consumed by these synthetic checks.

```sh
python3 tests/test-menu-preferences-store.py
cmake --build working/build/qt-shell --target mnm-qt-shell engine-preferences-store-test menu-preferences-controller-test menu-spell-bridge-test menu-result-controller-test
ctest --test-dir working/build/qt-shell -R 'qt-(engine-preferences|menu-(bridge|battle-bridge|spell-bridge|mini-bridge|preferences|result)|preferences)' --output-on-failure
python3 tools/test-live-menus.py --preferences-restart
```

The live workflow uses an isolated config home and two separate shell/game/Wine
process runs. First it checks preview, Cancel rollback/file preservation, OK,
reopen and normal Quit. The second starts with the saved store, checks all seven
fresh engine and Qt values, then closes Preferences through Cancel/Main Quit and
requires the store to remain byte-identical. Original manifests are verified
before/after game experiments; no manual testing is required.

The two-launch run `working/tests/live-menus/run-nxp69d9j` passed: initial
`[0,-250,0,0,1,0,0]`, accepted `[0,-500,0,0,2,0,0]`, then all seven
values restored in both the fresh engine and Qt. Both launchers exited normally;
the second Cancel/Quit preserved the store byte-for-byte. Ten targeted Qt checks
and three Python checks passed. [Recorded evidence](preferences-persistence.json)
pins reports, source hashes and the live executable. Both original manifest
checks verified 2,927 files. UI23's
historical evidence is retained unchanged; host/session changes make its old
fingerprints stale. UI24 supplies current host validation without refreshing the
historical hashes. No new original equivalence, drawing bypass, hardware audio,
resolution-change or full caller coverage is claimed. The configured store is
specific to the pinned no-CD build. Original-menu fallback edits are not exported
through this policy.

The focused and central coverage audits passed without errors. UI24 evidence is
current; three focused UI23 evidence records remain stale by design. The current
source census reviewed 846 files and linked the new importer, store and restore
harness to `NP.preferences-persistence`. The central register was reconciled
with the focused register and its imported hash, preserving unrelated records.
Audit outputs and the reviewed census are under
`working/tests/re-coverage-*-preferences-persistence*` and
`working/tests/preferences-persistence-central-source-index.json`.
