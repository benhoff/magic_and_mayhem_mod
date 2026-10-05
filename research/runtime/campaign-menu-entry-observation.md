# Original campaign entry observation (UI27)

Pinned No-CD executable SHA-256:
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
[Machine-readable evidence](campaign-menu-entry-observation.json) retains source
fingerprints, experiment paths, original callback/tick records, artifact hashes,
a synthetic forwarding fixture and the existing Preferences observer regression.
Confidence: high for this bounded fresh New Game path. This is live observation,
not native equivalence or replacement.

## Confirmed entry flow

An automated XTest click uses the installed Main CFG's first button rectangle
(center 400,330), after observing initialized Main ownership without pending
transition/return or fade. The original local-0 callback requests Realm Viewer
`0x00659408`. The forwarded custom tick runs on the same thread as that callback.
It has screen ID 4, vtable `0x005c6a60`, campaign context 5, nine Celtic wizards,
player 0 and last-region field 9. Observed after-tick progression is:

| Tick | State after | Selected region | Active owner | Stack depth |
| --- | --- | --- | --- | --- |
| 1 | 1 | 1 | Realm Viewer, screen 4 | 3 |
| 2 | 2 | 1 | Realm Viewer, screen 4 | 3 |
| 3 | 0 | 1 | Region Entry, screen 18 (`0x006578c0`) | 4 |

The captured original screen is **Forest of Pain**, with difficulty choices,
Character/Grimoire/Portmanteau icons, Enter Region and Cancel. The run never clicks
these controls, starts a battle or requests a campaign return.

Throughout those Realm ticks, `+0x823` visible-region count, `+0x12` borrowed
realm-name pointer, `+0x59` hit-map pointer and `+0xa23` auxiliary callback pointer
remain zero. Thus fresh New Game reaches Region Entry before the Realm map is
initialized. This confirms the early state-0 admission branch identified in
[UI26](campaign-menu-engine-contract.md). It does not prove loaded-Realm readiness.
The base `+8` initialized field is also zero here; common Main/Preferences
readiness rules cannot authorize a Qt Realm handoff.

## Opt-in probe and diagnostic format

`runtime/menu/campaign_observe.h` changes only Realm's tick slot at vtable
`+0x10`, after checking the expected slot and original eight entry bytes. It
forwards `0x005517a0`, preserving the original result register and LastError,
then records post-state. It does not poll a command channel, alter realm state,
skip initialization or perform a synthetic pop. Normal menu sessions have no
campaign hook unless `MNM_MENU_CAMPAIGN_OBSERVE` explicitly selects a new file.
The runner requires campaign-enabled staging, the exact experiment-local path
and no existing diagnostic file.

The diagnostic file starts with `MNMCAMP1`, little-endian version 1 and record
size 128 (16 bytes total). Each record has 32 little-endian DWORDs:

```text
sequence phase thread receiver vtable screen owner depth
 tick_state returning visible_regions realm_name hit_map auxiliary_callback
 wizard_count player last_region initialized fade_active tutorial world_return
 selected_region outcome parent owner_screen context result last_error
 spell_flag grimoire_flag character_flag mini_flag
```

Phase 1 precedes the original tick; phase 2 follows it. Addresses are raw
process-local diagnostics for this pinned build, not a portable Qt command or
state contract. Each phase deduplicates stable state separately; result/error
register differences do not create idle records. At most 256 records are written.
A future semantic Realm projection still needs independent ownership, resource,
transition, tutorial and campaign-return admission checks.

## Validation and remaining work

```sh
python3 tools/test-campaign-observer.py
python3 tools/test-menu-observer.py --preferences
python3 tools/test-live-campaign-entry.py
```

The synthetic PE32 probe checks wrong slot/bytes, receiver rejection, repeated
installation, stack/result/LastError forwarding and 300 idle calls producing
only two before/after state pairs. The existing V6 fixture passes its original
callback and transaction checks with campaign observation disabled. The live
run stages a disposable installation and new Wine prefix, uses a private Xvfb
screen, waits at most 80 seconds for Main, observes campaign ingress for at most
15 seconds, and stops the private process group/prefix. Game execution itself
has a 100-second smoke bound. All 2,927 original files verify before and after
original-consuming tests, and staged executable/DLL hashes remain unchanged.
No manual testing is required.

Earlier failed attempts are retained in `working/tests/live-campaign-entry`:
a sandbox socket restriction and a harness readiness check that demanded an
after-tick record suppressed by existing Main deduplication. The corrected check
accepts either phase with the same Main readiness fields. Those failures are not
campaign behavior evidence.

Next recover Region Entry's original Cancel/Enter, difficulty, caller and nested
auxiliary contracts. Observe its return to a genuinely initialized Realm map
before adding the Qt Realm controller. Campaign Mini mode 4 confirmation/return
remains a separate requirement; the current battle-only adapter stays disabled
for that context. No native menu capability or gameplay policy is added by UI27.
