# DRAW-02 journey suite (schema 1)

This is a native test policy, not a recovered game format. The suite pins the
No-CD executable hash and immutable component catalog hashes. Its fourteen
journeys and twenty-one operation requirements must exactly match the reviewed
`tools/drawing_journeys.py` denominator. A catalog cannot remove an inconvenient
journey or failure case. Catalog identities must be unique and repository-local;
paths cannot enter original/ or .git/ or escape through symlinks.

Adapters distinguish operation/World component comparisons, complete composed
canvases, retained producer sequences and independently captured presented-frame
sessions. A composed canvas does not establish physical presentation. Successful
sampled World output does not establish temporal continuity. Existing component
and startup-scene catalogs contribute bounded comparisons only.

Coverage has two independent axes: acquisition of independent original references
and successful native parity. Native mismatch, unsupported rendering or execution
failure retains admitted reference coverage but never adds parity. Journey gates
require positive frame counts, contiguous presented frames, explicit action/frame
bindings and independent original provenance. Operation fixtures may separately
cover narrow API contracts, including destination-before inputs; they cannot
establish whole-scene source ownership. The checker derives coverage from admitted
observations; neither catalog pass bits nor missing cases count as execution.

The most ambitious completion policy requires all journey gates and operation
contracts, with no mismatching, unsupported, errored or pending admitted cases.
The denominator includes transitions, failure cleanup, original return values,
palette behavior, borrowed reads and recovery alongside pixels. Future acquisition
must bind those claims to actual original execution and independently retained
outputs. Synthetic acceptance tests exercise accounting only and never contribute
real original-reference coverage.

A `producer_sequence` catalog binds its exact historical report, full source
stream hash, ordered chunks, producer counts, final queue/canvas boundary, encoded
assets and every checkpoint ordinal. Each original plane carries compressed and
raw hashes and historical path. `xor-gzip` additionally carries the decompressed
`delta_sha256` and `base_checkpoint` ordinal. Its base must be the immediately
previous checkpoint with the same canvas generation and extent. Decode is bounded
per artifact and by a128MiB producer stream; full-plane original hashes are checked
after XOR. Delta decoding is confined to the comparator. The native process
receives the unchanged producer bytes, encoded sources and requested output
identities, including no expected-plane bytes or hashes.


The current native sequence transport removes observed raster EAX (kind9,
field21) before replay; portable captured artifacts remain byte-for-byte immutable.
This diagnostic-field normalization preserves all owned raster/frame/colour/row
inputs. The full sequence includes a recorded native bypass, so its checkpoint
pixel parity cannot be relabeled as full original return-state parity. The
separate DRAW-03 prefix before suppression compares low16 results independently.

## Presented session catalogs and backend ABI

A session catalog pins `schema:1`, build/source hash, registered original evidence
ID/report hash, unique session identity, scenario, actual observed requirement
IDs, operation IDs, scope and remaining limitations. `presentation` declares the
observed first/last indices, boundary, zero dropped frames and sampling interval1.
`actions` have contiguous indices, semantic action strings and nondecreasing
`before_presentation` boundaries. `frames` bind session/presentation index, extent,
pixel format (`rgb565` little-endian or byte-order `rgba8`), completed action count,
owned input IDs and independent full-plane artifacts. `effects` bind ordered
identity/operation/presentation index to the complete expected result object.
Every claimed operation needs a result, including failure returns where observed.

The original collector's immutable `drawing_session` record must match these
identities and scope. Its input map must independently pin every source and
original plane raw hash. Acquisition contracts normalize artifact paths/codecs to
historical path/raw hash identities, so re-retention cannot alter provenance or
promote a composed checkpoint into a presented frame. Per-artifact decode bounds,
frame/action/input budgets, path containment, duplicate identities, nonintegral
identities, empty input bindings and oracle-as-input hashes are guarded.

The backend runs as `EXECUTABLE OWNED_INPUT_DIRECTORY NEW_OUTPUT_DIRECTORY`.
`session.json` contains only schema/session identity, owned bindings (ID/path/role/
SHA-256), actions, frame identities/input IDs and effect identities. Its roles
are owned-state, encoded-asset and operation-input. The backend must understand
and validate its admitted owned-state protocol; the generic suite does not infer
raw engine pointers or invent a simulation-state decoder. No expected effect
results, oracle paths, pixels or output hashes are sent to rendering.

Output `session-report.json` binds success/session identity, zero remaining
surfaces, false oracle-use flags, ordered frame identities/SHA-256 and complete
ordered effect results. Frame files have fixed names `frame-000000.bin` onward.
The comparator checks every plane and effect; missing/reordered/partial output is
an error. Backend exit64is a distinct unsupported result. Native errors retain
independent reference acquisition coverage but cannot add parity.

Action and frame identity objects have strict field allowlists. Extra fields,
including oracle paths or expected hashes disguised as identity metadata, refuse
admission. The backend declaration cannot inherit arbitrary capture fields.

Native frame identities and effect results use canonical typed JSON comparison;
booleans cannot impersonate integral frame identities or HRESULT values.
