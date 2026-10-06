# Campaign Quit defeat report bridge

V11 presents the original fresh campaign Quit defeat report in Qt and sends OK
through its registered original callback. The engine still owns confirmation,
score production, fade, destruction, World/Realm return and shutdown. Victory,
natural defeat, loaded campaigns and other outcomes are not admitted.

## Recovered contract and evidence

The pinned No-CD build is SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
The reproducible `tools/export-campaign-defeat-support.py` export retains
read-only assembly, selected decompilation, vtables and artifact hashes under
`working/decompiled/campaign-defeat-support-565qg4je/`. Static display recovery
is backed by the new original-bytecode fixture and automated native observation.
Confidence is high for the stated fresh Quit path, without whole-function or
scoring equivalence claims.

The original report owner is `0x6db967`, screen ID 6, vtable `0x5c5ebc`.
It differs from Quick Battle results (ID 26, `0x5c5ef4`). Selected report setup
at `0x473d70` builds 21 labels, stores their pointer array at report `+0x4b`,
and forms the title in its inline 256-byte buffer at `+0x57`. Each label's
original display string is reached through label `+8`. The defeat reason at
inline `+0x157` is copied into label 19 (zero based), observed as
“You quit the battle.” The native adapter copies those strings; it computes
no points, ratings, reward descriptions or maximum values.

The report's `+0x53` is zero for defeat. `+0x47` points to the single OK button
array and `+0x4f` to the registered receiver. The receiver has vtable
`0x5c5eec`, function `0x4747a0`, owner the report. The button registers local
index 21, links that receiver through `+0x25` and stores its index at `+0x2d`.
The original OK callback calls `0x557510`, writes report return flag `+0x43=1`,
returns zero and uses `ret 4`. The selected fade-start body calls original
audio and the object's slot `+0x18`, then sets fade byte `+0x0c=1` and counter
`+0x0d=0` when fade is enabled and inactive. Those audio/focus bodies remain
opaque and synthetic in the isolated fixture.

On the observed path, original Yes sets World `+0x100a0`; the original World
consumer moves that to `+0x100a1` and pushes this defeat report. Original OK
then resumes World and returns through Realm to fresh Main. This is the
[previous Quit lifecycle](campaign-mini-quit-engine-bridge.md), now with native
report presentation and the same original OK callback. World/Realm bodies,
cleanup and simulation are not replaced.

## Wire, admission and presentation

`menu_v11.h` retains all previous lanes and adds a 5,648-byte report payload at
byte 94,208 in a 102,400-byte channel, magic `MNMMCM11`, version 11. The four
little-endian fields are offered actions (OK=1), context 5, depth 5 and outcome
0 (defeat). The original title and 21 original display strings follow, each a
bounded 256-byte NUL-terminated CP1252 string. OK uses semantic action 26.
No engine pointers enter the wire contract.

The adapter requires the exact owner/type, defeat discriminator, context 5,
layout 0, World exit flag, five-level stack and exact Main/Realm/World parents,
registered receiver and button identity, and bounded nonempty title/reason.
Native admission additionally requires provenance from the forwarding Mini
confirmation observer seeing original campaign Quit/Yes. This is an intentional
native admission policy, not an original engine field or recovered rule. It is
cleared on fresh ready Main. Unobserved/natural defeat therefore keeps original
presentation. Unsupported, stale, modal, duplicate and retired requests do not
dispatch. Install verifies the expected original OK entry prefix and common
tick slot before substituting only that slot in the disposable installation.

The common report tick remains original, with menu polling around it. Native
OK calls original local index 21 once. Qt hands the viewport back for fade and
World/Realm return; only a fresh engine-confirmed Main snapshot restores native
Main. Closing the Qt window from the report first requests that same OK and
then follows normal Main Quit, with no forced shutdown policy.

The existing BattleResultWidget hosts the report. Its new display-only method
copies all 21 original strings verbatim, including original numeric formatting,
and the original title. Hiding empty labels and placing the native title banner
are presentation policies; pixel/geometry equivalence is not claimed. The
existing preview model and Quick Battle results remain separate.

## Validation and remaining boundaries

[Durable evidence](campaign-defeat-engine-bridge.json) retains current source
fingerprints and original build anchors under UI33 evidence IDs. Historical
UI32 and earlier fingerprints are preserved and may be stale after shared edits.

The private PE32 fixture executes the original `0x4747a0` and `0x557510` bytes.
Audio/focus and outer tick/World/Realm dependencies are synthetic. It checks
report strings and bounded termination, absent labels, provenance/context/type/
stack/receiver guards, stale and modal rejection, unsupported actions,
one-shot dispatch, retirement, original fade/return fields and LastError.
The V10 original Quit/No/Yes fixture is rerun as a compatibility regression.

Eleven focused Qt tests cover the new bridge/controller and exact display text,
CP1252 conversion, malformed fields/strings, generation and one-shot checks,
and existing Mini/V9/V10, Region Entry, Quick Battle results and V1/V6 contracts.
The bounded Wine/Xvfb test runs native New Game and Enter, waits for three
initialized World updates, and uses original Escape and confirmation input.
It checks Quit/No resumes World, repeats Quit/Yes, compares all native report
labels and title against the original wire snapshot, presses native OK and
requires fresh Main and normal Main Quit. No manual input is required.
Original manifest verification brackets every artifact-consuming experiment.

Pending: victory and other outcomes, natural defeat and loaded-campaign
admission, scoring/reward/helper equivalence, native confirmation and Realm,
full callback/tick dependency reconstruction, pixel equivalence and live
replacement. Static selected setup/draw addresses `0x473d70` and `0x474940`
remain candidate links with their discovery gaps preserved. A linked callback
or fade body covers only the stated scope; it does not recover the whole screen.
