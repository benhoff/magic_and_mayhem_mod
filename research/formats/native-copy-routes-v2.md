# Application copy route census version2

The opt-in all-copy observer emits `lock-capture/copy-routes-v2.bin`. It is a
versioned scalar observation file, with no host pointers or shared C++ layouts.
The128-byte header uses magic `MNMCRT02`, version2, row size176 and capacity65536.
Header meanings and fields0..39 match version1, including explicit terminal-tail
exclusions and fail-closed active misses/capacity exhaustion.

Each176-byte row appends four little-endian DWORDs:40 first emitted COPY sequence,
41 last COPY sequence inclusive,42 logical source resource and43 logical target
resource. CPU-prepared copies bind exactly one command. GPU-prepared copies bind
the existing GPU census interval, including its single frozen-source snapshot for
self copies. No-command/fill/admission failures have no COPY interval and cannot
supply emitted-copy output proof. Object identity tokens in the original columns
remain process-local; module base/extent is captured per run.

Version1 remains the legacy World-ancestry diagnostic format. Consumers explicitly
recognize both formats and refuse malformed, mixed or incomplete observations.
No new wire fields change rendering admission, authority, copying or fallback.
