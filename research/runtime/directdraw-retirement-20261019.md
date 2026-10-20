# DirectDraw retirement execution — 2026-10-19

The requested retirement was checked against the pinned build's
graphics caller routes, native storage ownership, both launch paths, normal
shutdown and separate renderer acceptance. Disabling Wine ddraw and observing
normal original exit alone do not establish clean reference retirement.

## Confirmed implementation gap: page residency hints

Static original caller `0058aeaa` conditionally invokes Surface2+ slot 37 with
flags zero during offscreen wrapper creation. Slot 37 is PageLock; slot 38 is
PageUnlock. Both were terminal unknown-method routes in the native facade.
Evidence is the unchanged disassembly in
`native-draw-device-surface-discovery-20261010.json`; the executable SHA-256 is
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.

The independent unchanged PE32 caller now exercises Surface2/3/4, idle and locked
storage, five flag patterns, repeated and unbalanced hints, interface references
and every pixel. Wine's selected system-memory calls all return success without
altering the plane or references. Native execution originally terminated at this
route (exit 98). Thirty final cases compare 180 page calls, 480 caller fields and
all 3,510 pixel words exactly, with zero remaining owners. Native owned host
storage now supplies those hints without changing the
drawing lease or pretending that host allocations are physical display memory.
Confidence is high for the pinned static call and measured Wine callers; this is
not a recovery of arbitrary physical-driver residency.

## Shutdown accounting

An earlier source-stable successful normal-Quit journey retained one device and
one surface in its final 96-byte status. That report remains unchanged. A new
independent shutdown record captures all owner classes and live device/surface
origins **before** process reclamation. It never releases a game reference.

The local `MNMDRET1` version-1 record contains a 64-byte header, then 64-byte
device and surface rows. Header words are magic, version, header size, process
termination indicator, device/surface/DC/clipper/palette counts, allocated pixel
bytes, factory/surface creations, forbidden ddraw module presence and two zero
reserved words. Each device row records creator and per-interface references;
each surface row records creator, per-interface references, dimensions, caps,
device owner and outstanding mapping/DC state. Addresses are launch-local
diagnostics, not shared pointers or stable identities.

The menu runner assigns a fresh session-local path. Native-only completion now
requires this receipt, positive factory activity, absence of real ddraw, and zero
devices, surfaces, DCs, clippers, palettes and pixel bytes. Balanced World work and
exit code zero cannot bypass that check. Missing, malformed, symlinked and
inconsistent inventories refuse. Forced termination never supplies normal-Quit
acceptance.

The source-stable 299-World diagnostic reproduced the leak: the temporary
device was created at return address `004bd96d`, its 800x600 primary at
`004bdadc`, and Surface3 and Surface4 each retained one reference. No DC, clipper,
palette or drawing lease remained. The leaked primary retained the device and
960,000 pixel bytes. Confidence is high: the live inventory and original checker
instructions agree.

## Guarded capability-check repair

The original checker acquires Surface3 at `004bdb1a` and Surface4 at `004bdb4f`.
Its successful path releases only Surface1 at `004bdb62`, then the device at
`004bdb6c`; neither queried alias is released. Failure paths also lose stack-only
child references. This original lifetime defect does not justify changing
ordinary COM reference semantics.

Native staging replaces the ten-byte final device Release sequences at RVAs
`000bdb20` and `000bdb65` with a stack-equivalent call through the newly imported
`NativeDrawReleaseStartupProbeV1` thunk. The adapter checks caller and creator
RVAs, the exact idle native primary graph, default mode and reference bounds
before mutation. It releases only the checker's temporary aliases and device,
preserving the original Release return and LastError. Unknown callers, extra
references, active leases and additional children terminate before mutation.
No final-shutdown sweep hides leaks.

All original section hashes and exact replaced instructions are guarded before
staging. Both native launch paths use the repair. In addition to the two prior
12-byte loader-string edits, only these two ten-byte instruction regions change;
other original section and IAT bytes remain unchanged. Eight composed import
configurations, twelve mutations and eight duplicate staging refusals pass the
offline check. No files under `original/` were modified.

Owned PE32 stubs test three temporary graphs (both queried aliases, first query
failure and second query failure), preserved Release/LastError, zero owners and
five atomic refusals. They execute native code at the guarded return RVAs; they
do not execute the original checker or prove original failure reachability.
Seventeen independent Python methods check shutdown/admission and malformed,
missing, inconsistent and leaked-owner receipt refusal. These current combined
controls pass with stable source fingerprints.

## End-to-end results

| Run | Result |
| --- | --- |
| Strict private native-only campaign | 179 balanced Worlds, normal original Quit, no fallback/forwarding or ddraw module; zero devices/surfaces/DCs/clippers/palettes/pixel bytes before reclamation |
| Public Hard restart | Two sessions, one deliberately failed generation, fresh channel identity; successful generation completes three balanced Worlds and normal Quit with zero graphics owners |
| Standalone launcher on NVIDIA TITAN RTX | 33 actual native GPU menu frames, zero ordinary readbacks/uploads/recoveries, ddraw absent, zero consumer resources after bounded finish |
| Factory/surface/service regressions | 128 factory lifetimes, 64 surface cases and 64 DC/clipper cases; original caller fields and complete pixel planes match, zero retained owners |
| Resource regressions | Clipping, five-format storage, palette/DC and HWND-region matrices pass with their existing scoped policies |

The actual original checker produced a positive cleanup receipt before gameplay:
two lost queried aliases, one primary and one temporary device released. Normal
Quit now has positive zero-owner evidence, rather than an inference from process
exit. The failed restart generation receives no rendering or normal-Quit credit.

The live runs are source-stable in their retained isolated tree. Eighteen
unrelated terrain-test/build files changed concurrently in the main checkout and
were included by the older broad declaration wrapper. Their recorded hashes are
preserved; broad live reports consequently remain historical for main-tree
freshness. Current focused native ownership/ABI, Python policy and facade
comparisons are separately executed and registered. No old hashes are refreshed.

Setup failures remain separate: a probe without cooperative level, fixture text
allocation/alignment attempts, an undeclared protocol dependency, a mutated
default working executable refused by the pinned hash, a test symlink copied into
a broken runtime link, an undeclared compiled `.inc` dependency, a wrapper path
error and a concurrent Wine reservation refusal. The symlinks are preserved;
the corrected test installation is materialized. No admission guard was relaxed.
Original manifest bookends verify all 2,927 immutable files. All output uses the
approved 90 GiB retention guard; no cleanup or archiving was performed.

## Remaining acceptance

Real DirectDraw is excluded from both tested native launch paths, and their
confirmed page-hint and semantic-shutdown implementation gaps are fixed. The
broader `NR.directdraw-retirement` aggregate remains unresolved for complete
required caller/writer/alias closure, physical context-loss behavior, whole-game
original pixel equivalence and sustained performance. The unmodified storage
reference's 252-pixel failure remains a separate failing acceptance result.
These runs do not establish arbitrary gameplay, physical driver equivalence,
19/60 FPS acceptance or full native-renderer replacement. The tracker retains
that aggregate boundary while marking the new bounded implementation scopes
independently; a successful native launch is not universal compatibility proof.
