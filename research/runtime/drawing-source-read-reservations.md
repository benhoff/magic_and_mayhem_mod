# Observed-path COPY source read reservations

The opt-in guard now excludes observed mutations before they forward, from the
extra synchronous READ map through the exact emitted command's worker and a
signaled fence in that worker's current GL context. The older source-version
guard only detected an epoch change afterwards. Neither policy establishes
ownership of physical backing across unobserved aliases.

Add `--wine-source-reservations` to a bounded source-version capture with
`--wine-gpu-completion`. The runner declares the policy in the staging manifest
and sets `MNM_DRAW_WINE_SOURCE_RESERVATIONS=1`; it clears that ambient variable
when the staged policy is absent. Invalid selectors or missing prerequisites
refuse initialization. Default observation remains unchanged.

The reservation pins public resource identity, texture generation, subresource
and write epoch, then binds the payload ordinal, emitted packet and worker.
Writable public maps, public copy destinations, opcode42 worker destinations
and descriptor/client-memory rebinds all refuse before forwarding if any reader
holds that resource. Multiple readers retain independent reservations. Texture
decrefs conservatively refuse even when they might only reduce a nonterminal
reference count. A separate retirement lease is published before the extra
resource getter and retained through the original decref and generation-journal
return. New source acquisitions refuse unresolved or aliasing retirements,
closing the reader-start race during the original release body. Unknown early
installation getters remain wildcard leases; no null getter is called. Close
refuses either outstanding source or retirement leases. Same-resource self-copies are unsupported. Reservations cover
all subresources conservatively. No worker or GPU waits occur under the journal lock.

The READ map pointer is discarded on unmap. The diagnostic owns its copied
oracle bytes; those bytes are never inputs to native rendering. It does not
retain, protect or read an invalid mapped pointer. Bounded journal format and
refusal reasons are documented in
[drawing-source-reservations-v1](../formats/drawing-source-reservations-v1.md).

## Current execution

The fresh original-active first120 capture completes normal original Quit and
verified child exit0 with all2927 immutable inputs verified before/after. Its
2,734 reservations all reach their exact selected worker and signaled
current-context fence; no reservation remains at close. Source hashes remain
stable. The launcher's exit0 is not independently witnessed. The original run
uses llvmpipe and diagnostic readbacks, so it is not a production cadence or
hardware execution claim.

The controlled PE32 final suite passes19 cases through real observer wrappers with
owned original-body stand-ins and fake GL responses. It covers writable-map
conflicts before/during consumption, rebind, worker self-alias, public-copy
mutation entry, conservative decref, early close, outstanding shared readers,
post-release writes, unrelated writes, no-current-context refusal, writes during
the fence and default-off forwarding. Two added controls exercise acquisition during an
actual decref body and acquisition after its successful return. Six decoder methods include changed
identities, omitted/reordered fences, consumer mismatches and incomplete logs.
Public-copy mutation and decref guard entries are called directly in their
controlled refusal cases; map/rebind/worker cases traverse the real wrappers.
The existing source-version13-case and GPU14-case PE32 regressions pass too.
The standalone GPU fixture verifies the new hand-off arguments; enforcement is
exercised by the separate reservation fixture.

A retained direct-write control deliberately bypasses the observed writer
entry points, changes the consumed byte and is still admitted. This confirms
the remaining ownership limitation on current code; it is not a passing
atomicity test.

The explicit cursor provenance join accounts for270 wrapper calls and450 Wine
consumer chains, including all34 wrappers pending at source boundaries
(20keyed sheet copies,13background restores,1background save). It joins exact
primitive/API/submission identities to resource generation, epoch, mapped
payload, packet, worker and reservation, with each resource's observed writer
history. Some wrappers emit an additional primary-surface consumer: the
wrapper's logical source is not necessarily every internal command's source.

| Logical cursor operation | Observed consumer resource/generation | Chains |
|---|---|---:|
| Save primary into background | 1/13 | 90 |
| Sheet into primary | 6/19 and 1/13 | 90 each |
| Restore background into primary | 5/18 and 1/13 | 90 each |

These IDs describe this launch only. Public object and observed texture aliases
are recorded; physical allocation identity remains explicitly unknown.

Independent analysis matches394 selected source/worker/EGL regions at RGB
tolerance1. First120 accounting retains120 source queues,1,968 input calls and
300 matching primary witnesses. The complete interval gate still rejects the
same four ownership/route conditions. It is not relaxed by these reservations.

Execution records, pixel-suite results and retained setup failures are indexed
by `source-reservations-provenance-20261017.json`. Original comparison and live
replacement statuses stay separate from this diagnostic implementation.

## Remaining ownership boundary

The four observed mutation entry points do not cover direct CPU writes, other
Wine command opcodes or separate resource objects sharing storage. A public
resource generation is not a physical allocation generation. A signaled fence
covers the current GL stream, not all contexts, display delivery or scanout.
Broader hook/module/timer quiescence remains separate from the bounded session.

Physical backing and alias writer closure, quiescent source boundaries, live
source points for every primary witness and global presentation routes remain
open. Generic47journey/21operation requirements and complete native replacement
remain open. Enabling this policy must not promote those gates.
