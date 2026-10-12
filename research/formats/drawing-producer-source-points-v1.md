# Drawing producer source points v1

This is an intentional observation protocol, not recovered engine storage.
`MNM_DRAW02_PRODUCER_BIND=1` requires physical API observation and the opt-in
producer recorder. `SceneDrawingSourcePairV1` returns two little-endian64-byte
scalar snapshots under one recursive recorder critical section. It does not
hold that section across original drawing bodies or synchronize their bytes.

Each snapshot has16words: byte size,version,flags,producer sequence,World queue,
recorder failures,logical canvas generation,width,height,WORD stride,dirty
and sampled generations,outstanding return frames,caller primitive depth,
local caller thread ID,and reserved zero. Flags are trace1,live2,stopped4,
ambiguous8,and known generation16. Interface pointers stay inside the original
process. Unknown,ambiguous,stopped and pending states remain explicit. The
getter preserves LastError and refuses invalid or unwritable outputs.

The additive `api-producer-points.bin` has a64-byte `MNMDSRC1` header,version1,
192-byte records,build marker0x40209ca7,and maximum786433records. Each record
contains48words: ordinal,phase,API entry,API completed-record ordinal,method,
object,related interface,local thread,tick,result,physical ordinal,five reserved
zeros,then destination and source snapshots. Phases1/2/3 mean before original
call,after its return,and after physical acknowledgement. Completed-record and
physical ordinals exist only in phase3. Terminal phase4 instead records reason,
API terminal ordinal,pending calls,planes,drops and gaps. Admission cross-checks
the API journal and call intervals,paired recorder counters,phase identities,
results and closed producer generation prefixes. It never assumes global point
order is producer order across threads.

Optional acquisition retains the first1–16World-return producer prefixes and
all observed physical API witnesses. Source-only prediction restricts itself to
stable known/live prefixes outside World queues,without observed competing
mutators,and full opaque BltFast copies. Every exclusion stays in the result.
Native inputs are producer parameters and independently pinned encoded assets;
original output planes and recorded raster results are comparison-only. RGB565
uses the independently validated normalized-nearest Wine/Xvfb conversion.

The original03capture records17674points/5891surface calls,576physical witnesses,
49010producer records and16closed World queues with zero recorder failures.
Native prediction05compares106selected witnesses,104exact matches and two
300-pixel differences;470others remain excluded. This is a negative comparison,
not coherent presented-frame admission,byte ownership or complete journey credit.
See `research/runtime/draw02-end-to-end.md` for the continuing investigation.
