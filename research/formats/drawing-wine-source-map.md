# MNMWMS01 diagnostic mapped source

Little-endian64-byte header and256-byte rows, version1, build40209ca7,16384rows,
16map attempts for each of two declared shapes, at most four extra public calls.
Samples retain submission/API/thread/source-generation/subresource/resource IDs,
query/map/unmap results and errors, exact lifecycle prefixes, public descriptor,
pitches, per-shape attempt, payload ordinal/size/CRC32 and READ flag. No mapped
pointer is exported. Payloads are dense little-endian RGB565, copied while mapped.

Status0 retains an owned payload;1untrusted/query refusal;2unsupported descriptor;
3budget;4observed existing map;5map failure;6invalid range;7generation change;
8allocation/storage failure;9unmap failure;10overlapping diagnostic invocation.
Statuses8/9/10abort. Terminal type4 records stop reason and sample/payload/attempt
counts. A stopped or incomplete stream cannot pass as completed acquisition.
