# Direct-word clipping shadow diagnostics, version 1

The existing `MNMWRC01` sample and `MNMWRD01` statistics layouts remain unchanged.
Mode 3 is an explicit clipping-shadow extension; old readers may reject it. The
`word-NNNN.bin` sequence in this mode is a capture ordinal 1..8 for clipped positive
requests. Workspace/pixels after come from original execution. There is no
body bypass. All other fields follow [word-sprite-samples-v1.md](word-sprite-samples-v1.md);
reserved fields remain zero and empty requests are not sampled.

`clip-stats.bin` appends little-endian 80-byte snapshots: eight-byte `MNMWCL01`,
DWORD version 1, record bytes 80, and sixteen counters/fields:

| DWORD | Meaning |
| --- | --- |
| 4 / 5 | Installed / seen hook calls. |
| 6 / 7 / 8 | Compared positive requests / original-clipped-branch requests / strict interior requests. |
| 9 | Compared frames whose footprint has no visible intersection. |
| 10 / 11 | Original executions, including tail forwards / fallback requests. |
| 12 / 13 / 14 / 15 | Mismatches / capture-log errors / retained samples / stopped. |
| 16 / 17 / 18 / 19 | Empty-dimension forwards / retained clipped samples / original calls inside comparisons / reserved zero. |

A clipped candidate has left/top before clip-left/top or an exact/overrun
right/bottom edge (`>=`). Hidden here describes footprint intersection, not an
all-transparent sprite. Counters are snapshots/lower bounds after forced shutdown;
compared/clipped counts must not be interpreted as performance measurements.
Snapshots report admission/call branches even when capture is bounded.

Private host comparison results use `MNMWSH01`, DWORD version 1, record bytes 288.
First 64 DWORDs follow [word-cpu-clipping-results-v1.md](word-cpu-clipping-results-v1.md)
with field 52=actual shadow comparison count, 53=actual fallback count, 60=actual
production shadow caller-state check mask (`1023` succeeds). Extra DWORDs:
64=actual original body calls (must be 1),65=LastError preserved,66..70=the five forced
refusal check masks (`4095` means ten state/guard checks plus original-once and
LastError/fallback counters),71=clipped comparison count. This is a host-only
private format, not a process wire channel. Partial/unknown records are refused.
