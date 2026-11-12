# Passive palette custody v2: observed floating-point context

Version2 uses magic `MNMPAL02` and header version2. All sizes, identity, owner,
budget, payload and selection/retirement rules from v1 remain unchanged. Builder
and tracked-free entry/return records additionally use header words16–19 for
x87 control word, MXCSR, x87 status word and abridged x87 tag byte. The entry and
return assembly passes its already-owned aligned FXSAVE area to the observer;
these are snapshots before observer code executes. The observer never changes
that saved context and FXRSTOR restores the original state before continuation.

All other reserved fields remain zero. The reader keeps v1 separate: old captures
have no observed floating context and must not silently acquire a default one.
The numerical replay may use an explicitly authored v1 diagnostic context, which
does not prove actual runtime state or satisfy cycling acceptance. V2 numerical
comparison instead uses the recorded entry CW/MXCSR and restores the host's prior
context after its bounded computation. Captured status/tag are retained; this
does not recover exceptional/trapping/nonfinite arithmetic or a full caller ABI.

V2's bounded observation cache retains each first frame/allocation/table value,
each changed value and every frame's selected allocation/ordinal transition,
including A→B→A. Repeated unchanged spatial shade choices are checked without
expanding the log. There are65,536 frame slots and262,144 private value slots;
exhaustion records permanent loss16. This is a changed-state source diagnostic,
not a journal of every draw or a claim of per-actor allocation identity. The
complete producer stream remains authoritative for actual drawing and ordering.

The first v1 runtime probe exposed this missing input: unchanged original replay
matches all71 non-profile-dependent nodes/952,576 WORDs under authored CW027f;
the other eleven tested precision/rounding combinations differ. This is an
inference about the missing context, not a v1 observation. Direct v2 collection
must establish the actual context before any output claim. The initial native
comparison and both diagnostic source phases remain immutable.
