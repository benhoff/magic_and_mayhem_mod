# Driver worker producer-prefix observations V1

This optional observer sidecar is a native wire contract, not an original game
format. `MNM_DRAW_WINE_WORKER_PRODUCER=1` requires original-active producer
points and the selected Wine worker observer. Defaults retain the existing
worker journal and no sidecar. Source metadata comes from the existing read-only
`SceneDrawingSourcePointV1` with a null COM input; no addresses or pixels enter
this journal.

The 64-byte header contains `MNMWPP01`, version 1, row size 128, build marker
40209ca7, row limit 131072, source point size 64 and zero reserved words.
Each little-endian row has 32 DWORDs.

| Words | Meaning |
| --- | --- |
| 0–4 | Ordinal, phase 1 entry/2 return/3 terminal, worker ID, thread, tick |
| 5–7 | Getter status 0 success/1 unavailable/2 refused, legacy worker entry record, reserved zero |
| 8–23 | Pointer-free SourcePointV1, or all zero when getter unavailable/refused |
| 24–31 | Reserved zero |

The terminal's worker field counts entries; word 6 counts returns and word 7
counts pending callbacks. Getter/point fields are zero. Pending or missing pairs
remain incomplete. Every pair must match its legacy worker identity and thread.
A null-COM point has no canvas/extent/dirty/sampled association; its owner must
be the queried worker thread. Prefixes must not regress within one callback.
Rows from different threads can arrive in a different order after their
outside-lock query; neither global prefix monotonicity nor entry arrival order
is imposed.

The getter runs outside the private journal lock, as does the original worker
body. Existing worker pending accounting prevents ordinary stop during the
observed callback. Retire the sidecar flag under the lock before closing; recheck
before emission. Forced mid-call teardown is incomplete, not an invented return.
The surrounding worker wrapper preserves original callback state. Source-prefix
bounds are observations, not atomic pixels, exclusive ownership, a total replay
order or full hook/module quiescence.
