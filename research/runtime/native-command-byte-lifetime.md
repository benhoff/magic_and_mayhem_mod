# Native command byte lifetime — 2026-10-16

Four cumulative byte ceilings are repaired: the actual owned session emitter,
32 MiB private queue, 1 MiB mapped transport and streaming decoder. The initial
[code review](native-rendering-gap-review-20261016.json) reproduced two of these;
following the complete producer/consumer path exposed the other two before final
validation. Continuous public Qt launches now create [V3](../formats/render-command-ring-v3.md).
V1/bounded files and explicit V2 keep their legacy budgets/refusals. No original
artifact is modified and no gameplay policy changes.

The [final source-bound controls](render-command-lifetime-controls-06-20261016.json)
pass actual production `session_record`/`session_emit`, queue and ring traffic of
4,297,064,448 bytes, and Qt adapter/decoder/consumer traffic of 4,295,064,708 bytes.
These independent paths verify bytes/owned-copy order and complete final output;
this is not solely accelerated cursor seeding. Eight rollover guards and eight
actual-emitter guards run natively and with ASan/UBSan. Existing writer/queue
controls and 146,800,640 cross-process bytes preserve V2 compatibility. The PE32
selftest build verifies its actual 53-source compiler closure without undeclared
inputs. Raw output: `working/tests/render-command-lifetime/run-3xhsd158`.

[Actual PE32 producer integration](render-command-v3-pe32-controls-20261016.json)
then runs hooked synthetic Lock/Unlock through the real owned producer, queue,
idle scheduler, Qt decoder and GPU. Three valid V3 archive policies (disabled,
record-exhausted, byte-exhausted) pass 210 independent complete display comparisons;
missing-present and bounded-V1 cases explicitly refuse. Raw output:
`working/tests/render-continuous/run-m5485fkl`. This establishes PE32 integration,
not original-game rendering equivalence or four-GiB PE32 execution.

Storage bounds, pressure pacing, FIFO, pinned identity/version, ACK progress,
cancellation, END draining, finite sequence/identity exhaustion and fresh-producer
failure policy remain. A single/nested callback can still exceed capacity and
no-progress deadlines still terminate. Unsupported history is not admitted by
wrapping byte counters. The [separate portable session](recorded-native-session-comparison.md)
validates recorded pixels, rather than extending these synthetic results to all
original-game paths.

## Fresh original-game attempts: no new Save/Load acceptance

The old [successful V2 storage/pressure journey](resident-command-pressure.md)
keeps its original source hashes and evidence. Fresh V3 attempts did not close a
new complete live journey:

- `working/tests/native-campaign/run-hdv9lamh` refuses unsupported source executable
  hash before launch; later attempts use the independently verified supported
  pristine source installation.
- `working/tests/native-campaign/run-sh6ibpik` / menu observer `run-78w1i5m0`
  plays enabled intros and refuses unknown initial history (GAP reason 4) before
  Main/World. Publication and ACK both reach 1,920,088, then explicit terminal
  cancellation. This is neither a rollover boundary nor queue capacity failure;
  without a matched V2 original control its cause remains unclassified.
- `working/tests/native-campaign/run-5qqr6c_l` / observer `run-l9sclquj`, with
  movies skipped, never starts original bridge traffic: no lifecycle/events,
  publication zero until cancellation, no World. The source freezer also reports
  a concurrent unrelated Cure source change. Original startup/loader diagnosis
  and a fresh supported run remain pending; no rendering regression is inferred.

Private processes are terminated and original manifest checks pass before/after
these consuming attempts. These failures must not be hidden by synthetic passes
or by refreshing old live evidence. Complete V3 Save/Load and enabled-intro
history remain explicit live acceptance work.

## Reproduce

Use a private renderer test build so live launcher reconfiguration of the shared
Qt build cannot disable test targets. The lifetime runner creates/configures it.

```sh
python3 tools/draft-coverage-claims.py \
  --behavior NR.command-ring --behavior NR.command-ring-adapter \
  --behavior NR.command-streaming --output working/tests/<new-lifetime-claims>.json
python3 tools/working-retention.py run --reserve-gib 2 -- \
  python3 tools/test-render-command-lifetime.py \
    --qt-build working/build/render-lifetime-qt-20261016 \
    --claims working/tests/<new-lifetime-claims>.json \
    --output working/tests/<new-lifetime-result>.json
```

Consult runner `--help` for the private Qt build path. PE32 integration uses
`tools/test-render-continuous-producer.py <renderer-build> --wire-version 3` under
a private Xvfb; it needs Wine and the bridge selftest, without original media.

Immutable development reports `render-command-lifetime-controls[-02..-05]-20261016.json`
retain wrong manifest-path, missing declared compiler-input and shared-build test
target failures separately from positive code checks. The earlier positive `-03`
precedes discovery/repair of the owned-emitter ceiling. `-06` is the complete final
four-layer result. No historical source fingerprints are refreshed.
