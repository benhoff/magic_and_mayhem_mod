# Global size-one lighting storage and installed source selection

This investigation concerns the pinned No-CD build
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
It complements the [captured-cycle comparison](terrain-single-cell-lighting-20261026.md).
It introduces no native production change or gameplay configuration change.

## Storage origin and selected writers

The installed map receiver is the global object at `006c5490`. Its inherited
single-cell byte is at hexadecimal offset `6380`, absolute `006cb810`.
Both addresses lie in the zero-filled virtual tail of the PE `.data` section,
outside its stored bytes. This particular object's initial byte comes from
image storage rather than a heap allocation. An arbitrary caller-owned map
object still inherits its own supplied byte, as the earlier 256-value tests show.

The original kernel builder `004f1890` puts the eight size-two kernel bytes at
map `+6378..+637f`, and stores the index-one pointer at `+6381..+6384`, pointing
back to `+6380`. The byte between these regions is not written by rebuilding
the kernels. The apparent index-zero pointer slot at `+637d` is only arithmetic
used by the stamp lookup; treating it as a real independent pointer would
misclassify the packed layout.

The [write-watch report](size-one-provenance-results-20261026/watch01.json)
executes unchanged initialization `004ecdd0`, direct kernel rebuild `004f1890`
and clear `004ecbe0` against the global receiver. Each probe starts with the
loader-zero state or one of all 256 explicitly supplied byte values. It runs
initialization, rebuild, allocated clear, reconfiguration, reinitialization and
a second allocated clear. A CPU write watchpoint detects writes overlapping
the exact byte, including stores that preserve its value. Explicit byte and
unaligned overlapping DWORD stores provide positive controls in every case.

The opaque delete endpoint at `00597870` is an expected-byte-checked private
fixture adapter. It observes the ten buffer retirements per probe while the
host vectors retain ownership. No selected original function body is patched;
the executable text is RX during execution. The fixture does not establish
original CRT heap behavior, Windows loader/API equivalence or arbitrary aliases.
Both optimized32 and host UBSan32 variants exercise the same original instructions.
The watch geometry is19 by21 with five layers and1197-byte light buffers;
ambient/ramp retain image defaults -108/-16. The separate capture comparison
covers its four declared paired dimension/configuration cases.

## Installed inputs and actual observations

The same report retains the packed installed `effects.cfg` bytes and decodes
the three existing FX lighting fields. The original `0049c2e0` loader consumes
those bounded strings through declared profile endpoints, and `0049cf60`
selects all 89 resulting entries from the actual global collection at `00689498`.
The nonzero installed diameters are FXA_3=4, FXA_13=6, FXA_21=10, FXA_22=10,
FXA_24=5 and FXA_36=4. None selects index one. A separately labeled authored
FXA_0="1" control demonstrates that the original loader/selector permits one;
it is not a natural installed source and does not change any configuration file.

The collector also analyzes two retained successful original-observation cohorts:
`fix-observe-reload01` and `observe-stress01`. Their 64 exact V1 packets retain
the original collector and source hashes. Header DWORD `+56` sign-extends the
inherited map byte; every observed value is zero. Active static records in the
stress cohort include indices zero and six. These are observations from earlier
executions, freshly checked for payload identity, extent and checksum. They
are not a new live session or renewed validation of the old capture source set.
The ordinary creature updater's fixed index 33 remains separately recovered.

## Scope and remaining work

`RE.terrain-single-cell-global-storage` owns this scoped storage-origin,
selected-writer and installed-table finding. Its implementation, native
comparison, integration and replacement milestones remain none. The prior
kernel/capture implementations and independent evidence remain unchanged.
`TL.special-size-one` retains the remaining whole-program alias/write census,
other source-producing paths and any natural index-one reachability beyond
the two observed cohorts and pinned installed FX configuration. Absence of
one from this table is not proof that scripts, altered configuration or every
unclassified caller can never supply it. The 256 inherited values remain part
of the recovered component contract; the native zero default is not generalized
to arbitrary original allocations.

All new observations, exact source variants, compiler/runtime logs and binaries
use SQLite evidence references. Original manifests are verified before and after
the original-consuming runs; earlier reports and fingerprints remain immutable.

The successful report records257 probes per build: one untouched loader-zero case and all256 explicit inherited bytes,514 initialization calls,257 direct rebuilds,514 allocated clears,2570 supplied retirements and257 authored index-one selector controls. Both builds detect514 positive-control writes and zero selected-path writes. The64 retained packets and three malformed-packet refusals are separately recorded.

[Configuration lineage](size-one-provenance-results-20261026/config-lineage01.json) verifies that the retained clean and NoCD packed copies differ only in their encryption seed: both decode to the same8364-byte plaintext, and the267 selected fields match the exact fixture consumed by the original probe. The clean packed hash also matches the prior immutable installed-effects report. The two legacy focused packages receive only the same gap clarification and document links; their historical evidence, source fingerprints, provisional aggregate confidence and execution statuses are preserved.
