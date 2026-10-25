# Ten named native sprite services

This batch supplies a widget-independent consumer for the partial TAG/SPR
resource path. Existing TAG/SPR parsers, sprite preparation and OpenGL masked
copy are reused suppliers. No original TAG lookup, animation timing or new
reverse-engineered function is claimed. TAG remains optional metadata in the
wider engine; this explicit paired-input consumer requires it by native policy.

| `NP.sprite-session.` component | Bounded implementation |
| --- | --- |
| companion-resource | Owned SPR/TAG acquisition and exact ordinal/name/occurrence agreement; immutable source after files close |
| exact-name-index | All eight bytes, including bytes after NUL; occurrence returns the matching SPR ordinal |
| clip-range | Nonempty bounded occurrence ranges; interleaved physical frames preserve occurrence order |
| loop-playback | Integer host ticks, modulo whole-clip duration, arbitrary uint64 elapsed values |
| one-shot-playback | Saturating phase, explicit finished state, final frame held |
| checkpoints | Owned source identity plus validated clip, rate, mode and phase; atomic restore |
| atomic-reload | Validate the existing clip against the replacement before publication; reset phase; preserve old plans |
| draw-plan | Immutable source lifetime and value-owned palette/placement capture |
| cpu-pixels | Complete batch admission followed by atomic clipped RGB565 pixel publication |
| gpu-pixels | Complete logical admission and all uploads before drawing through native masked sprite copies |

Resource limits are1..4096 frames, at most2048x2048 per frame and16Mi total
source pixels. The existing file loaders retain their own encoded/decoded bounds.
Construction also validates every frame through the existing renderer supplier.
Names use exact bytes without text decoding, case folding, path inference or
normalization. A TAG occurrence must equal earlier occurrences of that exact
name. Unknown names and out-of-range occurrences refuse. This strict companion
policy is independent of the permissive raw TAG parser and confirmed corpus
correlation; it is not a recovered original malformed-input contract.

Playback frame duration is1..1,000,000,000 host ticks. A clip uses occurrence
indices, not a contiguous SPR interval. Loop phases are strictly below duration;
one-shot phases may equal duration. Single-frame clips and empty visual frames
are allowed. Clock units, cadence, pause and scheduling belong to the caller.
These policies do not interpret ANI or claim original actor animation timing.
A restored checkpoint can deliberately restore an older immutable table. Failed
restores/reloads preserve the complete prior state.

A batch accepts at most4096 placements and1Mi aggregate processed pixels,
including repeats and offscreen frames. Logical validation precedes prepared
plane allocation. Sources held by caller-created shared plans must remain
immutable. Palette overrides contain full RGB565 values and only apply to
indexed input. Signed origins and anchors clip in64-bit coordinates; opaque
zero and separate coverage remain distinguishable. CPU destinations require
complete RGB565 words and dimensions1..2048. GPU destinations must be native
RGB565 surfaces. Every GPU batch owns temporary color/mask uploads and retires
them before returning. This is bounded synchronous delivery, with no resident
cache or upload-performance claim. Existing resource caches remain separate.

CPU refusal is atomic. GPU logical/prepare/upload refusal precedes drawing;
physical GL failures during the drawing phase may leave an accepted prefix.
Renderer lifetime/thread and global budget contracts apply. No original allocation,
OS/driver failure, tinted blend, world construction, full scene, physical hardware,
performance or live replacement credit is granted. The parent TAG scope stays
partial. No original artifacts are read by this batch.

The prospective runner pins contracts and the complete compiler dependency
closure before Release and ASan/UBSan execution. Independent per-tick playback
and per-pixel fixture oracles cover indexed/direct sources, palette snapshots,
exact raw names, signed extreme placement, empty frames, rollback and lifetime.
Existing TAG and sprite loader controls are supplier regressions. CTest verifies
the application wiring through a private software OpenGL display. Required logs,
binaries, SQLite payloads and verified backups are durable; build/staging output
is temporary. A RAM quota refusal is retained separately as setup evidence.
