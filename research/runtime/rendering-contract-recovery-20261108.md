# Font installation and DC caller recovery — 2026-11-08

CR08 recovers two bounded original caller contracts behind the pending
`operation.text-font-dc-lifetime` and `operation.flags-and-return-values`
requirements. It does **not** close either requirement in either player profile.
The selected DF08 inventory remains 102/156 passed, 54 pending, with both final
decisions pending. No game, renderer takeover, gameplay rule or original file is
changed. The new portable models are separate from runtime hooks and widgets.

The original build is NoCD SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
All five functions already have exact central binary-inventory ranges. Their
entire compared bodies remain unchanged in the private mapping. Only named
downstream supplier entries/IAT/fixture vtables are redirected, with the complete
original file hash checked before execution. Immutable manifests bracket each
attempt. Confidence is high within these explicitly bounded normal-return cases.

## Font/resource installation coordinator

`RS.screen-font-installation` models all local normal-return branches of
`5575b0..557949`. It formats resolution-dependent heading/body names, loads the
four font roles and two sprite resources, writes all four font tracking fields
at object+230 to 2, associates consumers with their resources, selects the
resolution-indexed camera table and performs the optional screen/widget setup.
The heading and body names read the resolution separately: a first-load supplier
can change the second selection. The original tests five resource results and
does not test the sixth sprite result. If the supplied error reporter returns,
the coordinator continues; actual fatal-error/exception behavior is not inferred.

The optional screen branch publishes 640×480 or 800×600 rectangle data, three
200-valued fields, selected text identity and the default string. The other
branch publishes self+22 and returns after clearing self+26. All ordered supplier
arguments, owned/global bytes and callee registers/stack are checked. The finite
512-case matrix covers both resolution values, mode/owner/cursor branches,
load failures, an error reporter returning, first-load resolution mutation,
palette-request failure and the original unchecked sixth result.

This is a coordinator model, not another SFT loader. The actual file/heap/font,
sprite, widget, cursor, imported-time and error bodies remain named supplied boundaries.
It does not retire earlier font allocations. The independently recovered
`4a5ae0` reload retention in [font provider recovery](actor-font-providers-20261022.md)
therefore remains relevant; calling installation does not establish leak-free
replacement or observed destructor/atexit execution in a real session.

The pinned import table identifies IAT `5c52e8` as `WINMM.timeGetTime`. The
coordinator forwards its DWORD to supplied consumer `54e170`. The initial model
used the incorrect local name `thread`; its address and value flow were tested,
but the name was not supported by an import-table check. The corrected model
uses `stamp`, and the producer now refuses a different DLL/symbol identity.
Actual API timing, wrap behavior and consumer semantics remain supplied.

## DC image wrapper caller contracts

`RS.dc-image-wrapper-callers` models complete normal-return control flow for
`58d1a0`, `58d240`, `58d280` and `58d370` with supplied image/COM endpoints:

- Load variants construct and destroy the temporary image on both load outcomes.
  Their EAX is the normalized image-load success, even if GetDC, drawing or
  ReleaseDC fails. The unpositioned loader can return success with no surface.
- Both unpositioned wrappers guard a null surface. The positioned variants
  require a nonnull valid surface; unsafe null dereferences are not executed or
  relabeled supported native behavior.
- GetDC HRESULT is ignored. An unwritten DC output retains the load variant's
  incoming path argument or the draw variant's saved receiver. The image helper
  receives a pointer to that same slot and can rewrite it before release.
- Release rereads wrapper+8 after the image supplier. It can therefore receive
  a different surface than acquisition. Draw variants return ReleaseDC EAX.
- In the draw variants, POP ECX reads the reused GetDC output slot. Returned ECX
  is the final HDC, not necessarily the incoming receiver. Load variants expose
  the restored entry SEH-chain value in ECX and the final HDC in argument slot 0.
- Callee registers, exact RET4/RET12 stack movement, stack guards and restored
  FS exception-chain identity agree. Load variants' defined final ADD ESP24
  flags are predicted from the supplied original entry SP. Draw variants retain
  the declared supplier flags, or the null-surface TEST flags. Undefined AF after
  TEST is excluded. Both empty and nonempty supplied entry exception chains run.

The 1,024-case finite matrix includes independent failure/result/output-write,
DC-slot rewrite, changed surface and prior-SEH choices; the positioned cases
retain their nonnull precondition. Negative X/Y values are forwarded unchanged.
The image, COM and GDI implementations are supplied: no actual pixel, device,
GDI object selection, Windows SEH delivery, LastError or physical-driver
equivalence follows from this fixture.

## Comparison and retained failures

[Comparison 04](rendering-contract-recovery-results-20261108/comparison-04.json)
recorded 1,536 passing cases. The 32-bit harness independently resets inputs before
each native-model and unchanged-original execution and compares the complete
selected image-data/owned-memory byte arrays and ordered callback vectors
directly, in addition to original ABI assertions. The 64-bit release and
ASan/UBSan models produce identical finite case transcripts and diagnostic
FNV state/event fingerprints. Those fingerprints are diagnostics; original/native
byte equivalence is established by the in-process full comparisons, not an
unproven collision-free FNV property. Report and artifact custody uses SHA-256.
Native inputs never contain original output bytes.

[Comparison 05](rendering-contract-recovery-results-20261108/comparison-05.json)
freshly reruns those 1,536 cases with the corrected imported-time scope, renamed
native temporary and explicit pinned import-table identity check. All original,
native and sanitizer results match. It does not execute the actual time API.
The old execution and hashes remain intact; its incorrect semantic name is not
used as a new thread-provider claim.

The initial coverage audit rejects attaching only `isolated_original` evidence
to the headless scenarios. Its full result, summary, log and exact register are
retained as `audit-refused-01` artifacts. The corrected registration names the
already executed native64/sanitized fixture as `CR08.callers-native04`, separately
from `CR08.callers-original04`. Both resolve the same immutable comparison bytes;
no model, scenario contract, historical result or engine/player status is changed
to correct this evidence-kind mismatch. Native integration status remains none.

The earlier [corrected audit](rendering-contract-recovery-results-20261108/audit-02.json)
has zero errors and zero validation gaps at its original comparison04 source
phase. Integration and replacement remain none. Existing unrelated historical
staleness remains. The earlier [closeout](rendering-contract-recovery-results-20261108/closeout-01.json)
verifies all 76 preceding new artifact payloads (150,230,960 logical bytes) and
the exact unchanged four pending player rows. It makes no commit-hook claim.

The current [audit03](rendering-contract-recovery-results-20261108/audit-03.json)
has zero errors and zero scoped validation gaps for comparison05. Both scoped
implementations and original comparisons have current complete-source support;
integration/replacement remain none. [Closeout02](rendering-contract-recovery-results-20261108/closeout-02.json)
verifies 104 preceding payloads and preserves the 102/54 player inventory.
The two incoming heads `85dcfea26` and `d76127603` have 278 exactly reviewed
committed file versions. The original combined 163-version receipt in
[history04](rendering-contract-recovery-results-20261108/committed-history-04.json)
passes the hash lookup but is rejected by the normal finish because it repeats
the journal path for distinct commits. It was never committed or accepted by
the gate. Its exact proposed journal and complete old SQLite cache, including
drafts and auxiliary data, remain preserved with a verified logical restore.
The [rejected attempt](rendering-contract-recovery-results-20261108/committed-history-05.json)
is retained separately from the corrected active journal. Fresh per-commit
receipts in a separate review database leave zero gaps in
[history06](rendering-contract-recovery-results-20261108/committed-history-06.json).
No old database row or historical report is rewritten. The
[current closeout](rendering-contract-recovery-results-20261108/closeout-03.json)
also checks every current/archived receipt's shape; it does not claim the final
commit gate has passed.
The concurrent X11 staging source changes do not intersect this comparison's
dependencies. These are separate source/metadata phases, not a renewed public
player execution or an assertion of a past gate result.

Three earlier failures remain immutable, with every exact source preimage,
compiler log and tested binary retained through SQLite:

1. Comparison 01 catches the probe's PUSHFL overwriting the dead RET4 argument
   before it was sampled. Flag-neutral MOVs now preserve the slot first.
2. Comparison 02 rejects the incorrect expected saved-receiver ECX and reveals
   the reused-HDC return contract. The model now explicitly returns EAX, ECX and
   first-argument state, including the supplied prior SEH value.
3. Comparison 03 passes all original/native32 cases, then GCC16 rejects a vector
   insertion with an array-bounds diagnostic during native64 compilation. An
   exactly sized snapshot with explicit copies compiles without weakening warnings.

All generated research results, source preimages, logs, dependency records and
tested executables are SQLite-backed references. Compiler scratch is temporary
RAM output; tested executable bytes and reproduction commands are durable.

## Acceptance boundaries and next proof

Both targeted operation requirements remain pending for default and software.
The new evidence does not assert current public-player execution or renew any
historical glyph, word-route, font-provider or player source fingerprints.

Text/font/DC closure still needs an actual installed trace linking font
acquire/load, selection/use, reload and teardown to the same resource generations
and independently checked presented glyph output. DC image handoffs must remain
separate from SFT text: a DC call alone is not font drawing.

Caller-state closure still needs the required production bypass/caller inventory,
actual caller-used AX/register/flags/state effects and associated pixel proof.
The known glyph and word comparisons retain their narrower floating-point,
Win32 admission, reentry and live-source boundaries. No synthetic supplier,
private SEH cell, successful wrapper result or normal process exit substitutes
for those missing installed observations. These gaps also remain in the original
recovery and final supported-player decisions.

Reproduce the scoped comparison in a fresh owned worktree with resolved local
SQLite evidence and a read-only original input link:

```sh
python3 tools/working-retention.py run --reserve-gib 1 -- \
  python3 tools/test-render-contract-recovery.py --name comparison-next
```

Use a lowercase unique acquisition name. The runner records exact source changes
against its Git base before compiling, verifies declared compiler inputs, retains
both success and failure results, and does not launch the game or set player rows.

The final integration joins corrected recovery commit `1e41f22aa` with master
`a04895c2b`, including its selection and finite scene/attachment acceptance.
All nine comparison05 source fingerprints and all 186 sources of the retained
218-control scene result match the combined tree. The comparison remains a
1,536-case isolated proof; the merge adds no execution or player acceptance.
Master's 108 passed and 48 pending requirements remain unchanged, including both
font/DC lifetime and production caller-state requirements in both configurations.
The merge review preserves every parent receipt and records all 24,172 incoming
committed file transitions with exact hashes, including 22 retrospective metadata
custody bindings. See the SQLite reports under
`research/runtime/rendering-contract-master-merge-results-20261108/` for conflict
preimages, history, applicability and the current combined audit. Historical
scope/freshness gaps remain explicit; no broad source-hash refresh is asserted.
