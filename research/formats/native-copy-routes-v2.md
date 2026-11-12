# Application copy route census version2

The opt-in all-copy observer emits `lock-capture/copy-routes-v2.bin`. It is a
versioned scalar observation file, with no host pointers or shared C++ layouts.
The128-byte header uses magic `MNMCRT02`, version2, row size176 and capacity65536.
Header meanings and fields0..39 match version1, including explicit terminal-tail
exclusions and fail-closed active misses/capacity exhaustion.

Each176-byte row appends four little-endian DWORDs:40 first emitted COPY sequence,
41 last COPY sequence inclusive,42 logical source resource and43 logical target
resource. CPU-prepared unclipped copies bind one command; clipped CPU copies bind
the complete contiguous piece interval. Each piece preserves the caller source
to destination translation, source key and logical resources. Empty clipped CPU
plans have no COPY interval. Whole-output closure refuses these rows because
there is no independent CPU plan-piece count to distinguish emptiness from lost
commands; supplied rendering controls cover empty clips separately. GPU-prepared copies bind
the existing GPU census interval, including its single frozen-source snapshot for
self copies. No-command/fill/admission failures have no COPY interval and cannot
supply emitted-copy output proof. Object identity tokens in the original columns
remain process-local; module base/extent is captured per run.

Version1 remains the legacy World-ancestry diagnostic format. Consumers explicitly
recognize both formats and refuse malformed, mixed or incomplete observations.
No new wire fields change rendering admission, authority, copying or fallback.

The CPU clip preparation follow-up retains these offsets and the generic inclusive
interval representation. Earlier readers that require a single CPU command reject
multiple-piece CPU rows; they cannot silently count a partial interval as complete.
Fills remain UPDATE operations and receive no COPY pixel credit. Independent
complete destination checks accompany emitted COPY pieces; clipper attachment
alone does not establish geometric reduction or natural GPU-source reachability.
