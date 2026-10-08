# Startup queue diagnostics v1

Private observation files `startup-queue-NNNN.bin` retain every consumer entry in
an explicitly bounded prefix of 1..256 queues, before scene sampling or native
admission. They are never native renderer inputs. Raw addresses belong only to
the captured process. Files contain a 64-byte header followed by zero or more
unmodified original 36-byte draw rows. Integers are little-endian uint32.

| Byte offset | Meaning |
| --- | --- |
| 0 | Eight-byte magic `MNMSTQ01`. |
| 8, 12, 16 | Version 1, header size 64, complete file size. |
| 20 | Consumer entry sequence starting at 1, independently of scene samples. |
| 24 | Successfully saved scene sample number, or zero for no saved sample. This does not assert successful effective World capture. |
| 28 | Raw capture status: 0 complete (including empty), 1 unreadable queue header, 2 count exceeds capacity or 12,320-row bound, 3 unreadable row storage. |
| 32, 36, 40 | Original view, row count, capacity (zero when header unreadable). |
| 44, 48 | Raw process-local queue and row-storage addresses. |
| 52, 56, 60 | Row size 36, captured row count, reserved zero. |

The original row's nine words retain depth, frame pointer, x, y, shade, the
uninterpreted word at +20, kind at +24, auxiliary word at +28 and the word at
+32. Kind and coordinates can be interpreted as signed integers without losing
their original uint32 bits. Hidden kind -2 remains admitted. Unsupported means
outside the effective World observer's current kind admission, not unsupported
by the original game. The decoder reports every unsupported row, its zero-based
ordinal, signed and hexadecimal kind, coordinates and process-local frame pointer.

Allocation/file failures leave missing or incomplete entries; the harness refuses
the capture rather than silently omitting queues. The decoder requires exactly
the requested prefix, matching contiguous filenames/sequences and unique nonzero
sample correlations. An invalid queue still has its own header-only record.
Reentrant entries rejected by the observer's existing busy guard and arbitrary
concurrent queue producers remain outside this single-engine-thread scope.

The decoder's `startup_replay` mode admits selected wave kinds16/17/20.
`unsupported_draws` and `unsupported_kind_counts` reflect that effective mode;
`default_policy_unsupported_draws` and `default_policy_unsupported_kind_counts`
retain the conservative default-policy result for the same actual raw rows.
The wire bytes and all kind values are unchanged.
