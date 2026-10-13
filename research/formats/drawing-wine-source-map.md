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

## MNMWMS02 broader bounded mode

Version2 keeps the sample layout and replaces the fixed two-shape policy. Header
word6 is the positive attempt cap (at most8192), word7 is1, word11 the positive
retained-byte cap (at most2147483648), words12/13 the800/600dimension limits and
words14/15zero. Shape word35 is1 for an eligible RGB565 source and0 otherwise.
Eligibility requires MAP_R, no multisampling, depth1 and descriptor byte size
`width*height*2`. GPU residency is not required. Word31 is the global attempt
number, or attempts already used for a quota refusal. Both attempt and retained
byte caps apply before calling map. Failed attempts consume the attempt cap.
Terminal words5/6 retain global attempts and retained bytes. Refusals remain
rows and cannot be renamed a complete source interval. The runtime accepts
strict positive decimal lower caps through `MNM_DRAW_WINE_SOURCE_MAP_ATTEMPTS`
and `MNM_DRAW_WINE_SOURCE_MAP_BYTES`; the interactive runner clears inherited
values and uses the declared8192/2GiB defaults.

## MNMWMS03 padded source storage

Version3 uses `MNMWMS03`/version3 with the version2 row/header layout and budgets.
Descriptor size may cover padding: require at least dense byte size and at most
`height*32768`. The public map's validated row pitch determines readable row
spans; each retained row still contains exactly `width*2` bytes. The public
598×36 descriptor size43200 therefore admits43056dense retained bytes. Version2
continues to require exact dense descriptor size so its historical refusals keep
their original meaning.

## MNMWMS04 source read clock

Version4 keeps version3geometry/budgets and adds two little-endian64-bit Windows
QueryPerformanceCounter values in sample words39/40and41/42. The first follows
exclusive diagnostic admission and precedes source queries/read; the second
follows copy/unmap, before the final lifetime snapshot and retention. Values must
be nonzero and nondecreasing. Reserved sample words now start at43; terminal
layout is unchanged. Both clock calls are metadata reads, separate from the four
counted extra public Wine calls. Legacy versions remain exactly decodable.
Counter intervals delimit observed accepted updates; temporal containment is not
producer identity, actual worker consumption, source atomicity or presentation.
