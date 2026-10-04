# Native Join / Create Multiplayer previews

Implemented 2026-10-04 in `apps/qt-shell/multiplayer_setup_widget.*`. A shared,
mode-specific widget hosts two independent preview forms. No networking,
session enumeration, game creation or original process launch occurs.

## Installed inputs

Read-only inspection confirms:

| Mode | Installed layout | Controls |
| --- | --- | --- |
| Join | `Interface/JoinMultiplayerScreen/Screen (Join Multiplayer Game).cfg` | 3 headings, 1 edit, 3 transport radios, OK/Cancel |
| Create | `Interface/SetMultiplayerScreen/Screen (Set Multiplayer Game).cfg` | 4 headings, 2 edits, 3 transport radios, OK/Cancel |

Join's editable field is a username, not a host address. Create's first field
is a game name, second a username. Both use IPX/SPX LAN (ID 44), TCP/IP LAN
(ID 45), Null Modem cable (ID 46), OK (ID 10) and Cancel (ID 11). The widget
uses installed string-table labels rather than comments. Font roles are LARGE
for title/edits/buttons, SMALL for captions/radios, MIDDLE heading alignment
and LEFT transport labels. Confidence is high for these configuration contents.

The 800x600 backgrounds are `Join Multiplayer Screen 800-600.JPG` and
`Set Multiplayer Screen 800-600.JPG`. Rect2 coordinates are loaded per mode.
Join has edit `(160,210,480,40)`, transport rows at y 330/360/390, buttons y 465.
Create has edits `(160,160,480,40)` / `(160,250,480,40)`, transport rows at
360/390/420 and buttons y 515. The hidden second edit/caption cannot contribute
a game name to a Join request.

## Native form and request semantics

The local `Transport` enum expresses named choices; its ordinal is not a
recovered engine/wire value. TCP/IP is the native preview default, not a
recovered installed-user or engine default. Three radios form one exclusive
group. All legacy choices remain selectable for preview testing; availability
on an actual host has not been established.

`Form` contains game name, username and transport. `setForm()` rejects invalid
transport values, multiline/NUL or oversized names, and a nonempty game name
in Join mode before changing visible state. Empty drafts are permitted for
editing. Names are limited to 64 characters as a native presentation policy;
original length, encoding and normalization rules remain unverified.

OK requires a nonblank username and, in Create, a nonblank game name. Submission
emits a typed `Request` with mode, transport and names trimmed at their ends.
Join always supplies an empty game name. No server-address parsing is performed
because the installed layout contains no address field. Enter submits once,
autorepeat is suppressed, Escape or focused Cancel emits cancellation. Qt
radio/Space/tab behavior remains native. Initial focus selects the first field
for editing. These policies do not establish original networking behavior.

Both forms open from their respective native Quick Battle buttons or CLI flags.
Cancel returns to Quick Battle with focus restored to the initiating button.
Join OK now opens the [sample session selection preview](multiplayer-game-selection-qt.md);
Cancel there returns to Join with its local input retained. Create OK now opens
the [sample host lobby](multiplayer-lobby-qt.md). Lobby Cancel returns to Create. Independent
local drafts survive reopening; sample inputs are supplied only on creation.
No connecting/progress state or successful session is simulated.

## Fidelity, validation and use

Original background art, text and positions are reused. Native serif fonts,
edit borders, radio indicators and button colors remain approximations; scaling
and black letterboxing are native policy. No original screenshot comparison or
live networking validation was performed. See the
[visual-fidelity boundary](battle-results-qt.md#visual-fidelity).

```bash
./tools/run-qt-shell.sh --join-multiplayer
./tools/run-qt-shell.sh --create-multiplayer
ctest --test-dir working/build/qt-shell -R 'qt-(multiplayer-setup|preferences|save-game|load-game|map-selection|quick-battle-results|battle-results|mini-menu|main-menu|quick-battle-menu|shell-help|shell-startup)' --output-on-failure
```

All twelve targeted Qt tests passed. The new synthetic test exercises both
modes, all transports, exclusive radios, required/whitespace input guards,
trimmed request payloads, hidden-field isolation, transactional enum/text/layout
rejection, Enter/repeat/Cancel, scaling/background, Quick Battle routes,
initiating focus and independent retained drafts. Both installed-asset smoke
checks and conflicting-preview rejection passed. Captures at
`working/tests/multiplayer-setup-preview/join.png` and `create.png` were visually
inspected. Original-manifest checks surround artifact use; no original game
process was launched. Original DirectPlay/transport contracts, session browsing,
connection errors, host creation and live replacement remain unconnected.
