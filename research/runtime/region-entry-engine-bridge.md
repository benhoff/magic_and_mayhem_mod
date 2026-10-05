# Region Entry engine bridge (UI29)

This increment connects live Qt Main New Game and Region Entry difficulty/Cancel
to the pinned original engine. It supports fresh Celtic region 1 only. Enter and
all auxiliary controls remain disabled in the live Qt widget. The original engine
continues drawing and ticking; native simulation and gameplay balance are unchanged.

## Contract and ownership

[UI28](region-entry-engine-contract.md) establishes the original caller-dependent
callback and confirms fresh-campaign Cancel goes through Realm resume to Main.
V7 (`MNMMCMD7`, 81,920 bytes) preserves every V1–V6 lane and offset, appending a
320-byte engine payload at 44,784. The payload contains caller, depth,
different-region flag, selected difficulty, radio availability, Cancel availability,
region number and bounded CP1252 realm/title strings. Reserved words are zero.
The host reuses the existing action, generation and argument lane. No addresses
or shared host objects appear in the wire contract.

New Game action 22 delegates original Main action 0. Region difficulty action 23
passes a value 0–3 to original radio-group selection `0x4ce730`; action 24 invokes
original Region Entry action 1 (Cancel). Selection updates original control state,
not global committed difficulty `0x689924`. Cancel likewise preserves that global;
original callback controls transition and Realm return flags.

The adapter requires the exact screen-18 receiver/final vtable, initialized original
controls, callback identity, four radio tags/pointers/selection states, button
identities and availability. It also requires depth 4, Realm parent/caller 4,
current-region flag false, region 1, Celtic realm and context 5, an uninitialized
Realm with no pending custom return, and Main below Realm. These restrictions are
intentional bridge policy; they are not claims that other original callers are
invalid. Invalid pointers, contradictory selections, unsupported callers and
loaded Realm state cannot authorize this bridge.

Commands run on the first observed engine thread, before the forwarded original
tick. Existing readiness, generation, one-outstanding-request, heartbeat lease
and consume-rejected-ID semantics remain in force. Installation checks original
callback/selector bytes and the final Region Entry common-tick slot before its
isolated table write. That slot forwards to original `0x5595d0`. The separate
campaign diagnostic probe retains its V1–V6/no-channel behavior; V7 owns this tick
slot, avoiding two competing wrappers. Diagnostic logs are not command channels.

`RegionEntryWidget` emits semantic difficulty/Cancel signals. The controller owns
pending draft/Cancel intent and suppresses signals during engine projection.
Session orchestration waits for original acknowledgements and owner transitions;
Cancel targets Main because this increment supports only the confirmed fresh
caller. Window close requests Cancel, waits for Main readiness, then original Quit.
Widgets contain no game addresses, staging logic or Wine details.

## Validation and boundaries

[Machine-readable evidence](region-entry-engine-bridge.json) records exact tested
source hashes and the pinned original hash. Targeted validation includes:

- Private PE32 fixture executes original four-choice radio selection and Cancel,
  with synthetic transition services. It rejects malformed snapshots, loaded Realm,
  wrong caller, invalid choice, unavailable radio, stale generation, transition-time
  requests and repeat IDs; verifies radio states, LastError and unchanged committed
  difficulty.
- Six focused Qt tests cover V7 decoding/request guards, widget projection/draft
  handling, disabled Enter/auxiliary/Escape, V1 request regression, V6 Preferences
  bridge/controller and existing Region Entry preview behavior.
- V6 original Preferences callbacks and optional campaign diagnostic forwarding
  regression retain their previous scopes.
- Bounded isolated Xvfb/Wine live Qt test exercises New Game, all four difficulty
  choices, Cancel to Main, reopening and window-close Cancel/Quit, checking original
  callback trace, acknowledgements, consistent engine thread, channel retirement
  and successful launcher exit. No manual interaction is required.

Original-manifest checks surround original consumption; disposable staged EXE/DLL
hashes are checked again after live observation. No original files are edited.
Historical UI27/UI28 fingerprints are retained; source edits make their shared
observer evidence stale rather than silently refreshing historical results.

This is scoped live action integration, not original/native engine equivalence or
replacement. Loaded Realm navigation, other regions/callers, Enter/world loading,
auxiliary identity/lifecycles, campaign Mini mode 4 and live pause remain pending.
Next: validate original Enter and the world-setup handoff in a bounded observation
before exposing it as a Qt command.
