# Native panel loop splices

The three No-CD panel wrappers now have an optional native pixel-loop adapter.
It retains the original surface Lock, Unlock, caller stack cleanup and wrapper
origin translation. It replaces only the admitted bevel, highlight and whitewash
passes. This is a partial producer implementation, not complete rendering takeover.

| Wrapper | Post-Lock splice | Original Unlock tail | Domain |
| --- | --- | --- | --- |
| Bevel `0x58ddf0` | `0x58de5a`, 10 bytes | `0x58e28d` | Prepared local rectangle, percentage 0..1000, both boolean branches |
| Highlight `0x58d8c0` | `0x58d92c`, 8 bytes | `0x58db5f` | Asymmetric inclusive edge ordering |
| Whitewash `0x58e2c0` | `0x58e32c`, 10 bytes | `0x58e40b` | Half-open rectangle |

The freestanding kernel accepts RGB565 and RGB555, initialized owned storage,
positive dimensions up to 2048, and word strides up to 4096. Complete footprint
admission precedes writes, preserving repeated corner visits and row padding.
Bevel uses the original wrapper's already translated endpoints; it does not
guess or cache a live origin. Format/masks, every readable/writable memory region,
the thread owner and every splice/tail signature are checked before admission.
Only explicit `MNM_PANEL_TAKEOVER=1` installs this independent DLL.

The native menu staging/runner now admits that selection only with native
menu/render/drawing hosting, a hash-pinned source executable and an owned matching
DLL. Inherited environment settings cannot activate an unstaged adapter. Use
`MNM_PANEL_TAKEOVER=1 ./tools/run-qt-shell.sh` for the optional partial route;
the normal default remains unchanged. The source installation itself must match
the pinned executable hash; staging rejects an already patched source.

Entry assembly retains x87/SSE and direction flags across native work. Original
tails retain caller ABI and surface callbacks. Private intermediate loop scratch
and arbitrary register values at intermediate instructions are outside the
contract. Failed admission forwards the original pixel loop once, before native
writes. Installation rejects a late signature mismatch without earlier edits;
successful installation is idempotent. No synchronous diagnostic file I/O occurs
in an admitted drawing call.

The reproducible comparison runs the production installer/resolver/entry/kernel
against separately executed pinned original wrappers in a private PE mapping.
It supplies external Lock/Unlock and Win32 memory/API adapters on Linux. This
establishes only its isolated comparison scope. Real Wine/DirectDraw leases,
concurrent writers on a shared surface, original Lock failure/recovery, integrated
producer-history observers and public gameplay remain pending. No live replacement
status is claimed from an isolated loop bypass.

Before running, prepare a prospective declaration for `RS.panel-loop-state`,
`NP.panel-loop-admission` and `NP.panel-loop-launch`, then use the storage guard:

```sh
python3 tools/draft-coverage-claims.py --behavior RS.panel-loop-state \
  --behavior NP.panel-loop-admission --behavior NP.panel-loop-launch \
  --output working/tests/panel-splice-claims-next.json
python3 tools/working-retention.py run --reserve-gib 0.1 -- \
  python3 tools/test-panel-splice.py --claims working/tests/panel-splice-claims-next.json
```

Historical panel-domain evidence stays unchanged. New result files and evidence
IDs must retain the actual compiled source and original input hashes.

The current [comparison](panel-splice-staging-comparison-20261013.json) passes 22,272
original/native wrapper executions and 5,066,880 full physical WORDs, both pixel
formats, nonzero bevel origins, padded rows, degenerate/edge rectangles and
percentage/boolean branches. All caller GPR, defined flags/DF, selected full FX,
Unlock arguments/count/volatile GPR and LastError match. The 1,392 different-thread
refusals execute the original loop once; late signature refusal makes no earlier
patch. Sanitizers pass 131,072 independently expected whitewash words and 34
atomic refusals. The production PE32 DLL builds and eight synthetic launcher
activation guards pass. No public game or original surface recovery is claimed.
The positive staging increment also exercises the actual pinned-source guard,
DLL build/copy, executable import appending and session activation on an empty
disposable fixture. It launches no game. The earlier isolated comparison remains
at its original recorded source hashes.

Earlier fixture build failures and the sandbox's 32-bit syscall refusal remain
retained in `working/tests/panel-splice/run-5u98y8ve/`, `run-4rkrppnz/` and
`run-4droyhnl/`. The subsequent isolated comparison uses the same original hash
with the required 32-bit process access; thresholds and original wrappers remain
unchanged. All source-bound result hashes are preserved.
