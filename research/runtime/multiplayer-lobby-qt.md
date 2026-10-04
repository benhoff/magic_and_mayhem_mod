# Native Multiplayer Battle Setup / lobby previews

Implemented 2026-10-04 in `apps/qt-shell/multiplayer_lobby_widget.*`.
These are standalone host/guest previews with supplied data and local intent.
They neither initialize a transport nor establish a connection or start a game.

## Installed configuration evidence

Read-only inspection confirms three distinct configurations under
`Interface/MultiplayerBattleSetup`:

| Layout | Declared controls |
| --- | --- |
| `screen (MultiPlayer Battle Setup).cfg` | 40 SMALL texts, four handicap sliders, eight portrait/colour standard buttons, message list, chat edit and Cancel |
| `screen (MultiPlayer Battle Create).cfg` | Thirteen battle-setting sliders, Map/Start and three boot buttons |
| `screen (Multiplayer Battle Join).cfg` | Ready button |

The shared background is `Multiplayer Battle Setup 800-600.jpg`. All Rect2
positions and installed static labels are reused. The shared chat area is
`(25,325,350,200)` and the edit is `(25,540,749,35)`. Cancel is
`(400,490,175,35)`; host Start and guest Ready share `(600,490,175,35)`.
Map is `(400,100,60,35)` in the host variant only. Confidence is high for
these installed configuration contents, not for original runtime permissions.

The files reuse section names for different controls: shared `TEXTBUTTON_1`
is Cancel; create `TEXTBUTTON_1` is Map; join `TEXTBUTTON_1` is Ready. Likewise,
shared sliders 1–4 are handicaps, whereas create sliders 1–13 are battle rules.
The loader retains these layouts separately rather than flattening sections.
`loadMenuLayout()` reads additional configurations through the bounded read-only
AssetStore and parser. Guest display domains also come from the create file.
Shared empty `TEXTBUTTON_3`–`TEXTBUTTON_8` sections do not create visible buttons.

The thirteen rule domains and four handicap domains match
[Single Player Setup](single-player-battle-qt.md#installed-layout-evidence).
Rule names follow the visible Neutral/Chaos text rows, despite conflicting
slider comments. Shared literal rule values include Mana 400, Health 650 and
100 for several other rules; these conflict with the create slider domains.
They are not treated as confirmed defaults. The preview uses supplied values
within the configured slider domains, initially their minima. Engine defaults,
units, sentinel values and field ordering remain unrecovered.

## Native model, permissions and signals

`Lobby` carries an opaque session ID/game name, map ID/name, four player
records, thirteen rule values, local slot and local Ready state. Portrait and
colour IDs are caller-owned semantic identifiers; Rule ordinals are not engine
or wire fields. Invalid slider ranges/steps, off-step values, partial IDs/names,
invalid local slots and incomplete active players are rejected before mutation.
Active player names are single-line and at most 64 characters as native policy.
Host mode uses local slot 0 and has no Ready state. Modes are immutable.

Hosts edit rule sliders and active-player handicaps, choose a map, emit Start
and remove supplied guests in slots 1–3. Guests see numeric rules without host
sliders, Map, Start or removal controls, and edit only their own handicap.
Both modes can request a change to their own portrait/colour. Remote portrait
and colour buttons are disabled; previews cycle local sample choices. These
ownership rules are native policies, not recovered original authorization.
Removed guests are not automatically re-added; a future roster update supplies
actual membership changes.

Start requires a session, an active local host, a map and another active player.
It emits a typed snapshot and remains pending on the host screen. Guest Ready
requires an active local slot/session, toggles a local flag and emits
`readyRequested(bool)`. Readiness does not prove a network acknowledgement;
Start does not enforce remote readiness because no recovered roster contract
is available. Enter respects the focused button; repeat is suppressed.
Escape/Cancel emits cancellation.

Chat is a read-only plain-text log and a single-line editor. Enter in the editor
emits trimmed `chatRequested(text)` and clears the draft; blank input is ignored.
Enter in the log never starts a game. The preview echoes only local messages,
without delivery claims. Messages have single-line, nonblank sender/text limits
of 64/256 characters; at most 100 entries are retained. Invalid replacement or
append preserves existing messages. No rich text or chat protocol is used.

## Preview orchestration

Create OK opens a host lobby using its username, game name and transport
context. Join OK opens session selection; session OK opens a guest lobby using
the selected opaque ID/display name and submitted username/transport. Samples
are explicitly previews, not connected peers. Host and guest widgets retain
independent state. Returning to the same context retains settings, Ready,
chat history and an unsent draft; changing session/game, username or transport
resets the lobby and chat. Cancel restores Create or session selection and
focuses its OK button. Standalone CLI previews prepare these same callers.

Host Map Selection preselects its current map, updates ID/name only on OK and
returns to the host with all other draft data retained. Cancel leaves the map
unchanged. Existing Single Player and standalone Map return routes remain intact.
Asset failure leaves the caller and its draft available.

## Fidelity and validation

Original art, labels and coordinates are preserved. System serif fonts, control
colors, sprite state/disabled styling, Ready check styling, message rendering
and scaled letterboxing remain approximations. The shared Handicap captions
overlap the sliders; the native preview hides those captions and supplies
accessible slider names. No original screenshot equivalence or live multiplayer
validation is claimed. See [visual fidelity](battle-results-qt.md#visual-fidelity).

```bash
./tools/run-qt-shell.sh --multiplayer-lobby host
./tools/run-qt-shell.sh --multiplayer-lobby join
ctest --test-dir working/build/qt-shell -R 'qt-(multiplayer-lobby|single-player-battle|multiplayer-game-selection|multiplayer-setup|preferences|save-game|load-game|map-selection|quick-battle-results|battle-results|mini-menu|main-menu|quick-battle-menu|shell-help|shell-startup)' --output-on-failure
```

All fifteen targeted Qt checks passed. Five installed-asset smoke checks and
two invalid/conflicting-preview-flag checks passed. Synthetic fixtures cover separate layout roles, mode ownership, local/remote
controls, step snapping, request snapshots, Ready, chat trimming/plain text,
bounded history and transactional rejection, model guards, keyboard behavior,
geometry/background, Create/Join/Map caller routes, retained and reset contexts,
and asset failure. Installed-art smoke checks and visually inspected captures
are retained at `working/tests/multiplayer-lobby-preview/host.png` and `join.png`.
Original-manifest verification surrounds original-derived artifact use.
Discovery, connection errors, authoritative rosters/settings, delivery and
readiness synchronization, host migration, wizard catalogs and the engine
command adapter remain outstanding.

Portrait/colour selections now accept explicit artwork indices (0..11 and 0..7,
-1 for text fallback), independent of opaque player IDs. Original SPR portraits,
colour tokens and host boot controls have [integration evidence](menu-sprite-integration.md).
Local/remote permissions are preserved; art selection does not imply networking
or original wizard/catalog binding.
