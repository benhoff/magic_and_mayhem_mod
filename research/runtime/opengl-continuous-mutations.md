# Continuous mixed resource mutations

2026-10-06. Native policy `NR.continuous-mutations` covers the third continuous
producer chunk. Existing admitted inputs remain separate from original-driver
comparison, integration with original gameplay, and live replacement.

## Inputs and new behavior

The producer already supports owned full and partial writable Unlock, argument-
derived constant fills, admitted same-format keyed/unkeyed copies, observed
simple two-buffer swaps, palette colors, and validated application-owned DIB
handoff at successful ReleaseDC. This increment validates their sustained mixed
use without diagnostic CHECK output or sample-operation termination.

For continuous mode, a successful complete writable Unlock with changed dimensions,
bit depth or RGB masks emits DELETE for the previous wire layout, then CREATE
with a fresh monotonic ID and the complete new owned pixels. The COM alias graph
and application identity remain intact. Native palette state resets on the fresh
ID. The new resource uses the simultaneous pixel/storage budgets and normal
channel admission; any failure still refuses the stream. The ordinary bounded
policy retains layout mismatch GAP. Partial writes cannot establish a changed
layout, and descriptor-only changes, Restore and missed operations retain refusal.
This is committed replacement input rather than recovery from unknown mutations.

A first indexed palette remains a complete 256-entry RGB command. Subsequent
continuous updates use the smallest contiguous range containing changed RGB
entries. Entry flags do not become alpha; a flags-only mutation updates the
producer cache without emitting a palette command. Existing publication rules
can still present unchanged RGB pixels. Bounded palette serialization stays full.

## Unsupported branches

All preexisting ownership, thread, epoch, alias, pending-lock/DC, complete-base,
format/mask, palette completeness and simultaneous resource budgets apply.
Stretch, overlapping self-copy, unobserved clipping/key state, extra effects,
unsupported flags and complex flip chains remain unimplemented. Successful
unsupported drawing refuses instead of silently dropping a mutation. Failed
original calls preserve valid prior inputs; successful retries capture fresh
pre-call pixels or palette arguments. This adds no observer COM calls, driver
readback, GDI/text instruction replay or movie producer.

## Validation

```sh
xvfb-run -a -s '-screen 0 1280x1024x24' \
  python3 tools/test-render-mutations.py working/build/render-ring
```

The independent PE32 engine normalizes its own native backing pixels through
padded negative-pitch borrowed rows, poisons those rows at original Unlock and
palette input buffers at original SetEntries, performs its own draws/swaps, and
owns actual Wine DIBs. Its complete native frames and palettes are recorded
outside the producer and converted independently for every GPU frame hash.
Original call counts, HRESULT/LastError and archive opcode/range checks separate
failed attempts from committed inputs. No original assets are consumed.

## Recorded result

[Final-source mutation execution](opengl-continuous-mutations.json) passes twelve
cases: eight complete sessions, four intentional refusals and 933 independent
complete-frame comparisons. Each RGB mixed stream performs 40 cycles and 201
presentations; indexed8 adds palette RGB/flags-only changes for 281 frames.
Successful failed-call retries are included in original counts, while archives
contain only committed operations: each mixed stream has 159 UPDATEs, 80 COPYs
and 40 SWAPs without CHECK output. Two initial IDs persist through every mixed
operation and are deleted at shutdown.

Indexed commands comprise one complete initial palette and forty two-entry RGB
updates at entries 5..6. Flags-only updates produce presentations with unchanged
RGB but no palette command. Twelve repeated actual DIB handoffs per RGB format
include a failed ReleaseDC followed by changed input and successful retry.
Replacement transitions RGB32 to RGB565, changes only masks to RGB555, then
changes to RGB24 and indexed8: five monotonically increasing IDs, one ordered
DELETE per old layout, no redundant full UPDATE on new CREATE, and a fresh
indexed palette. Four refusal cases cover bounded redefinition, invalid partial
layout, successful unsupported drawing flags and unsupported palette flags.

Valid streams acknowledge their complete publication, end without consumer
surfaces, and use zero ordinary native/RGBA readbacks or viewport uploads.
[Fresh resource regression](opengl-continuous-mutations-resource-regression.json)
passes all nine lifetime cases and 63 complete-frame comparisons. The initial
mutation attempt passed pixel checks but expected one extra UPDATE; that fixture
count was corrected to account for initial CREATE. Focused and incomplete attempts
remain under `working/tests/render-mutations/`; the retained result uses final
sources. Production/selftest PE32 builds pass. Historical fingerprints remain
unchanged; original driver/game equivalence and recovery remain pending.

[Fresh continuity regression](opengl-continuous-mutations-continuity-regression.json)
passes all eight cases and 352 complete-frame comparisons on the same mutation
sources, including independent bounded archives and finite capacity refusals.
