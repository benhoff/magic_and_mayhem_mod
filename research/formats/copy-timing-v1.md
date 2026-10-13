# Copy timing diagnostic file v1

Confirmed from native producer/reader source; controlled byte fixtures are the
validation boundary. This is optional observation storage, not shared pointers
or a game/save format. All integers are little-endian u32.

The 32-byte header contains `MNMCOPY1`, version 1, record size 100, QPC frequency
(low/high u32), capacity 512 and reserved zero. Zero frequency/clock indicates
unavailable high-resolution timing. Truncated/unknown/oversized input refuses.

Each 100-byte record has six common words: event, session, thread, GetTickCount,
QPC low and QPC high; followed by 19 payload words. Events:

| Event | Payload |
| --- | --- |
| 1 wait timeout | site,target,elapsed tick ms,failed attempts,wait calls,successful wait returns,start/end published acquisition counters,sampled thread changes,coherent/unavailable sample counts,max wait span tick ms,final owner coherence; words13–17 zero,word18 wait ID |
| 2 wait clocks | site,target,start QPC low/high,last pre-wait QPC low/high,last return QPC low/high; words8–17 zero,word18 wait ID |
| 3 owner sample | site,target,coherence,owner thread,published acquisition token,start tick,owner site,target,phase,start QPC low/high; words11–17 zero,word18 wait ID |
| 4 pixel discard | source tag,object,generation,surface pixel/metadata epochs,current pixel/metadata epochs,pixel-presence,width,height,bits,bytes,known-layout,primary,generation-origin/caller/owner; words17–18 zero |
| 5 checkpoint refusal | existing refusal reason,object,surface count,bytes,current epochs,surface epochs,layout,pixel-presence,width,height,bits,bytes,primary,pending pixel misses,pending alias reset; words17–18 zero |
| 6 recovery reset | old session,new session,checkpoint policy,current pixel/metadata epochs; words5–18 zero |

Timeout triplets correlate by session/thread/nonzero wait ID. The reader reports
incomplete triplets and refuses duplicates. A coherent sampled owner is not proof
it remained unchanged throughout the whole wait or until the worker wrote the
record. QPC points contain unscheduled time as well as time blocked on ownership.
Acquisition differences are modulo u32 and represent published diagnostic
acquisitions. Capture is bounded and can be partial even below capacity.
