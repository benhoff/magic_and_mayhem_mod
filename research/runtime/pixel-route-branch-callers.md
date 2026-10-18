# Five producer caller investigation

As of 2026-10-18, **we have never found or observed a confirmed natural
original-game caller for any of these five functions in the investigations
recorded here.**

| Native route | Original entry | Caller finding |
| --- | --- | --- |
| `pixel/pcx_words` | `0x54b5f0` | No confirmed caller found or observed |
| `pixel/brighten` | `0x58d770` | No confirmed caller found or observed |
| `pixel/row_copy` | `0x58d3b0` | No confirmed caller found or observed |
| `raster/splice15` | `0x57e1b0` | No confirmed caller found or observed |
| `minimap/camera3` | `0x5532f0` | No confirmed caller found or observed |

This records the outcome of the bounded static searches and retained live
campaigns. It does not prove that no caller exists. The classification remains
provisional `unused_candidate`; computed or external callers remain unresolved.
These are caller-discovery gaps, with no confirmed broken call site to repair.

The five helpers already have native implementations and isolated original
comparisons. Their unresolved milestone is a natural original-game caller.
This investigation adds direct branch encodings omitted by the previous
E8/E9-only byte search; it does not force dispatch or grant live route credit.

`tools/inspect-pixel-route-branches.py` scans every byte of raw-backed executable
sections for near CALL/JMP, short JMP/Jcc/LOOP/JECXZ, near conditional branches
and immediate far CALL/JMP offsets. Far segment selectors remain unknown.
Operand-size-overridden near rel16 clears upper EIP in a 32-bit segment and
cannot reach the selected addresses above 64 KiB. Overlapping byte patterns
remain candidates until their surrounding ordinary instructions are reviewed.
Computed pointers, indirect branches, runtime code changes, foreign modules,
other builds and deliberately overlapping execution remain unresolved.

The old camera3 search ended at `0x553640`, in the middle of an instruction.
The existing registered recovered range already ends at `0x5536bc`, after RET
at `0x5536bb`. The shared scanner window is corrected to use that complete range without rewriting earlier
evidence or claiming a new discovered inventory function. The other four
previously reviewed windows are retained.

The apparent branch at `0x54b5d5` starts inside ADD EBP,EBX; `0x54b854`
starts in MOV [ESP+70],EAX; `0x58d908` starts in AND ECX,-32. The apparent
edge from `0x553651` is inside SHL EAX,1 and is internal to camera3's corrected
range. These byte patterns do not demonstrate original callers.

The previously unassigned rdata candidate at `0x5c6022` is inside the aligned
eight-byte double at `0x5c6020`, bytes `88358dd41800f03f`, value `1.00002368`.
Original FADD QWORD instructions at `0x479f2e` and `0x47a026` consume that
constant during a repeated multiply/add arithmetic sequence. Its interior
DWORD happens to lie in the row-copy RVA interval; this interpretation is not
a callback reference. This establishes the observed arithmetic use, not a
proof against arbitrary aliasing by dynamically computed code.

Confidence: confirmed bytes, selected instruction alignment, constant value
and arithmetic consumers in pinned No-CD build `40209ca7`; provisional
`unused_candidate` classification for the helpers. The denominator remains
43 and the previously observed union remains 38/43. No original dispatch,
game inputs, counters or runtime routing are changed by this analysis.

Reproduce using a prospective `NP.pixel-route-direct-branches` declaration,
then invoke the scanner separately with `--mode controls` and `--mode image`,
new `--report` paths and the same `--claims` file. Include
`NP.pixel-route-entry-boundaries` for its renewed synthetic controls. The image mode verifies the
original manifest before and after its read and refuses an unsupported image.

The [final-source controls](pixel-route-branch-controls-20261018.json) pass
23 methods (10 branch tests and 13 PE/range tests). The
[original image scan](pixel-route-branch-image-20261018.json) retains three
external operand-overlap candidates, no new confirmed caller and 24 stored
RVA/file-offset interpretations at the same twelve positions as before.
Rechecking preferred VAs and PE entry/export/TLS roots against the corrected
camera tail adds no candidates. The source declaration and report are immutable;
earlier exploratory scans remain in `/var/tmp/mnm-five-callers-20261018`.

The shared range correction changes provenance bytes used by the broad
`NP.pixel-route-27-followup` and `NP.finite-cpu-displacement` historical execution
records. Those results retain their historical milestones; their current-source
validation is pending. No game was launched and no renderer equivalence was
renewed by these scanner controls. The PE entry-boundary implementation has
fresh synthetic controls, separately from its historical original-image result.
The broad game record also supports CPU-selection and RGB-caller observation:
their live integration validation is now pending through that same provenance
coupling, while their independent fresh implementation controls remain current.
The separate RGB original comparison and strict-boundary evidence stay current.
