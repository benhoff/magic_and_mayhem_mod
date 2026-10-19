# Bounded owner multi-source decision transcript v1

`MNMMS001` is an original-first test transcript, not a save format or a portable
World capture. `tests/owner-multisource-pipeline-test.cpp` defines the authored
fixture deck and the exact field order. The report binds that source and every
compiler dependency to SHA-256. Confidence is high within the compared domain;
no installed-model or original-perception producer claim follows from this wire.

The first eight bytes are ASCII `MNMMS001`. All remaining fields are unsigned
little-endian 32-bit words. Signed integers, sentinel `-1` and binary32 scores
retain their bit patterns. The header contains fixture count 32 and slice count 128.
Each fixture begins with its zero-based index. Each slice contains ordinal,
captured clock, total count allocation, disabled 0/1, baseline regional hazard 0/1,
hazard pulse 0/50, then a word count and that many state words.

The state vector contains, in order:

1. Brain clock, scheduler phase, current committed source ID, length-prefixed
   pending source-manager IDs, selection reset, shared score, pending source
   decision stage and winner. Stage/winner are `UINT32_MAX` when reset is set;
   completed stale phase fields are outside the comparison.
2. Strategic RNG left/right cursors and all 250 state words.
3. Length-prefixed installed task-map keys, pending execution-map keys and source
   manager IDs; committed graph node count, then each node's parent ID and
   length-prefixed ordered children. IDs replace process-local pointers. Committed
   parent links come from child lists; raw original parents must agree except for
   the two known scratch proposal roots at brain+`11ea6`/`11f33` during search.
4. Next zero-based task key, installed task count, then each task in ascending
   key order: key, source ID, started/expires, value/baseline, successes/attempts,
   result14/invalidation15, target x/region/risk, feature count 72 and 72 saved
   features. Original installed-map keys must equal the source-bound task keys;
   the pending map's traversal count must equal its stored count.
5. Five length-prefixed vectors: executed task keys, retired task keys, semantic
   command records, candidate-next call records, and execution visit records.
   Commands contain actor ID and kind (1 target, 2 move, 3 stop), followed by target
   actor ID or x/y/z for target/move respectively. A candidate call contains
   member count, ordered actor IDs, budget before/after and returned 0/1. Successful
   calls append score bits, generated duration, risk and all 72 generated features,
   including losing candidates. Execution visits contain task key, budget before,
   budget after and continuing 0/1.

The reader limits files to 32 MiB, checks exact fixture/ordinal/vector extents,
strict flag and stimulus domains, sufficient bytes and exact EOF. The writer
refuses an existing capture and enforces the same output-size bound. Native and
sanitizer replay read the original clock jump and allocation stimuli; they do not
derive them from their own selected task state. Damaged magic, truncation,
trailing data and changed expected state are separate refusal controls.
