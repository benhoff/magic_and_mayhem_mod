# Isolated software endpoint recovery

This fixture runs actual Qt media against an owned private PipeWire/Pulse graph,
with stock daemon configurations under unique names, no automatic Pulse sink or
restore modules, policy-only WirePlumber, isolated runtime/config/state/cache,
and no desktop session bus connection. The initial and final graphs must be empty.
Every control command identifies the owned server; only three named null sinks
may be created. All owned daemon process groups stop on success or failure.

The intended scenarios exercise initial real endpoint absence, output arrival,
active removal, caller pause across replacement, a distinct nonnull default
switch, and complete movie/file-sound decode continuity. These are software
backend observations. Physical hardware, audible tail/latency, and original
driver equivalence remain separate. The current actual software endpoint execution passes for both movie and
file sound. Each retains exact66150-frame decoded PCM; the movie retains all30
RGBA frames. Loss/recovery/default-switch counters are1/2/1, caller pause and
logical position remain stable through replacement/switch, and acceptance
occurs once. Both graphs begin/end empty and all owned daemon groups stop.
Evidence: native-media-backend-private-endpoints-20261012.json.
Stock config copies and actual per-phase reports are retained in the raw run;
physical/audible waveform/tail and whole original-driver equivalence remain
separate from this muted-output decoded-continuity result.

The preparation probe initially lacked routing policy, then inherited a site
recorder route, and then exposed Pulse's automatic fallback sink. Those results
remain under working/tests/private-audio-backend. Only run-h14iwvmx passes the
strict empty-before/empty-after graph check with explicit stock configuration
and policy-only routing. That capability probe is not a native playback result.
