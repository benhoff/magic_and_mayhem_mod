# Region Entry contract and original return observation (UI28)

Build: No-CD SHA-256 `40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Confidence is high within the isolated action branches and the single fresh-campaign
observation below. Qt campaign commands and battle launch remain pending.

## Identity and selected lifecycle

Region Entry is screen 18 at `0x6578c0`, with final vtable `0x5c667c`.
Constructor `0x4b2730` temporarily assigns base vtable `0x5c6564`; that is not
the operational Region Entry table. The final table prefix is:

| Offset | Target | Reviewed scope |
| --- | --- | --- |
| +0 | 0x4b2960 | Enter clears deferred start; initializes controls when needed |
| +4 | 0x4b2990 | Leave invokes cleanup and clears pending screen |
| +8 | 0x4b29d0 | Static export only |
| +12 | 0x4b29b0 | Static export only |
| +16 | 0x5595d0 | Common tick, forwarded unchanged by opt-in probe |
| +20 | 0x4b35c0 | Escape delegates to Cancel; otherwise base input |
| +24 | 0x4b38c0 | Control processing, static export only |
| +28 | 0x4b29e0 | Control registration, static export only |
| +32 | 0x4b2ae0 | Cleanup, static export only |
| +36 | 0x4b2c10 | Builder, static export only |

This prefix does not establish complete dispatch recovery. Builder allocates four
radio choices with IDs 0–3 and clamps the starting global difficulty `0x689924`
to that range. Selected control pointers use receiver +0x5f, selected index +0x63,
count +0x67 and control +0x2d. Receiver +0x6f is region, +0x73 realm name,
+0x77 the different-region flag, +0x7c caller, and +0x80 deferred world start.
The first Celtic region statically disables standard-button indices 1 and 2;
button labels and callback ordering require reconciliation before native dispatch.

## Original callback

`0x4b3420` is thiscall(receiver, unsigned action), returning zero.
For callers other than 22 it reads selected difficulty before dispatch. Caller 22
uses zero initially; action 4 explicitly rereads the group even for that caller.

| Action | Original effect checked in private original-code fixture |
| --- | --- |
| 0, Enter | Saves difficulty. Caller 4 invokes `0x54eec0` with region in ECX, requests transition and return. Other callers set participant counters and +0x80 deferred start without immediately transitioning. |
| 1, Cancel | Requests transition and sets +0x43 return. When +0x77 is zero, sets Realm custom return `0x659e3f` to 1 and clears battle flag byte `0x659e50`. With nonzero +0x77 both fields are preserved. |
| 2 | Transition, save difficulty, pending receiver `0x6e0088`; realm-kind/region request globals populated and request flag set. |
| 3 | Transition, save difficulty, pending receiver `0x6f2aa0`; selected wizard pointer bound using stride 0x93a and related flag cleared. |
| 4 | Transition, reread/save difficulty, pending receiver `0x6c5148`. |
| Other | No dispatch side effects; returns zero. |

The auxiliary destinations above are address identities. Their full control-label
mapping, destination lifecycles and return paths are not confirmed by this result.
Do not infer a bridge command from preview navigation names alone.

Deferred hook `0x4b3880` consumes nonzero +0x80, invokes world preparation
`0x470950`, requests `0x6cbb78`, transitions and marks return. The test privately
stubs world preparation; it does not start a real world. Realm resume `0x552210`
consumes a nonzero custom return at Realm +0xa37, clears it and `0x657d37`, then
pops through `0x557130`.

## Executed evidence

[Machine-readable result](region-entry-engine-contract.json) preserves source
fingerprints and original hash, static export manifest, isolated original execution,
synthetic forwarding fixture, Preferences regression and bounded live trace.

- 168 original callback cases: callers 4/22/3, four difficulties, both different-region
  states, actions 0–4 and two invalid actions. Dependency calls use private stubs.
- Four original deferred-start cases and three original pending-Realm-return cases.
- Synthetic PE32 probe tests cover byte/slot/receiver/repeat guards, result and
  LastError preservation, idle deduplication, and pre/post resume recording.
- Existing V6 Preferences observer regression passes with the optional probe disabled.
- Automated Xvfb/Wine run clicks original New Game, all four difficulty controls
  (1, 2, 3, 0), and Cancel. Region Entry is caller 4 with different-region false.
  Realm resume sees return 1, consumes it, and ownership becomes Main screen 3,
  depth 2. Realm resources never become initialized.

Fresh-campaign Cancel therefore returns to Main. This does not validate a return
to a loaded Realm map. No manual testing was needed. Original-manifest checks
before and after original consumption preserve all 2,927 files; staged EXE/DLL
hashes also remain unchanged during observation.

The diagnostic channel is opt-in through the existing campaign-observation option.
It forwards Region Entry common tick and Realm resume without replacing their
bodies. Campaign records are `MNMCAMP2`, version 2, 128-byte records: phases 1/2
remain pre/post tick and phases 3/4 add pre/post resume. Decoder retains version-1
support for historical evidence. Existing `MNMMENU1` records add events 9/10
for pre/post Region Entry tick: argument is selected difficulty, result packs
caller ID with the different-region byte shifted 16 bits. These are diagnostic
records, not the Qt command wire contract; unchanged states are deduplicated.

Earlier attempts using the transient base vtable failed readiness checks; another
attempt moved and pressed too quickly for original hover polling. These are
harness failures, not recovered game behavior. The final probe uses the constructor's
final table and the test separates pointer motion, button press and release.
Historical UI26/UI27 fingerprints remain unchanged; their affected source hashes
are now stale. UI28 records fresh scoped evidence without asserting whole-bridge
equivalence.

## Next bounded step

Project Region Entry caller, readiness and selected difficulty into a versioned
semantic bridge, then dispatch difficulty and Cancel on the engine thread with
acknowledgements. Validate fresh-campaign return to Main automatically before
adding Enter, real world initialization, loaded-Realm return or auxiliary actions.
