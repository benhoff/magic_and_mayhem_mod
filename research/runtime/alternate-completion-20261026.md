# Alternate canvas recovery and native padding validation

This batch combines recovery commit `7d03bde5` with master `dfff4634` in an
owned worktree. Later master changes receive a separate integration review.
Original alternate ownership and intentional native storage policy remain
independent.

## Original campaign error: confirmed local cause

Fresh [640](alternate-completion-results-20261026/copy-low02.json) and
[800](alternate-completion-results-20261026/copy-high02.json) observations capture
the actual original call and return, not inferred rectangles from a dialog:

| Visible mode | Source and requested rectangles | Destination surface | Result |
| --- | --- | --- | --- |
| 640×480 | 736×480 | 640×480 | `88760096` |
| 800×600 | 896×600 | 800×600 | `88760096` |

Campaign caller `4b2e1c` returns at `4b2e21`. Source wrapper `6a49e0` constructs
full-source rectangles; `58c03a` calls windowed `IDirectDrawSurface::Blt` with
`DDBLT_WAIT`. Both destination rectangles overflow the nominal capture surface
by 96 pixels and return `DDERR_INVALIDRECT`. This is an ordinary campaign
capture destination, distinct from the earlier fullscreen primary failure.
The [byte-pinned local code](alternate-completion-results-20261026/callsite02.json)
agrees with the call observations. Confidence is high for this bounded cause.
The [incremental builder inventory](alternate-completion-results-20261026/builder-discovery01.json)
uses final vtable `5c667c+36` to anchor builder `4b2c10`: 600 byte-checked
direct-CFG instructions include the confirmed call. All old functions/flows
remain unchanged; three computed calls and wider builder semantics remain
unresolved. The prior unassigned body is re-exported as a partial inferred
function, granting no complete-builder or alternate-owner claim.

Both alternate pointer slots remain zero. All reported created threads retain
both pointer write watches. This diagnostic replaces the shutdown execution
watch, and bounded termination is a failed continuation. Original stored
sections remain unchanged; original manifests pass before/after. The initial
reporter-only stack and first call captures remain retained. Eighteen controls
check navigation, diagnostic extent, returned-call state and corrupted attribution.

Original alternate allocation, natural publication, backing initialization,
base/source relationship, computed/bulk/external writers and retirement remain
unrecovered. This observation supplies no global dormant/unimplemented claim.
No original rectangle, global or executable correction is installed.

## Native padded storage and validation

`GpuAlternateCanvas` now accepts bounded native padding. Zero padding retains
one GPU plane. Padded operation uses a visible-relative GPU scratch plane and
zeroed padded backing, copying the completed visible result into its centered
backing rectangle and back to ordinary output before UI. Native `x/y` padding
applies on each side; it is not the original 96-pixel width increment or a
recovered original offset. Exclusive generation leases and retirement remain
owned by the native service. Original pointer slots are untouched.

[Final controls](alternate-completion-results-20261026/native-controls03.json)
cover both video modes, padding 0/0, 4/3 and 96/0, 54 frames, 144 prepared
replays, 94,112,736 pixel checks, every backing guard word, six explicit context
checkpoint/rebuilds, bad settings and teardown. Ordinary frame readbacks remain
zero. Sixteen recovery readbacks are explicit diagnostic checkpoints.

The [asynchronous Apprentice journey](alternate-completion-results-20261026/native-async04.json)
completes 303 native Worlds/copy-backs with padding 4/3 and normal Quit. The
[Save/Load/rebuild journey](alternate-completion-results-20261026/native-storage04.json)
completes 737 native Worlds/copy-backs with padding 96/0, two publications,
complete source capture, display rebuilds and normal Quit. Source declarations
remain stable and private Wine cleanup is verified.

[Original raster comparison](alternate-completion-results-20261026/original-comparison01.json)
replays the captured source history through the padded native owner and unchanged
original raster entries. All four complete 800×600 World checkpoints agree:
1,920,000 RGB565 words, zero mismatches. Two additional authored entry histories
remain separate controls. This is selected World raster equivalence under
diagnostic entry canvases, not independent original presented-session,
simulation/save-state, physical device-loss or complete renderer equivalence.
The asynchronous live result and explicit context rebuild controls do not prove
arbitrary in-flight hardware reset recovery. No fixed performance acceptance is
asserted.

## Accounting and retained attempts

The [incoming history review](alternate-completion-results-20261026/history02.json)
checks seven commits and 3,825 file transitions from `d77015d4` to `dfff4634`.
It preserves 3,438 existing exact receipts and seven append-only journal
transitions, then adds retrospective exact receipts for 380 ancillary binary,
image and log introductions. These receipts grant no past gate or engine proof.
Both parents' evidence records and independent stages remain retained.

Failed fixture compilation, RAM-build symlink header resolution, host NVIDIA EGL
Xvfb failure, missing launcher manifest, and guard-interrupted execution remain
separate negative evidence. The initial fill fixture used an unavailable API;
its exact source is retained. The corrected fixture uses an authored native
upload. The source installation manifest is copied and required files verified.
Two orphaned private control displays are explicitly retired; this does not
change the measured native surface teardown result. Original media is unchanged.

The user explicitly authorized a temporary 88 GiB main-drive guard after the
90 GiB guard stopped execution. Subsequent space recovery permits ordinary
90 GiB checks again. Both worktree relocations are file-level verified, and the
old secondary-drive path remains a symlink for retained absolute evidence paths.
No evidence is published. Final source census, exact reviews and retained SQLite
backups are separate from engine validation.
