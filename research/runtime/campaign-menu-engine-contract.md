# Campaign menu entry contract

Build: No-CD PE SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
All addresses below are preferred build VAs, not portable runtime pointers.
Evidence: [UI26](campaign-menu-engine-contract.json). Confidence is high within
executed isolated branches; initialization and complete navigation remain static
findings. This chunk recovers the contract; it does not activate a Qt bridge.

## Main New Game

Main action `0x004a75c0`, local index 0, selects the New Game branch at
`0x004a7611`. The original performs, in order:

1. Main transition helper `0x00557510`.
2. Copy `Celtic` into `0x006dde58`, then configuration reset `0x004ca460`
   on `0x006ddd58`.
3. Set campaign context (`0x0068991c = 0`, `0x00689920 = 5`); reset the
   wizard manager `0x0065b250` through `0x00593be0`, then load `_start.wzd`
   through `0x00590c10` with arguments 0 and -1.
4. Reset controller `0x006f2aa0` through `0x005755d0`, bind its wizard manager
   at `0x006f2b30`, and clear `0x006f2cf0` and byte `0x006f2d08`.
5. Request Realm Viewer `0x00659408` through Main's pending slot, load its
   embedded realm state `0x00659e51` through `0x0054e230`, clear counters
   `0x006e2034/38`, then reset scripts through `0x0056cf00`.

Seven isolated original-action cases cover repeated New Game from dirty state,
Load Game, Quick Battle and invalid indices. Dependencies are privately stubbed;
the test checks ordering and arguments, not real configuration/wizard loading.
Load Game uses a different pending target (`0x006de4e8`) and caller/context
state. A future bridge should delegate the original action rather than recreate
these writes in a QWidget.

## Realm Viewer ownership and lifecycle

Startup constructs receiver `0x00659408` through `0x0054fb30`, installing
vtable `0x005c6a60` and screen ID 4. The six reviewed prefix slots are:

| Byte slot | Target | Recovered role |
| --- | --- | --- |
| 0 | `0x00552210` | Enter/resume; pending-return branch or original setup |
| 4 | `0x005522f0` | Cleanup and original media/resource helpers |
| 8 | `0x004c1370` | Forward to receiver's slot 4 |
| 12 | `0x004c1380` | Forward to receiver's slot 0 |
| 16 | `0x005517a0` | Custom tick/state machine |
| 20 | `0x00552310` | Window-message input |

This is a reviewed prefix, not a complete vtable claim. Realm state is embedded
at receiver `+0xa49`; custom return is `+0xa37`, custom tick state `+0xa2b`,
selected region `+0xa33`, and auxiliary callback storage `+0xa23`.
The Main/Preferences `+0x33/+0x43` readiness rules must not be reused: those
positions overlap Realm Viewer resource storage. The original custom tick must
remain in control until its lifecycle has separate runtime evidence.

Initialization probes sequential silhouette files under
`Interface\\RealmViewer\\Generic`, stopping at the first failed open, and stores
the resulting visible-region count at `+0x823`. Zero count takes an error/pop
path. Positive count builds original resources, lookup data and controls.
The CFG `regioncount` values (Celtic 10, Greek 15, Medieval 17) are distinct from
this asset-derived count and the Qt preview's art counts. Do not infer that a
missing preview region can be enabled merely by changing a count.
Full initialization, hit-map construction, tutorial handling and resource
failure cleanup were inspected statically and have not been executed by UI26.

## Region admission

Original `0x0054e690` counts eligible wizards at a region using realm-state
wizard count `+0x104`, eligibility bytes `+0xd4c` and location bytes `+0xfcc`.
Twenty-five cases cover negative/zero counts, ordinary counts and an 80-entry
fixture. Original `0x0054eec0` accepts the region in ECX, with no stack argument
or object receiver; 216 cases establish two selected branches:

- If the player is already there, the owner differs from the player and that
  owner is also there, request battle mode 4, set the battle flag, and record
  region/player/owner at `0x006f2d0c/10/14`. This bypasses the capacity branch.
- Otherwise fewer than four eligible occupants records the player's destination
  at `0x0065af5d + player`; four or more preserves it and requests sample 99999.

These are admission-helper contracts, not the complete region-click contract.
The tick also uses ownership, adjacency, hit-map IDs, tutorial and transition
state. Native availability/Region Entry remains unimplemented.

## Auxiliary actions and returning

The original auxiliary callback `0x0054fd20` takes one stack index (stdcall),
requests cue 22, then sets one flag for indices 1 through 4. Seven cases cover
all valid indices and ignored values. Static tick destinations are:

| Callback index | Flag | Original destination |
| --- | --- | --- |
| 1 | `0x006f2d28` | `0x006f2aa0` (Portmanteau/Spell Selection) |
| 2 | `0x006f2d34` | `0x006c5148` (Grimoire) |
| 3 | `0x006f2d2c` | `0x006e0088` (Character, with original bookkeeping) |
| 4 | `0x006f2d30` | Mini Menu `0x006a5088`, mode 4 |

Destination addresses/flags are confirmed statically; names follow original
control tooltip/build order. UI26 does not execute the full auxiliary pushes.
Escape (`WM_KEYDOWN`, key `0x1b`) also pushes Mini Menu in mode 4. Fourteen
navigation cases check that ingress, ignored messages/keys, the original
system-key V/v flag, and the pending-return branches in enter/tick: clear
`+0xa37` and `0x00657d37`, then invoke the original pop dependency once.
They do not establish the producer or live safety of a return request.

The [current Mini Menu adapter](mini-menu-engine-bridge.md) supports battle
contexts and excludes mode 4 and campaign context 5. Realm Viewer has no proven
direct Cancel-to-Main callback. Its Options route opens Mini Menu first, rather
than Preferences directly. Do not force battle context or synthesize a pop to
make these paths appear supported.

## Reproduction and next chunks

```sh
python3 tools/export-campaign-menu-support.py
python3 tools/test-campaign-menu-contract.py
```

Both tools pin the executable and verify all 2,927 immutable originals before
and after consumption. The oracle executes original code in a private 32-bit
mapping, verifies dependency bytes before patching that private mapping, compiles
with warnings as errors, and bounds execution to 20 seconds. No installed PE is
modified and no real game is launched. Export output retains disassembly,
read-only Ghidra functions, configuration summaries and source/artifact hashes.

1. Observe original Main New Game ingress and Realm initialization/ownership in
   a bounded run; keep original resources and custom tick.
2. Recover and test campaign Mini mode 4 confirmation/return before admitting
   Escape or Options through the live adapter.
3. Add a versioned semantic Realm projection/dispatcher and Qt controller for
   those proved paths; retain unsupported controls behind original fallback.
4. Recover Region Entry, adjacency/visibility and progression separately before
   exposing region launch or replacing simulation work.
