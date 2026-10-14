# DRAW-02 experimental COPY phase policy

This is an explicit offline native policy. Default replay remains producer-return
order. The optional `--copy-phase-plan` is authored from request/API/worker
metadata before native execution; neither its selector nor transposition reads
original pixel planes or comparison outcomes.

The admitted pattern is an in-bounds40×40restore5→1, full800×600copy2→1 and
save1→5. Successful exclusive worker bindings must show restore before full
before save, while the full/save producer returns are adjacent in the reverse
order. Callers are the known cursor restore/save and ordinary/shared-menu redraw.
The full interval must stay outside World and contain no other observed drawing
or lifetime changes. Bind/clip and CPU2checkpoints before the pair are retained.
Restore and save positions may differ during cursor movement: the intervening
full worker overwrites the whole primary. Unbound requests, other geometry/keys/callers, worker overlap, another draw,
lifetime/queue boundaries and World intervals are refused explicitly.

Only the two96byte COPY bodies are transposed; their chronological sequence IDs
remain at their positions. No checkpoint is moved or removed. The auxiliary
comparison publishes the save at its new position and keeps the original request,
oracle, exclusions and denominator. Source-only encoded assets remain pinned;
all original pixels are external comparisons. The CPU comparator recomputes the
selection from pinned original metadata before admitting a plan.

The earlier auxiliary capture has three exact inverted triples whose original
mapped auxiliary images match native current source regions rather than earlier
primary regions. Two later auxiliary failures follow reuse of bad saved bytes.
Those observations motivate this hypothesis but do not select applications of the
policy. Extra original readback can perturb cache/timing. Recorder-visible
stability does not establish all-writer/alias/GPU/CPU ownership, general replay
ordering or live replacement. The historical five auxiliary and two CPU failures
remain unchanged. Guard and comparison results will be registered separately.

The compact hash comparator executes the same metadata policy against a completed
default replay template and rehashes every external original plane. The native
renderer computes every full-plane checkpoint SHA256 while `retain_checkpoints=[]`
avoids duplicate plane dumps. Every ordinal/sequence/canvas/extent remains
required; mismatching hashes fail with pixel counts explicitly unknown. No raw
evidence or earlier replay outputs are deleted. Use absolute plan input paths
when invoking tools through the main-checkout storage guard.

Current results: guards03passes seven policy tests; auxiliary plan03admits29
triples/8refusals and policy checkpoint03matches2403/2403. Auxiliary native01
matches67/67with8World exclusions. Current control plan01admits25triples/6refusals;
compact control hashes01matches2406/2406, including defaultfailedcheckpoint1459.
Older control plan01admits35triples/5refusals, but native execution did not launch
because the shared1360GiBpreflight refused its measured0.5GiBreservation.
Historical failures and scope boundaries remain unchanged; no status promotion.

Fresh prior-control policy02 now matches all2400checkpoint hashes under the
approved1376GiBworktree budget, including historical CP1352and1437failures.
Across three independent captured cohorts, policy7209checkpoints and67Aux
buffers match; default and legacy reports remain unchanged. This result does
not extend scope to unbound/nonadjacent/World/all-writer/view/route cases.
