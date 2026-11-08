# Native image-copy caller state — 2026-11-08

CS09 investigates the still-open `operation.flags-and-return-values` requirement.
Its complete scope spans C01 surfaces, C03 World producers, C07 sprites and C09
screen presentation in both supported profiles. This increment fixes one actual
production entry; it does not close either acceptance row or either final decision.

## Confirmed discrepancy and correction

The unchanged NoCD `533a00..533b1a` bytes and production PE32 DLL produce identical
complete destinations in all 2,112 baseline cases. Nevertheless, image-abi-04
finds zero complete caller-state matches: the C replacement leaves EDX holding
the saved LastError, emits different arithmetic flags and retains the incoming
X stack argument where original `533b0b` writes the final row countdown. EAX,
ECX, callee registers, ESP, source bytes and tested floating state match. These
are confirmed routine-boundary differences; whether a particular caller consumes
each field remains a separate inventory/liveness question.

The full entry now saves integer/FX state, calls native clipping and copying,
then restores the exact outgoing state. A nonempty original copy returns the
next source-row pointer in EAX, zero ECX, the next destination-row pointer in EDX
and zero in its retired X argument. An empty intersection returns zero in all
three volatile registers and retains the argument. Other integer registers,
stack balance and the original rectangle provider's outgoing floating state and
LastError are preserved. Deterministic nonzero XMM inputs reveal a second real
boundary in image-abi-08: Wine IntersectRect clears XMM0 on empty intersections.
The native entry now calls the same platform service with the incoming FX image
and saves its outgoing image before native ownership/history work. It does not
replace the provider's result with the entry state.
The original final flag writer is `ADD ESP,38h`; the native helper derives those
flags from the original entry ESP, rather than its compiler-generated frame.
The existing forward-copy direction policy is explicitly checked at full entry.

No original image instructions run through the native entry: fixture bytes after
its first eight bytes are poisoned. The separate original reference stays byte
identical before and after execution and uses real Wine `IntersectRect`.
Destination identities are independently allocated and normalized only within
their complete buffers. No original output becomes native input.

## Validation and limits

The bounded final corpus contains 19,008 calls: two format branches, 88 anchor
pairs, nine clipping/empty-shape/pitch variants and twelve masked precision /
rounding modes. It varies four caller-stack alignments, supplies three active
x87 values and deterministic XMM/MXCSR inputs, and compares full 4,096-byte
destinations including guards, source preservation, GPRs, defined integer flags,
all six retired argument slots, LastError and FX bytes excluding instruction/data
history. These are finite normal-return controls, not malformed-pointer,
unmasked-exception, arbitrary-overflow, multithreaded or installed-game proof.

Actual PE32 extended drawing and protected owned-backing controls run alongside
the new comparison. Existing overlap/history/continuation checks and expected
bad-span / bad-prefix refusals remain, with a full-entry direction refusal added.
Reader controls reject partial/missing/repeated domains, shared destinations,
wrong declared pitch, unknown result bits and pixel-only false equivalence.

All reports, raw state records, tested DLL/EXEs, logs and changed source bytes
use SQLite under `caller-state-results-20261108/`. Early input/link/Wine setup
refusals remain. Runner03's cleanup exception prevented report/raw temporary
retention; its exact source and traceback are retained explicitly without a
completed execution or equivalence claim. Later runs persist diagnostics,
source preimages and tested executables before execution.

## Remaining acceptance boundary

The binary inventory names one direct call at `58f9e3`, owned by `58f9c0`, into
the repaired routine. This does not classify computed callers or establish live
reachability. Required surface/provider, World producer, sprite and screen
bypass/caller state must still be joined to independently checked pixels from
the supported default/software sessions. Existing Word/Glyph comparisons retain
their own floating, reentry, provider and live-source limits. The 112 passed /
44 pending parent inventory remains unchanged by this component correction.
