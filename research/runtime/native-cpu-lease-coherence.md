# CPU reads under writable leases

The previous bounded census identifies two remaining implementation boundaries:
typed CPU readers refuse stale GPU history during an active Lock, and the
single-step store census briefly exposes public pages to another thread.

The native policies are `NP.active-lock-reader-restoration` and
`NP.protected-cpu-lease-storage`. They are separate from the historical mode2
discovery census and from recovered original-game behavior.

The ordered reader stages pending CPU writes before requesting pixels,
validate the same physical allocation, generation, nonce, Lock owner and command
session after waiting, and replace its private checkpoint before resuming CPU
work. A readback is an explicit reader operation, never an ordinary World return.
Partial Locks retain their physical allocation identity. Foreign, reentrant and
DC-held transfers are refused. An overlapping protected Lock/GetDC request returns
surface busy and leaves the Lock intact.

`RenderRequireCpuHistoryV1` holds the existing ordered-copy admission while the
FIFO consumer answers CPU_READ. Its private staged bytes are merged into that
reply using an exact per-byte store mask. A write equal to the old backing still
counts as a write; a single-byte store preserves the other byte from native GPU
history. Complete merged WORD updates are published before the restored public
mapping becomes readable. A partial-WORD Unlock uses the same explicit reader
boundary rather than uploading its stale companion byte. No readback occurs in
the exception handler or ordinary World return.

`NativeDrawCpuLeaseTransferV1` uses twelve process-local DWORDs: physical base,
width, height, pitch, generation low/high, nonce low/high, owner thread, Lock
serial, Lock count and full-Lock flag. Restoration must match all twelve and the
exact tight RGB565 byte count. This is a local DLL ABI, not a shared-channel
pointer contract; the versioned CPU_READ wire contract is unchanged.

Protected storage must keep stale public pages inaccessible while copying private
baselines and executing admitted pure stores. A private mapping of the same
storage permits this without changing public permissions. Instruction decoding
must validate the effective address, complete width, value and instruction length;
unsupported instructions remain terminal diagnostics. Publishing current history
must finish the complete copy before making the public mapping readable.

Mode3 implements this policy with two views of the same page-file mapping. Public
pages remain NOACCESS while stale, including active writable Locks and private
World snapshots. Admitted scalar MOV and STOS stores are decoded and executed
through the private view, with exact byte tracking and checked PE32 segment,
address, width, owner and instruction context. No single-step writable interval
is used. Unknown reads and unsupported stores still produce a terminal module/RVA
record; this policy does not emulate arbitrary application instructions. Modes1
and2 remain explicit diagnostic choices; resident World sessions default to3.

World guard V2 has explicit begin/end ownership. During the interval the mandatory
World guard retains fault ownership, and premature snapshots/history invalidation
are refused. At the end, private binding snapshots through the private view while
the public view remains inaccessible.

Held DCs are initialized from current history. A new GPU generation must not
invalidate that physical history while its DC lease is live. This is an ownership
rule; it does not turn a copied GDI DIB into an alias of arbitrary CPU writes.

All43 optional producer handlers already exist. Original-reference controls,
controlled native integration and live caller reachability remain separate proof
levels; absent live routes cannot be relabeled as exercised by fixture counters.

## Execution evidence

The [PE32 controls](native-cpu-coherence-joined-controls-20261018.json) pass five
schedules,19 restorations and37,909 decoder checks. A foreign observer sees
NOACCESS throughout protected stores/snapshots; a foreign raw load terminates
with its module-bound fault record. Held-DC invalidation and foreign release are
refused. All twelve identity mutations reject before restoring; full/partial
Locks, same-value writes, AH/STOSW stores and byte-only Unlock are exercised.

[Actual native OpenGL replay](native-cpu-coherence-joined-gl-20261018.json) independently
checks9,120,000 reader WORDs and9,600,000 presentation WORDs over20 frames. It
records19 explicit reader snapshots, zero ordinary World readbacks and complete
surface retirement. Presentation inspection adds20 diagnostic readbacks and is
counted separately. [Legacy schedules](native-cpu-coherence-joined-legacy-20261018.json)
and [eight policy tests](native-cpu-coherence-joined-policy-20261018.json) also pass.

[Producer regressions](native-cpu-coherence-joined-routes-20261018.json) retain five
independent original-reference suites: panel22,272 cases; pixel/fade5,028 native
cases; minimap486 terrain,2,054 overlay and756 border cases; colour3,054 native
cases including256 empty rectangles; raster9,323 native cases and4,467 workspace
cases across all23 splice sites. Supported original preparation, CPU-ready
planes and ABI/physical pixel comparison are their scope. Explicit original
fallback/refusal controls remain in these suites; they do not establish original
GPU-history equivalence or live caller reachability. Original-manifest bookends
pass for every suite. The colour fixture counts action0 CPU reads separately
from action3 protection metadata, preserving the empty-pixel-read assertion.

[Acquisition provenance](native-cpu-coherence-joined-acquisition-20261018.json) preserves
immutable inputs and unsuccessful/superseded attempts. Current execution claims
bind complete declared compiler/test sources; historical records are unchanged.


Reader-kind counters update in memory under existing serialization. Their file is
persisted only at the common successful joined-shutdown boundary, covering both
RenderShutdown and automatic RenderStop. The final PE32 fixture uses RenderStop,
then checks idempotent RenderShutdown and the exact19-ticket counter file; it
also requires that no counter file exists during active reader work. This removes
counter file I/O from tracker/copy ownership. Prior live failures include the
tracker-held diagnostic stall, its downstream session/history refusal, and a
successful original journey whose counter file was absent on automatic stop;
[acquisition records](native-cpu-coherence-joined-acquisition-20261018.json)
retain these boundaries. [Source-mode restoration](native-cpu-coherence-source-modes-20261018.json)
records the compact snapshot's shell execution setup correction without changing
source bytes or granting failed setup runs acceptance credit.


## Bounded original-game investigation

The [joined-shutdown live journey](native-cpu-coherence-joined-live-20261018.json)
passes under mode3:14 balanced World imports/returns,41 explicit GetDC snapshots,
1,155 native producer calls, zero forwards, unknown-reader faults or forbidden
history/transfer records, and normal original exit. Gameplay, Mini Cancel,
Quit No and Quit Yes complete; original-manifest bookends, stable staged/source
inputs and stopped private Wine state pass. The counter file is emitted by the
shared automatic-stop path and exactly matches the last41-ticket CPU reply.

This advances protected storage to bounded live observation. Active typed-Lock,
byte-Unlock and other typed-unborrowed counters are zero in this journey; their
new restoration implementation remains current headless integration, backed by
the separate PE32/GL controls.27 installed producer routes remain unexercised
live. Ordinary64-World/FPS acceptance remains false, with no original GPU pixel
equivalence or full live-replacement promotion. The final current code is the
joined-shutdown source version; earlier control/acquisition records retain their
hashes and explicit historical limitations.
