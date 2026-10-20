# High tranche 2 implementation review — 2026-10-19

This review concerns the seventeen High animation/sprite/native-drawing tasks,
not the eleven Critical canvas-producer tasks. The reviewed committed HEAD is
`0dc91365aed634b16c61f31cc8f34b2746eee62a`, unchanged since the High planning
review; no new committed range requires retrospective accounting. Existing
uncommitted engine, campaign and dashboard work is preserved.

The concrete missing implementation is the selected original distortion DWORD
read model. `distortion_word_read.hpp` now models that recovered load/store
contract and an explicit unreadable span safely. Original tight/padded and
one-byte-tail controls live in reproducible repository sources. The selected
model does not require inferring the uncaptured live crash operand or copying
unsafe loads into the renderer. Those broader recovery questions stay open.
See [read model and boundaries](surface-final-word-read.md).

The other fifteen narrow services already supply their declared mechanisms:
the checked native image-row ABI; canonical five-format/key/stretch copies and
history publication; projected shadow-envelope refusal; forward animation and
phase-preserving switching; sprite/attachment placement and mode-one controls;
the production world-facing callback; unstable queue sorting/depth keys and
visibility grids; decoded sprite uploads; sprite/layer, terrain/light and
bounded World scene composition; and strict portable fixture validation.
Their existing admission/refusal boundaries are intentional. Current original
comparisons and integrated live journeys are distinct validation milestones.

## Surface umbrella crosswalk

`NR.surfaces` describes owned surface presentation and command replay. Its
implementation links already lead to `GlBlitter`, framing/validation and the
incremental command consumer. The broad title does not introduce a second
surface allocator, general DirectDraw emulation or every operation/scene.
The existing services below supply each required operation family from
`native-surface-operation-matrix.json` within their registered domains.

| Required domain | Delivered supplier and source | Remaining boundary |
| --- | --- | --- |
| RGB565, other canonical formats and signed pitch | `NR.surface-backend.formats-pitch`, `SurfaceBackend::createRows/updateRows/readRows`, `renderer/pixel_rows.cpp`, `renderer/blit.cpp` | Arbitrary masks and original gameplay format reachability remain unvalidated. |
| Opaque copies and source keys | `NR.surface-backend.owned`, `NR.surface-backend.formats-pitch`, `SurfaceBackend::copy`, `renderer/surface_copy.cpp` | Native transform policy and original driver equivalence are distinct. |
| Rectangle and region clipping | `NR.surface-copy.clipping`, `NR.surface-copy.clipping-stream`, `planSurfaceCopy`, version-3 command decoding/consumption | Selected RGB565 flags and partial writes only; live clipper observation and other domains stay separate. |
| Fills | `NR.surface-backend.owned`, `NR.surface-backend.formats-pitch`, `SurfaceBackend::fill` | Supplied native masks and sampled driver domains do not prove all original callers. |
| Self overlap | `NR.surface-backend.owned`, `renderer/blit.cpp`, `SurfaceBackend::copy` | Opaque per-piece snapshots and bounded forward keyed overlap are distinct; arbitrary aliases are unproved. |
| Palette lifetime and two-buffer flip | `NR.palette-resources`, `NR.surface-backend.ownership-recovery`, `SurfaceBackend::bindPaletteObject/attachBackBuffer/flip` | Owned identity and finite leases do not prove physical driver aliases or longer chains. |
| Restore and reload validity | `NR.surface-contents.validity`, `NR.surface-reload.dib-rgb`, `NR.surface-reload.dib-indexed`, `NR.surface-reload.bitmap-dc`, `NR.surface-backend.ownership-recovery` | Unknown restored bytes stay undefined; original real-loss/Windows/live composition remains unvalidated. |
| Lifetime, updates and lock/DC admission | `NR.surface-access`, `NR.surface-backend.owned`, `NR.continuous-resource-lifetime`, `NR.continuous-mutations` | Bounded owned policies, not all original leases or cross-thread semantics. |
| Ordered replay and presentation | `renderer/commands.cpp`, `renderer/command_consumer.cpp`, `GlBlitter::present/presentGpu` | Protocol versions, palette generations, defined pixels and finite limits refuse unsupported inputs. |

All twelve required matrix cells have delivered scoped suppliers. The four
conditional cells (ranged/destination keys, conversion/stretch driver parity,
general effects and complex flip chains) remain explicit discovery backlog;
they are not silently added to this umbrella. Conversion/stretch native CPU
policy is already supplied separately by `NP.native-copy-transforms`.
This crosswalk identifies no concrete missing required code in `NR.surfaces`.

`NR.surface-fixture-corpus` remains a strict validator over five retained
RGB565 opaque BltFast captures. Only two of twelve required operation cells
are in that portable corpus. Other independently retained driver fixtures are
registered under narrower suppliers; they are not retroactively relabeled as
members of this corpus. Missing corpus cells are validation-data work, not
evidence that a renderer primitive is absent. Full corpus acceptance remains
incomplete even when every admitted case compares successfully.

The new model, original comparison, native regressions and static crosswalk
have separate evidence identities. Source review does not renew old execution
fingerprints. No live replacement or aggregate renderer completion follows.
