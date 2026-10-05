# Main Preferences resolution and display rebuilding (UI25)

UI25 validates the retained original Main-caller resolution path. It does not
replace display rebuilding, extend Mini admission, or establish physical-monitor
or gameplay resource equivalence. The executable is pinned to SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.

## Recovered leave ordering

Original `0x004a8b60` always invokes the screen's release-resources slot +0x20.
With gameplay initialized (`0x006cbb80 != 0`) it calls `0x004711d0(0)`, rebuilds
only if screen byte +0x67 is set, clears that flag, then calls `0x00470ee0`,
`0x004d37c0` and `0x004d3b40`. Without gameplay initialized it rebuilds and
calls `0x004d37c0` only for a changed resolution. Both branches clear pending
push +0x33. A repeated leave after a rebuild does not rebuild again.

The private original-bytecode oracle now checks four game-initialized/change-flag
combinations and two repeated leaves. Only display/resource dependencies and the
release-controls slot are stubbed for these leave cases. The original leave body
executes unchanged. This proves selected branch ordering, not resource behavior.
Existing 216 OK combinations, 12 Cancel cases and device/MCI/writer cases remain
in the same targeted regression.

Original rebuild `0x004a88f0` selects 800×600 for High and 640×480 for Low.
It updates the border descriptor, resizes the HWND at `0x00656614`, recreates
resources and loads two width-specific fonts. It sets font mode globals
`0x006b0630` and `0x006568b0` to 2. All this remains original work.

## Test-only live observation

`runtime/menu/preferences_display.h` installs a forwarding wrapper in the
Preferences leave vtable slot only when `MNM_MENU_DISPLAY_OBSERVE=1` and V6
Preferences is active. Expected slot/entry bytes and readable globals are checked
before modifying the slot. It forwards once to original leave and preserves the
original post-call Windows LastError. Normal sessions leave this slot untouched.
The staging script checks original leave and rebuild bytes after adding the DLL
import. No display-setting or availability flags are forced.

The existing bounded MNMMENU1 64-byte observation contract gains optional events:

| Event | argument | result |
| --- | --- | --- |
| 6, before original leave | screen +0x67 change flag | raw IsHighRes setting |
| 7, after original leave | game client width from GetClientRect | game client height |
| 8, after original leave | cleared change flag | small/large font mode globals packed into low/high 16 bits |

The semantic Qt harness edits the original-admitted resolution radio. It cancels
one changed draft, applies four alternating changes, then reopens and closes
Preferences through original Cancel/Main Quit. Each reopen checks all seven
engine settings and the Qt resolution selection. The runner independently requires
six complete observation triples, exact original callback/ACK order, expected
client dimensions, cleared flags/initialized/pending state, restored font modes,
restored original settings and persisted values, and normal launcher exit.

```sh
cmake --build working/build/qt-shell --target mnm-qt-shell -j 4
python3 tools/test-preferences-contract.py
python3 tools/test-menu-observer.py --preferences
python3 tools/test-live-menus.py --preferences-display
ctest --test-dir working/build/qt-shell -R 'qt-(engine-preferences|menu-(bridge|battle-bridge|spell-bridge|mini-bridge|preferences|result)|preferences)' --output-on-failure
```

Original manifests are verified before/after original-artifact tests. Config and
Wine prefixes are isolated under the generated test directory. No manual testing
is required. The run `working/tests/live-menus/run-p_3gv6rh` passed with
High → Low → High → Low → High client sizes and normal original Quit. Both
Cancels preserved the applied resolution; the first also left profile/store
unchanged. The isolated oracle `run-zyywnd42`, V6 fixture `run-2a98rafu` and
ten targeted Qt checks passed. All original manifest checks passed for 2,927
files. [Recorded evidence](preferences-display-rebuild.json) pins the sources,
reports, callback/observation trace and staged DLL/live shell hashes. Confidence
is high within these scopes; the remaining boundaries below are unchanged.

The live scope is Main under Wine's virtual desktop in Xvfb. Native ownership,
original client dimensions and resource continuation are observable; visual font
quality, drawing/pixel equivalence, physical fullscreen transitions, actual
hardware display failures, gameplay resource rebuilding and other callers remain
separate boundaries. The original engine remains responsible for resolution
changes in normal Qt Preferences sessions.

The focused and central coverage audits passed with no errors. UI25 fingerprints
are current; UI22/UI23/UI24 historical evidence hashes remain preserved and
stale. The reviewed 858-file census links both new source files to
`PR.display-rebuild`; central focused-register reconciliation and its import
hash are current. Census/audit outputs are retained as
`working/tests/preferences-display-central-source-index.json` and
`working/tests/re-coverage-*-preferences-display*`. This indexing review does not
claim broader original or live coverage.
