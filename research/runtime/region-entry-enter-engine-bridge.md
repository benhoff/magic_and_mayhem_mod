# Fresh Region Entry Enter bridge (UI30)

Qt New Game now allows Enter from the fresh Celtic region 1 difficulty screen.
Qt commits the queued difficulty through the original selector before submitting
Enter. The engine calls the original callback and admission function on its
existing thread. The shell then presents the embedded original game viewport.
This is scoped menu integration; loading and simulation remain original.

## Recovered path and intentional restrictions

For the pinned NoCD build, callback `0x4b3420`, local action 0, saves the radio
selection to `0x689924`, calls `0x54eec0` with the region in ECX and requests the
original menu transition. The admission branch compares **32-bit** player and
owner locations at `0x65ae1d + 4 * wizard`, with ownership at
`0x65b1d9 + 4 * region`. The fresh opponent branch sets Realm state
`0x659e33 = 4`, battle flag `0x659e50 = 1` and region/player/opponent globals
`0x6f2d0c/10/14`. Realm tick's original `0x5510d0` handoff subsequently reaches
World receiver `0x6cbb78`, ID 2, vtable `0x5c5dd8`; tick slot `0x5c5de8` points
at `0x46afc0`. Static exports and automated original runs confirm these selected
anchors and destination, not the complete loading or World contracts.

V8 (`MNMMCMD8`, 86,016 bytes) preserves previous offsets. Region action mask bit
2 enables Enter; command 25 requests it. Handoff 3 means the original admission
branch accepted the campaign battle request. It does not claim loading completed.
Availability retains V7's exact receiver, caller, stack, callback, controls,
generation/readiness and one-shot checks, adding initialized/enabled Enter,
nine campaign wizards, different valid player/owner and both locations equal to
region 1. These restrictions are intentional native policy. Other regions,
loaded Realm and occupancy/movement admission branches remain unavailable in Qt.

The V8 World forwarding adapter verifies the original tick pointer and eight
entry bytes before replacing only its disposable process vtable slot. It
preserves the original tick's return value and LastError, records only the first
three tick pairs and maintains bridge heartbeats without issuing gameplay
commands. The original draw and simulation calls remain active. The host keeps
its battle ownership until a supported root menu returns; campaign return has
not been validated and is not claimed by this change.

## Automated evidence

[Machine-readable evidence](region-entry-enter-engine-bridge.json) preserves
source hashes and paths to isolated fixture, static export, Qt checks and live
runs. All original consumption runs verify all 2,927 immutable files before and
after; staged executable/DLL hashes and bridge input versions are checked.

- Private original callback/admission bytecode validates difficulty commit,
  accepted battle globals, unavailable controls/opponent location, generation
  rejection and duplicate command consumption. Transition dependencies are
  synthetic; this is not a full original menu execution.
- Synthetic World forwarding validates wrong pointer/entry rejection, repeat
  install rejection, return and LastError preservation.
- Six focused Qt checks cover V8 Enter, V7 compatibility, queued semantic
  projection, Region Entry widget and V1/V6 regressions.
- An original-only Enter run observes three initialized World ticks on the
  original menu thread. A separate Qt run selects Adept, submits Enter,
  presents the embedded original viewport and independently observes those
  gameplay ticks. A supplementary active Xvfb capture shows the original campaign
  scene and tutorial inside the shell; no pixel comparison is claimed. The run
  ends through a bounded launcher deadline, not original Quit.
- The existing Qt V7 difficulty/Cancel/window-close Quit round trip is rerun.

Run `python3 tools/test-region-entry-bridge.py --enter`,
`python3 tools/test-campaign-observer.py`, focused Qt CTests,
`python3 tools/test-live-campaign-entry.py --region-enter`, and
`python3 tools/test-live-region-enter.py --shell <built-shell>`.
No manual interaction is needed. A dedicated build accepts `--source-root`.

Confidence is high within this fresh-entry scope. Campaign completion/return,
loaded-game callers, other regions, auxiliary menus, campaign Mini, pixel/input
or simulation equivalence and native replacement remain pending. Historical
UI27–UI29 evidence retains its hashes; affected shared-source results become
stale rather than being rewritten as fresh.
