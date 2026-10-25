# Public SHM upload source diagnostic, version 1

The explicitly selected storage observer links a separate diagnostic dependency,
`runtime/display/egl_shm_source.c`. Ordinary EGL observation and native rendering
are unchanged. The dependency observes public XCB SHM attach/detach and upload
calls, queries target drawable geometry, and maps an associated public SysV
segment with `SHM_RDONLY`. It never changes upload arguments or pixel bytes.
Extra round trips and read-only attachments perturb timing and attachment counts;
this is not a neutral or universally complete ownership observer.

`shm-source-<pid>-<sequence>.bin` contains exactly 192 bytes: magic `MNMESH01`
followed by 46 explicitly encoded little-endian u32 values. Sequence is bounded
to 16 per process; each pixel sample is bounded to 8 MiB. Initial drawable use,
extent changes and one following upload select samples. Selection uses geometry,
never a frame number, image colour or reference oracle.

| u32 index after magic | Meaning |
| --- | --- |
| 0–1 | Version 1, exact metadata size 192 |
| 2–4 | PID, TID, process-local sample sequence |
| 5 | Status: 1 accepted; 2 unknown segment; 3 unsupported layout; 4 bounds/allocation; 5 unstable; 6 mapping failure |
| 6–9 | Local connection label, attach generation, XCB segment XID, public SysV segment ID |
| 10–13 | Queried target width/height and preceding observed target width/height |
| 14–17 | Pixel bytes, public segment extent, original forwarded cookie, before/after byte equality |
| 18–19 | Selection: initial 1, extent change 2, following 4; reserved zero |
| 20–23 | CLOCK_MONOTONIC begin/end, low/high u32 pairs |
| 24–39 | Same sixteen scalar arguments as the existing XCB image witness |
| 40–42 | Preceding observed upload image width/height; duplicate pre-call read equality |
| 43–45 | Incoming and original returned errno; extent source 2 = public XCB geometry |

The argument order is drawable, total width/height, source x/y/width/height,
signed destination x/y encoded as u32, depth, format, event, left-pad, data bytes,
segment XID and byte offset. Admission requires depth24 little-endian 32-bit
ZPixmap pixels, compatible setup format/padding, zero destination, complete
visible coverage, and image bytes entirely within the public segment. No host
address or C object layout crosses this record. IDs and attach generations are
local observations and must not be assumed stable across launches.

The matching `.bgra` file is the exact full image sample, including padding,
starting at the submitted byte offset. Two reads before the forwarded call and
one after it must agree before causal admission. This is bounded observed
stability, not all-thread quiescence or complete backing ownership. Original
arguments are forwarded once, and the original cookie/errno are returned.

`shm-source-<pid>-000000.terminal` is six little-endian u32 values: version, PID,
sample count, exhausted selection count, attach generation count and connection
label count. Missing terminal, gaps, unsupported samples, exhausted budget,
changed bytes or ambiguous packet binding refuse full sample-cohort admission. A sample
binds to exactly one enclosing existing XCB witness by PID/TID, all arguments,
cookie, target extent and monotonic interval. Pixel samples remain external
comparator oracles; native reconstruction never reads them.

A separate finite diagnostic may read an individually complete file pair from a
failed/terminated acquisition. It must bind the point to an independently closed
EGL physical interval, require complete points for every observed shrink in that
interval, and retain the acquisition failure. This grants no process/sample-cohort
completion, normal Quit, immutable producer-input or native-session acceptance.
The strict `samples()` reader still refuses a missing terminal. Point admission
must be explicitly labelled `individually_complete_points` in the result.

Confidence: provisional until the current source-bound controls and original
observations recorded in the associated investigation complete. Private driver
allocation internals, unchecked/asynchronous server error neutrality, unknown
routes and arbitrary concurrent writers remain outside this contract.
