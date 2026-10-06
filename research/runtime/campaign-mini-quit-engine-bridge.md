# Campaign Mini Quit and original confirmation

This milestone adds V10 campaign gameplay Quit beside the V9 Cancel path. Qt
sends a semantic Quit action; the original engine owns the confirmation,
answers, cleanup, World resume and campaign exit. Save, Load and Preferences
remain disabled. No original artifacts are modified and no timer or balance
policy changes are introduced.

## Recovered contract and confidence

The pinned No-CD SHA-256 is
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Static evidence is the retained `working/decompiled/mini-menu-support-gy6guhe9/`
export, checked against executable bytes by the isolated fixture. The
[previous Cancel contract](campaign-mini-cancel-engine-bridge.md) supplies the
confirmed fresh campaign hierarchy: singleton `0x6a5088`, screen 17, mode 2,
five controls, layout 0, context 5, depth 5 and exact Main/Realm/World parents.
The new original-only and Qt observations confirm the following selected
branches with high confidence within that scope:

- Original button callback `0x4b23f0`, local index 3, allocates the original
  confirmation at Mini `+0x3b`, registers the second callback and retains Mini.
- The second callback object is reached through `*(Mini+0x4f)+4`. Its vtable is
  `0x5c6674`, function `0x4b24f0`, owner the same Mini singleton. It uses ECX
  receiver and one stack argument, with `ret 4`.
- No is answer 1. The callback sets return flag `+0x43=1`, destroys and clears
  the dialog, leaves exit bytes `0x6dbc18/19` zero, and the original World
  resume path restores gameplay.
- Yes is answer 0. In this observed mode 2/context 5 campaign, the callback
  clears the dialog and sets byte `0x6dbc18=1`. The original engine exits the
  battle through the original World exit consumer and opens the campaign
  defeat report at `0x6db967` (“You quit the battle”). Its original OK control
  pops through World and Realm to fresh Main in this observed scenario. The label “Quit Game” matches original assets; this gameplay
  dialog does not terminate the process. Subsequent original Main Quit ends it.

The context-5 branch calls `0x4667c0`, `0x474b40` and `0x474b10`. Their full
semantics are not recovered here; older network-only interpretations do not
exclude the now-observed fresh campaign use. Mode 4 and context values other
than 5 remain outside this integration. Escape did not answer the confirmation
in the initial observation; validation selects the original labeled buttons.

## Wire and ownership

`protocols/include/mnm/menu_v10.h` uses magic `MNMMCM10`, version 10 and 94,208
bytes, retaining all previous offsets. The eight-word Mini payload offers
Cancel|Quit, mask 5, when nonmodal, and no actions when confirmation is present.
V9 remains Cancel-only; V4 remains separately experimental. Mode, caller,
stack, original button ownership and second receiver identity are guarded.
No shared host pointer or C++ layout enters the wire contract.

The engine adapter calls only original local index 3 or 4. It never invokes
confirmation Yes or No from a host command and never writes dialog, pending,
return or exit fields. Qt switches to the original viewport after Quit is
acknowledged. No returns ownership to gameplay; Yes leaves the defeat report
and Realm under original control. Native Main is restored only
through a fresh original ready snapshot with matching thread and acknowledgement.
The existing native Main Quit then uses the original shutdown path.

For evidence, `campaign_quit_observe.h` replaces only the function field of
that registered second receiver with a forwarding observer, after validating
its identity and eight original prefix bytes. It forwards return and LastError.
Events 21/22 record confirmation entry/return; 24/25 record context and the two
exit bytes before/after. Bounded World tick events 26/27 observe the consumer
and event 28 records the resulting original owner identity. The original Mini
tick installer also verifies the
confirmation prefix before enabling campaign observation or V10. The guard
accepts the original receiver or this private forwarding wrapper. This is a
scoped disposable observation hook, not native confirmation replacement.

## Automated validation and boundaries

[Machine-readable evidence](campaign-mini-quit-engine-bridge.json) retains fresh
source fingerprints, original build hash, fixture reports and live artifact
locations under new UI32 evidence IDs. Earlier UI31 and other evidence hashes
remain unchanged; shared source edits make some historical evidence stale.

The private PE32 fixture executes pinned original Quit/Cancel and No/Yes
callback bytes. Allocation, dialog setup/cleanup, audio and the context-5 helper
calls are synthetic, as are common tick and World resume bodies. It checks
mode/layout/count/stack/receiver ownership, stale generations, modal Quit,
one-shot dispatch, retired channels and incorrect confirmation prefixes.
Executable protection is explicitly established for every synthetic helper.
This supports only the tested callback branches, not those dependency semantics.

Eight focused Qt tests check V10 wire/action/controller guards, V9 Cancel and
V4 compatibility, the Mini widget, Region Entry and the existing V1/V6 bridges.
The V9 original Cancel fixture is rerun as a regression check.

Original-only observation runs New Game through Enter, opens Mini with original
Escape, chooses Quit/No and observes World resume, repeats Quit/Yes, and
acknowledges the original defeat report with OK. It then requires fresh Main
readiness after the original World/Realm return. The Qt live scenario follows
the same path using native Quit buttons and original confirmation/report
controls, then requires native Main Quit and
normal launcher exit. The live test translates the confirmed original 800x600
button positions into the centered embedded viewport; this is validation input,
not product code. The Qt harness waits for three initialized World updates before injecting
Escape; Region Enter admission alone can precede gameplay readiness.
Both runs are bounded and verify all 2,927 immutable originals
before and after. Live comparison and native replacement are not claimed.

The initial original-only test incorrectly matched the pre-campaign Main
snapshot. That intermediate result is not used as successful exit evidence.
The corrected test requires a later Main sequence, records defeat/report and
World/Realm return artifacts, and rejects deadline termination for the Qt scenario.

Observation samples selected direct World vtable calls. The alternate wrapper
`0x4757c0` and all simulation/drawing bodies remain original and are not
intercepted by this diagnostic.

The selected consumer reads World `+0x100a0` at `0x46b276` (absolute
`0x6dbc18`), sets `+0x100a1`, clears `+0x100a0`, and calls `0x46a150`
at `0x46b2ae`. The result path pushes World `+0xfdef` (`0x6db967`) at
`0x46b3d1`. Outcome/scoring/helper bodies remain opaque; no native body replaces
them. The observed Battle End is screen 6, vtable `0x5c5ebc`; native result integration
for screen 26 is separate. Realm Mini mode 4 remains unadmitted and untested.

Remaining gaps: native defeat report/Realm and confirmation presentation, Save/Load/Preferences from
Mini, Realm mode 4, loaded campaigns, other campaign completion/result paths,
allocation/cleanup/helper equivalence, and timer/mana/audio pause behavior.
Only entry-prefix recovered ranges are linked where the discovery inventory
lacks a function boundary; unknown bodies and unimplemented dispatch cases remain.

## Native defeat report activation (UI33)

[Separate V11 report integration](campaign-defeat-engine-bridge.md) admits the report only after observed campaign Quit/Yes, copies the original title and all 21 display strings, and sends native OK through original local index 21. Fade and World/Realm return stay original. V10 retains the original report viewport path; victory, other defeats and loaded campaigns remain outside V11 admission.
