# Native music channel v1

`MNMMUS01` is a1024-byte little-endian owned file, defined by
`protocols/schemas/music-v1.json` and generated C/Python bindings. It transports
native music commands, scalar status and a CD track table; process pointers,
HWNDs and C++/Qt objects never cross it. Local PE compatibility keeps its own
callback HWND and translates native completion events into original messages.

Bytes0..15 hold magic/version/declared size. Producer words at16..59 publish
request sequence/ID, operation, track, relative CD-frame endpoints, flags,
cancellation ID, installed features and local unsupported-CD/legacy-other-MCI
counters. Host words64..151 publish a seqlocked response, status/value,
heartbeat/readiness, playback phase/track/position/volume, completion event,
request/play/fault/recovery counters and decoded frame count. Track count and
leadout are explicit. Bytes160..951 contain up to99 pairs of start/end CD
sectors, indexed by track number. All reserved bytes remain zero.

Requests and response snapshots use independent odd/even publication sequences.
Single producer/host owners retain matching IDs; no retry may duplicate an
accepted command. Startup admits a fresh session file and withdraws readiness
at stop. Commands have bounded host acceptance and stale-heartbeat deadlines.
Completion has a separate event sequence/ID so asynchronous playlist messages do
not overwrite command ACKs. Endpoint/cancellation/device-loss semantics and
compatibility routing are implementation work in progress, with no validation
or retirement claim from schema presence.
