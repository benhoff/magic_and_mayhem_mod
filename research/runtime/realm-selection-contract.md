# Settled campaign Realm region selection

Build: No-CD SHA-256 `40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Original preferred VAs are build-specific. This increment reconstructs selected
state-0 branches of original custom tick `0x005517a0`; it does not activate
native Realm ownership or grant a sample campaign's availability to a live game.

## Confirmed selection contract

The original samples the current input-ring position, scales from either 640x480
or 800x600 to the 800x600 hit map at receiver `+0x59`, and reads an unsigned byte
region ID. Outside the canvas the region is zero. Region zero is a sentinel:
it performs no selection, even when a prior press remains latched.

For a nonzero hit, normal campaign selection admits a player-owned region or
a region adjacent to any player-owned region. The original loops source regions
1 through the asset-derived visible count at `+0x823`. Its adjacency lookup is
`receiver + 0x1df + destination*4 + (source-1)*0x50`: the source stride is
20 DWORDs, not the native preview's name/image count. Alternate mode
`0x006e0080 != 0` bypasses ownership/adjacency and uses the separate occupancy
branch. No claim is made about all producers or live callers of that mode.

When the tutorial pointer `+0xa44` is nonzero, the region press latch is retained
without region selection. Otherwise an eligible hover requests cue1. A pending
Portmanteau/Character/Grimoire/Options flag clears region latch byte
`0x006f2d3d`; original auxiliary dispatch then retains its own priority.
With no pending auxiliary, holding the original button sets that latch. Release
with a prior latch clears it and requests selection sample801:

| State | Original selection result |
| --- | --- |
| Normal mode, owned region | Set the player's requested destination |
| Normal mode, adjacent enemy region | Set custom state1, selected region `+0xa33`, and realm kind `+0xa2f`; Region Entry occurs in later original states |
| Alternate mode, fewer than four eligible occupants | Set requested destination |
| Alternate mode, four or more eligible occupants | Retain destination and request sample99999 |

An ineligible hover or region zero retains the old latch. Thus a previous press
can be released over a different eligible region. A native semantic command
must preserve the intended transaction policy explicitly; directly writing a
selected-region field would omit these recovered branches.

## Reconstruction and validation boundary

`reconstruction/campaign/realm_selection.*` consumes already-resolved valid
ownership, adjacency and eligible occupancy. It predicts the admission, latch,
cue and destination/entry/refusal effects. The service has no Qt, legacy-pointer,
input injection or progression dependency. It does not parse the live hit map,
validate resource readiness, change a campaign, or implement original travel.

The private oracle maps the complete hash-pinned original executable and keeps
the Realm tick and occupancy counter unchanged. It verifies expected dependency
bytes before patching only private resource/movement/audio/render/auxiliary-push/
tutorial-cleanup targets. Music-repair state, tutorial click latch and automatic
battle-ingress movement state are constrained to isolate the selected branch.
Both immutable-original manifest checks surround every run.

[UI47 scope-bound isolated-original result](realm-selection-bound-original-20261012.json) passes
15,360 matrix comparisons, 54 adjacency/count boundary cases and 14 canvas-edge
cases. Complete original tick and occupancy-counter bytes remain unchanged in
the private mapping. Source fingerprints remain stable across execution. This
records scoped original comparison of the reconstruction, with no live map
integration or replacement promotion.
The prospective declaration was captured and verified before execution; the
result binds its exact behavior contract and complete declared source closure.
The earlier [unbound result](realm-selection-original-20261012.json) retains
its original hashes and does not assert current-source validation.

The matrix compares two player indices, region zero/3, both resolutions,
ownership/adjacency, normal/alternate mode, six occupancy boundaries, pressed/
released and both region latches, no auxiliary or each of four requests, and
present/absent tutorial overlay. Live readiness, complete input ABI, tutorial
generation, arbitrary maps, victory/progression, map rendering and native command
dispatch are separate milestones. Failed fixture runs remain under unique
`working/tests/realm-selection-contract/run-*` paths.

```sh
python3 tools/working-retention.py run --reserve-gib 0.1 -- python3 tools/test-realm-selection-contract.py --claims research/runtime/coverage/planned-realm-selection-20261012.json
```

## Campaign menu work order

1. Realm: validate settled resource/ownership snapshots and original mode4 Mini
   returns using a genuinely progressed campaign; then add guarded semantic
   map projection and actions. Fresh New Game reaches Region Entry before map
   initialization; Cancel returns Main. Preserve that confirmed behavior.
2. Character: recover original six stat identities, price/index/refund behavior,
   authoritative wizard snapshots and OK/Cancel persistence before connecting
   the existing native editor.
3. Campaign Portmanteau/research: recover campaign modes, current inventory,
   knowledge/cost identities and original return paths independently of the
   Quick Battle selector. The separate research preview is not a recovered
   original screen or research command.
4. Grimoire: extend beyond fresh Celtic Introduction/Forest of Pain to progressed
   campaign knowledge, dynamic fields, later chapters and additional callers.
5. Campaign results/confirmations: add victory/progression reports and remaining
   modal/error returns with separate evidence for each path.

Each increment requires its own coverage links, execution evidence and commit.
Existing preview policies and historical live results retain their independent
scopes; this work does not unlock first-region Character or Portmanteau controls.
