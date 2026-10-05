# Campaign gameplay Mini Cancel bridge (UI31)

The default Qt live-menu shell now presents the campaign Mini when original
Escape opens it during gameplay. Cancel or native Escape invokes the original
campaign Cancel callback, then returns presentation to the original viewport.
Save, Load, Preferences and Quit stay disabled in this scoped native menu.
The toolbar's existing original-menu fallback remains available.

## Recovered contract and native policy

Original fresh New Game establishes context `0x689920 = 5` and layout selector
`0x68991c = 0`. Gameplay Escape enters singleton `0x6a5088`, screen 17, vtable
`0x5c6644`, mode `+0x53 = 2` and button count `+0x57 = 5`. Original Cancel is
local index 4 at callback `0x4b23f0`; it sets the original return request without
requesting another screen. Original shared tick `0x5595d0` pops Mini and invokes
World receiver `0x6cbb78` resume `0x46aef0`. The observed Mini stack depth is 5,
with Main at depth 2, Realm at 3 and World at 4.

The discovery inventory still lacks callback/common-tick entries. Eight-byte
entry exceptions are pinned by the private original Cancel/forwarding fixture;
these are byte-guard scopes, not whole-function recovery. Gameplay input entry
`0x46c070` remains an explicit original candidate and mapping debt. Original
Escape ingress is observed without claiming inventory coverage for that function.

The older V4 notes treated context 5 as a network exclusion. This original
campaign observation confirms that 5 also identifies this campaign path; it
must not be described as network-only. The older experimental battle adapter
and its gate remain separate. Realm Mini mode 4 is also separate from gameplay
Mini mode 2 and remains unsupported by this bridge.

V9 (`MNMMCMD9`, 90,112 bytes) preserves previous lanes/offsets and uses the
Mini payload at 42000. Campaign Mini has battle-layout boolean 0, context 5,
depth 5, parent ID 2, Cancel-only action mask 1 and mode 2 in word 6; word 7
remains zero. Original confirmation makes commands unavailable. No host
pointers, Qt objects or native simulation state cross this channel.

The engine validates the exact receiver/table/ID, layout/context/mode/count,
stack hierarchy and registered callback owner before offering Cancel. It
retains generation, readiness, thread, heartbeat, retirement and once-only
command guards. The V9 adapter verifies the original callback prefix, Mini
tick slot/common entry and World resume slot/entry before installing disposable
process vtable wrappers. It forwards original tick/resume return values and
LastError. Earlier V4 command dispatch remains compile-time gated.

The Qt controller projects the five-button campaign layout and enables only
Cancel. The session acquires native ownership only on fresh engine-confirmed
Mini readiness, then gives original presentation back after acknowledgement.
Opening remains original game input. Native Escape uses original button Cancel,
not the original Escape helper; this establishes functional return in this
scenario, not complete input/redraw/audio ordering equivalence.

## Evidence and limits

[UI31 machine-readable evidence](campaign-mini-cancel-engine-bridge.json)
records exact tested sources and artifact paths. Every run consuming originals
verifies all 2,927 immutable files before and after. Disposable executable/DLL
hashes and compiled bridge input versions are checked.

- Original-only campaign input opens Mini and returns twice, by original Cancel
  and original Escape. Both original World resume calls are observed.
- Private pinned original Cancel bytecode validates guarded V9 dispatch,
  unavailable/modal/stale/retired/duplicate requests and refusal of Preferences
  or Quit. Common tick, World resume and audio dependencies are synthetic.
  Separate slot/entry guards and original-forwarding return/LastError checks
  validate the wrappers without claiming a synthetic pause state.
- Seven focused Qt checks cover V9 wire/controller restrictions, malformed
  payloads, V4 compatibility, Region Entry, V1/V6 and Mini widget regressions.
- Enabled/disabled V4 original-bytecode fixtures are rerun, retaining the legacy
  experimental boundary.
- A bounded automated Qt run enters the first campaign battle, opens Mini via
  original Escape, uses native Cancel and native Escape on separate openings,
  and verifies two original World resumes on the original engine thread. The
  deadline ends the run; it is not campaign completion or original Quit.

Run `python3 tools/test-campaign-mini-bridge.py`, focused Qt CTests,
`python3 tools/test-live-campaign-entry.py --mini-cancel`, and
`python3 tools/test-live-campaign-mini.py --shell <built-shell> --source-root
<compiled-tree>`. No manual interaction is required.

Confidence is high within the observed gameplay hierarchy. Save/load,
Preferences from Mini, quit confirmation/campaign exit, Realm mode 4, loaded
campaigns, mana/timer/audio pause semantics, equivalence and native replacement
remain separate milestones. Original simulation and drawing remain active;
no native timer freeze or balance changes are introduced. Historical evidence
keeps its original hashes and affected shared-source evidence becomes stale.
