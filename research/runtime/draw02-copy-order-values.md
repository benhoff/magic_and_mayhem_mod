# Native copy-order value diagnostics

The approved held120 cohort matches all 2200 CPU checkpoints and all 126 admitted
physical witnesses while retaining 33 adjacent full-copy/background-save
inversions eligible for this probe. The older held120 cohort still fails 1600
pixels after a uniquely diagnosed local reversal. Ordering disagreement alone
cannot select a repair.

The probe publishes owned native buffers before an adjacent save, the current
full-copy source after that save, and the saved 40×40 buffer after the full-copy
return. Every producer parameter byte, operation order and checkpoint hash must
remain unchanged. Only pinned encoded assets and parameters enter native inputs;
original output pixels remain external. Active World updates, unsupported
geometry, overlapping diagnostic boundaries and changed inputs refuse.
Diagnostic success is completed read-only publication, not original/native parity.
Exact request/API/worker association and historical local diagnosis without
request IDs remain separate.

The probe also records the original caller address, next same-source checkpoint,
and first subsequent COPY into the saved buffer or from it into the full source.
A tracked canvas create/release stops this search. This deliberately narrow
predicate does not classify other destinations, non-COPY writes or backing
aliases; it cannot prove absence of propagation or general source ownership.

The first diagnostic reports (`draw02-copy-order-values-*-01.json`) used a
prospective contract with one duplicated dependency. They remain historical
results with that contract mismatch after dependency deduplication. New `*-02`
results use a fresh declaration and the current tool; neither result overwrites
older execution, claims or checkpoint evidence. All stage statuses stay partial
or none. The historical failed comparison remains failed.

Current-source `held120-02` compares 33 pairs, of which ten differ between the
prior native primary region and current native CPU source. Every saved buffer
equals the prior primary region; all 2200 checkpoint hashes are unchanged. These
ten full copies return to `0x46ba01`. Their next tracked COPY is into the saved
buffer rather than directly back into the full source. Reads into other
destinations remain outside that predicate.

`legacy120-02` compares the one historical failed pair: all 1600 native region
pixels differ. Full-copy caller `0x5597fd` is followed by saved-buffer→canvas2
COPY at sequence208519 and canvas2 checkpoint208520. All 2182 checkpoint hashes
remain unchanged and the original/native comparison still fails. The evidence
supports a distinct local propagation path, not a proof of original backing
identity or a safe general ordering repair. Confidence is provisional.
