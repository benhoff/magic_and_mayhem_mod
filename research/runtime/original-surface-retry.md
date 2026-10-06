# Original surface Restore and retry scheduling

2026-10-06. High confidence within scripted fault scheduling; driver loss and
restored pixels are separate pending evidence.

The [capture](original-surface-retry-capture-20261006.json) executes unchanged
no-CD instructions in a private i386 PE mapping with hash-checked input and
original manifests verified before/after. Four wrappers receive explicit COM
result schedules: success, lost→success, busy→success, generic error→success,
two losses, busy→lost→success and lost→busy→success. Both Restore results, key
success/failure, key-enabled global, optional source/destination reload callbacks,
Blt/BltFast and WAIT branches produce 4,480 retained input/ordered-call traces.
No Wine or DirectDraw driver is launched; fault injection is evidence of original
wrapper branching, not physical lost allocations or driver Restore behavior.

Confirmed rules:

- Partial fill `0x58bac0` draws once. Lost invokes destination Restore; successful
  Restore optionally reapplies the low16-bit saved source key and calls a nonnull
  reload callback with zero. It returns without repeating the fill.
- Full fill `0x58bc10` draws once, then after a lost result attempts destination
  Restore and exactly one additional full fill even if Restore failed. A second
  lost result does not cause another Restore or retry.
- Opaque rectangle `0x58c4a0` retries every nonzero draw result until success.
  Lost results restore source then destination; keyed rectangle `0x58c8a0`
  uses the same source-before-destination order. Either Restore failure still permits the
  other surface's Restore and the next draw.
- Only a successful Restore reapplies the saved source key and invokes that
  surface's optional reload callback. SetColorKey failure is ignored; the reload
  callback and subsequent work continue. Key values are truncated to16 bits.
- Error dispatcher `0x484c00` suppresses modal UI for the captured lost result;
  busy and generic error reach the bounded MessageBox/DestroyWindow observers.
  UI imports are rebound in private fixture data; original text is unmodified.

[Offline input-derived comparison](original-surface-retry-cpu-20261006.json)
checks every ordered event and consumed-result count. Eight regression/refusal
tests cover provenance, order, consumed counts, key failure continuation and a
finite schedule refusing an endless-error loop.

```sh
python3 tests/test-original-surface-retry.py
python3 tools/check-original-surface-retry.py --report working/tests/retry-new.json
```

Native implementation is pending in this findings commit. Unbounded original
copy loops require an explicit native execution budget; terminating on that
budget is a native policy, not an original success. Unknown restored pixels must
remain unknown until an actual reload or overwrite establishes their contents.
Calling the reload callback alone does not establish its success or pixel bytes.
Real driver loss/Restore, palette retention, asset reloader internals, the `-1`
reload sentinel route `0x4a2f90`, null-interface mutation during recovery, window
translation during retries and live adapter/replacement remain pending.
