# Native player failure diagnostics

These are authored native diagnostic contracts, not recovered original file
formats. Producers are `runtime/scene/minimap_row_diagnostic.h`,
`runtime/render/command_queue.h`, `runtime/render/command_scheduler.h` and
`apps/qt-shell/native_render_worker.cpp`. Confidence in the layouts is high from
source inspection; a field's presence does not validate an engine behavior.

## Minimap row refusal v1

`producer-minimap-row-NNNN.bin` contains 96 little-endian u32 words (384 bytes).
The producer writes at most four records per process. Virtual addresses identify
only that launch and do not establish original alias ownership.

| Words | Meaning |
| --- | --- |
| 0–1 | Eight bytes `MNMMROW1` |
| 2–3 | Version 1, byte size 384 |
| 4–6 | Observed tag, return caller, object ECX |
| 7–12 | Owned destination ID, object, pixel base, width, height, word stride |
| 13–16 | Row-table base, marker index, signed projected row, signed checked delta |
| 17–18 | Computed table-entry address, signed 64-bit low/high |
| 19–20 | Expected row pointer, signed 64-bit low/high |
| 21–22 | Entry readable flag, actual pointer when readable |
| 23–27 | Thread, zero, queue, sequence, zero |
| 28–30 | Object cached grid width/height and object word stride |
| 31–37 | Matching tracked planes count; last matching ID, pixel base, stride, width, height, object |
| 38–47 | Reserved zero |
| 48–84 | Existing 37-word owned-minimap metadata snapshot |
| 85–95 | Reserved zero |

The initial diagnostic checked five entries. The corrected admission checks
only the consumed center entry (delta 0) and exact owned stride. Matching-plane
observations do not authorize any pointer alias. The diagnostic was not triggered
in Extended02; absence there does not identify Extended01's refused row.

## Command deadline diagnostics

`lock-capture/lifecycle.log` uses its existing reason plus nineteen hexadecimal
u32 value columns. `command_watchdog_state` is emitted after an actual no-progress
expiry. Values 0–6 are deadline start tick, clock sample, initial cached ACK,
freshly validated ACK, published cursor, private read cursor and private written
cursor. Value 7 is zero. Values 8–9 are cumulative counts of deadline rechecks
that observed real ACK progress or fully drained ownership. Remaining values
are zero. Tick and sequence arithmetic is unsigned u32; V3 cursor rollover keeps
its existing bounded-distance validation.

`command_pressure_summary` retains its first eight fields: waits, resumes,
maximum wait ms, high/low watermarks, peak private bytes, failure reason and
shutdown-complete flag. Values 8–9 now carry the same recheck counters, with
10–18 zero. These are process-local observations; they do not extend a timeout,
acknowledge GPU completion, or turn new producer traffic into peer progress.

The watchdog revalidates identity, cancellation and ACK immediately before a
no-progress refusal. Confirmed late peer progress renews its observation time;
fully drained ownership disarms it. Genuine no-progress, unsent private bytes,
forged ACKs and cancellation retain refusal. A deterministic old-code control
reproduces the former stale-ACK false expiry; live failures require their own
causal evidence.

## Terminal render timing

When the native worker saves a terminal error, it also writes
`resident-world-terminal.json` beside its normal working report. It contains the
worker report, including its error and completed phase/turn timing. It survives
the resident session's later, narrower teardown report. This working debug
payload is retained through the run's SQLite raw acquisition; it is not a second
durable JSON export. It does not reconstruct an unavailable crash stack or
measure time spent in a failed phase that never reached its timing update.
