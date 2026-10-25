# Cursor resource provenance and owned native painter

Build-specific findings use NoCD `40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Static resource provenance is high confidence within the named instruction
windows. Native ownership below is an intentional policy with private resources;
it does not promote the original alias recovery or install a game hook.

## Original resource provenance

`tools/inspect-cursor-resources.py` verifies the immutable-input manifest before
and after inspecting the pinned image. `resources02.json` retains the complete
setup, reload/constructor and wrapper creation/release instruction windows plus
383 direct-operand references. This linear disassembly census can include data
and misses derived/indirect addresses. It is a discovery aid, not a proof of all
writers. Earlier exploratory `direct-map01.json` used a broader substring match
and has 387 candidates; its four additional candidates are not new alias proof.
The exploratory full listing was initially written as physical
`setup-instructions01.txt`:the writer does not manage that suffix. The final
storage correction retains its exact bytes and original hash in SQLite behind
`setup-instructions01.bin`;use `coverage_evidence.read_text` for that reference.
The old Git version and both explicit path transitions remain accounted for.
This storage correction grants no new engine validation or historical compliance.

The setup entry is **4a30b0**, ending at4a371b. The adjacent4a3720 routine initializes
a mouse sample ring; it is not cursor surface allocation. Setup builds an INI
path from the current directory, reads atlas dimensions clamped to0..1000,
and loops over22 descriptors at642678 with68-byte stride. Hotspots clamp to0..40,
frame tokenization writes at most12 indices, and countdown reload clamps to0..1000.
The repeated integer conversions and missing-key preservation are also covered
by the separate full caller comparison described below; the static census alone
is not that comparison.

The resource tail starts at4a35f2:

| Resource | Original storage and construction |
| --- | --- |
| Saved background | Static wrapper641fc8;58ad90 with a zeroed108-byte descriptor,flags7,width40,height40,caps40;58bc10 fills31. |
| Cursor atlas | Static wrapper642000;58ad90 with the configured dimensions and otherwise matching descriptor;fill31,set source RGB key(0,0,255) through58d0e0,load `bitmaps\\cursors.bmp` through58d1a0. |
| Reload marker | Atlas+40 is set toffffffff; reload4a2f90 targets the global atlas. |
| Cell grid |641a8c is configured width/40;642c50 is configured height/40,using signed division. |
| Cursor mutex |CreateMutexA(0,0,0) result is assigned to642c58; setup returns1 without checking the resource calls or unwinding them. |

Compiler-generated one-shot exit callbacks use bits in641a70 to retire saved and
atlas wrappers. These instruction anchors do not assign whole undiscovered
functions. Disposal4a37d0 closes/clears642c58 and calls58b3e0 on atlas,then saved
background. As already executed by `DRAW.cursor-resource-disposal`, it leaves the
original deferred queue untouched.

The already recovered `RS.canvas-release-alias-lifetime` and
`RS.canvas-retained-interface-writers` explain why simply adding references to
wrapper+4/+8 is insufficient.58b3e0 repeatedly calls Release until zero or its
shared1000-attempt cap,then clears the wrapper/registry. Held later interfaces
and CPU/DC writers are independent lifetime dimensions. In the native facade,
IUnknown queries select the Surface1 alias,so IUnknown is not an independent
retirement anchor either. No new general ownership claim follows from those
selected retained-interface fixtures. The diagnostic
`runtime/drawing/source_cursor_gate.h` disclaimer remains correct.

## Complete setup caller comparison

`reconstruction/rendering/cursor_setup.hpp` reconstructs the full1643-byte setup
caller against explicit CRT,OS and resource services. `tests/cursor-setup-reference.cpp`
executes the complete original4a30b0 function unchanged and compares its complete
22-descriptor state,wrapper fields,grid/mutex globals,return value and every
endpoint call with the model. Only external service entries/imports redirect to
scripted endpoints. Resource creation/fill/key/bitmap internals are not part of
this caller comparison.

Optimized and ASan/UBSan32 each pass528 cases:132 input schedules times four
current-directory forms,including empty and both trailing separator forms.
Each case requires90 profile calls,two resource allocations and22 descriptors.
Inputs cover negative/zero/below/at/above clamp thresholds,missing-key preservation,
empty frames,assignment suffixes,signed frame indices and14-token inputs made with
the original delimiter. Separate assertions require the12-frame cap and preservation
of the unused thirteenth frame word. Resource-call scripts leave selected creations
unbound,and mutex creation returns either zero or an identity; caller ordering,
unchecked continuation and return1 still match. Both immutable-input manifest
bookends pass. setup01 predates the explicit cap assertions;setup02 is current.

Real CRT/profile parsing,physical allocations,COM failure reachability and invalid
memory remain separate from this valid-memory scripted-endpoint comparison.

## Native owning policy

`renderer/cursor_compositor.hpp/.cpp` owns a private `SurfaceBackend`. Creation
copies canvas and atlas pixels into distinct RGB565 allocations and allocates an
independent saved-background surface. Callers never receive backend IDs, COM
interfaces, mapped rows, locks or DC leases. Thus no original wrapper destructor
or external alias writer can retire or mutate these resources.

A generation token names exactly one canvas/atlas/save-under triple. Tokens are
never reused and are distinct across compositor instances. A publisher holds
only shared metadata, copied frame rectangles and positions. It can outlive the
compositor. Foreground and timer producers publish through the same mutex;
OpenGL draws, background refresh and destruction execute only on the creating
thread. Wrong-thread owner operations refuse before accessing graphics state.

The queue accepts at most128 frames, returns an explicit full/invalid/closed/
retired result, visits priorities0..2 and preserves publication order within a
priority. A drain bounds itself by its starting publication sequence; concurrent
new work waits for the next drain. These policies intentionally differ from the
original borrowed-wrapper queue's silent drops, stale null rectangles,slot order
and interface rebinding. They must not be described as recovered baseline behavior.

Retirement refuses new jobs but retains accepted generations until their queued
work finishes or is cancelled. A replacement generation cannot redirect an old
job. Owner-thread collection frees only retired,drained triples. A full background
refresh restores the prior underlay,replaces the canvas,then saves and paints the
current frame over the replacement background. Each visible frame restores the
old underlay,saves its clipped new underlay and paints source-keyed atlas pixels.
Clipping uses64-bit intermediates,including INT_MIN/INT_MAX positions.

`Publisher::stop()` cancels pending metadata and closes publication from any
thread. A draw already executing on the owner may finish. Owner-thread shutdown
then retires all allocations before returning; producer endpoints remain safe
after actual context/compositor destruction. Allocation failure releases partially
created resources. The eight-generation budget bounds retained allocations.

## Execution and remaining milestones

`tools/test-cursor-compositor.py` runs optimized and ASan/UBSan configurations.
The test independently rebuilds full expected frames from an untouched background
and atlas; it does not use saved-background or renderer copy planning as its
oracle. It covers key holes,edge clipping,extreme positions,refresh/hide,caller
input destruction,owned rectangle values,full queues,priority ordering,retired
queued generations,recreation,foreign tokens,partial allocation failure,
wrong-thread refusal,2400 foreground/timer producer attempts,stop/cancellation,
late endpoints and eight races against actual owner destruction per configuration.
native08 passes714 assertions and187 complete frames in both configurations.
ASan leak detection is disabled for the Qt/driver process; explicit native
surface counts must still reach zero at retirement. Expected complete-frame
denominator is187 per configuration. Accepted concurrent
publication counts vary with scheduling; every accepted job in that control must
drain exactly once and at least four must be accepted.

Early native01..04 results preserve display initialization failures. native05
executes all714 optimized assertions and187 complete-frame comparisons,but the
runner initially required188 and correctly records failure rather than granting
success. native06 preserves a missing .inc dependency refusal;native07 preserves an
ASan/O2 build warning in the existing Qt command decoder. native08 uses the
normal O1 sanitizer configuration and binds the exact behavior/scenario contracts and
complete compiled dependency closure. The optional isolated user/mount namespace
uses a private/tmp for Xvfb when the account's shared tmpfs quota prevents lock
and keyboard-cache creation; it grants no host root privilege.

This change closes the private native painter/queue resource-lifetime contract
within the tested owner-thread graphics model. Complete indirect source-alias/writer closure,physical GPU/context-loss
recovery and installed live semantic cursor takeover remain separate outstanding
work. Existing simple background helpers from screen-services are reused as
research context and are not duplicated or replaced by this native policy.

## Master integration

The cursor implementation commit60070a69 is reconciled with actual master6919cb93,
including the thirty native rendering components. Stable-ID metadata resolution
preserves both parents,all original evidence/source hashes and every exact prior
review. native09 reruns the optimized and ASan/UBSan compositor checks against
the combined CMake tree;both pass714 assertions and187 whole-frame cases,with
171/170 accepted concurrent publications respectively and the same drained counts.
The original setup02 contract and its complete required source hashes remain
unchanged. No original runtime work is duplicated or activated by this merge.

Final reconciliation also includes actual master3a8f633f and its Save/Load capture
recovery. All recorded source bytes,required-source sets,scope fingerprints and
scenario bindings still match native09 and setup02;integration-bindings01 retains
that source check without inventing another experiment. The extended actual
committed-history review covers679 file transitions with exact receipts. The
initial missing .txt artifact receipt remains visible in history01 and is
corrected by an explicit retrospective pending receipt in history02/03;this is
not a claim that a historical gate checked that artifact.
