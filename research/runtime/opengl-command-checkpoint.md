# Complete owned checkpoint attachment

Native policy, separate from original-game equivalence. The Qt lifecycle service
exposes `attachCheckpoint()` for a consumer that has not read initial history or
for an active consumer requesting a fresh complete state. It cancels/discards
its prior ring/consumer and clears the frame, creates a distinct file with a
higher session ID, and requests administrative CHECKPOINT=2. Original drawing
continues through the installed callbacks. The initial producer and host still
establish their control channel together; independently joining external host
processes are outside this milestone.

The administrative worker closes the existing bounded callback admission gate,
checks observed lock/DC leases, shuts down and joins the prior publication worker,
then validates the complete retained state under the tracker. No original COM
query, Lock, GetDC or borrowed pixel pointer supplies checkpoint data. Every
observed surface must have a complete owned native pixel buffer and current
pixel/metadata epochs, known layout and unique observed identity. Exactly one
primary is required. Pending pixel misses, alias retirement, incomplete palettes,
unknown indexed bindings and borrowed locks/DCs refuse. This deliberately refuses
metadata-only surfaces rather than inventing initial pixels or silently omitting
an observed resource.

Admission limits the checkpoint to 32 surfaces, 16,777,216 total pixels, 32
complete palettes and the 32 MiB private publication queue, reserving orderly
cleanup. This serialization limit is stricter than the 64 MiB CPU cache limit.
The host does not acknowledge candidate bytes before READY, so admission cannot
rely on an actively draining reader. Serialization copies independently owned
pixels into the bounded queue while the gate remains exclusive. All CREATE,
palette CREATE/BIND and initial colors precede the checkpoint primary PRESENT.
The worker then starts and READY is published through the matching response
sequence. Qt reads only after that response, and activates input/presentation
only after a complete PRESENT.

The fresh wire namespace resets surface/palette IDs and archive sequence, but
keeps verified CPU pixels, observed interface aliases, immutable palette lifetime
generations, clip/source-key/attachment metadata and native mutation guards.
Following partial writes, admitted fills/copies, palette updates and flips advance
that retained authoritative state. Ordinary RECOVER=1 and the existing explicit
RenderRecover export still discard old state and wait for fresh observations.
CHECKPOINT is strict: invalid state refuses and falls back; it does not silently
switch policies. Both operations share the three-request launch budget and
existing reply/frame deadlines and stale session/file/acknowledgement guards.

Synthetic PE32/Qt validation deliberately leaves initial consumer history unread,
checks three fresh checkpoints each for RGB and indexed/shared/aliased palettes,
then compares every displayed native pixel against independent original fixture
buffers after partial negative-pitch locks, failed-call retries, fill, keyed and
unkeyed copies, flips and RGB/flags-only palette writes. Original metadata call
counts do not increase during checkpoint attachment. Retired ring files remain
immutable. Incomplete initial surfaces, partial-only extra surfaces, incomplete
palettes, held locks/DCs, excess surfaces, oversized checkpoint serialization and
uncertain metadata are refused. Terminal consumer resources are zero and ordinary
GPU execution performs no readback or CPU viewport upload.

No actual-game checkpoint/driver equivalence or replacement claim. Allocation,
worker startup, adversarial file changes, unobserved interfaces/leases, concurrent
checkpoint callback collision, implicit destruction and external host takeover
need additional validation. Existing host recovery collision/refusal tests remain
separate synthetic regression evidence. Interrupted archive prefixes can end
without END when their transport has already been cancelled; they are diagnostic
prefixes and cannot be replayed as complete evidence sessions.

## Continuous working-set policy update — 2026-10-06

[Finite consumer residency](opengl-resource-working-set.md) supersedes the eager
32-surface checkpoint materialization described above. Strict admission validates
complete owned state across the existing 128-entry producer tracker, retaining its
aggregate pixel/serialization and borrowed-state guards. New sessions materialize
a complete primary/palettes before READY and admit retained complete offscreen
dependencies when used. The earlier results and source hashes remain historical;
the new strict matrix includes three successful 33-owned-surface checkpoints.
