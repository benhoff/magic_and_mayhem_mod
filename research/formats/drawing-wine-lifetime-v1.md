# Selected imported Wine object lifetimes V1

`wine-lifetime.bin` is independent of the unchanged submission V1 journal.
Its64-byte little-endian header contains `MNMWLF01`,version1,rowbytes96,
build0x40209ca7,131072row cap and2048active-object cap;otherwordszero.

Each96-byte row has24u32words:0ordinal,1phase(entry1/return2/terminal4),
2callID,3method(texturecreate1/texturedecref2/swapchaincreate3/swapchaindecref4),
4Win32thread,5enclosing APIentryor0,6depth,7GetTickCount,8originalresult,
9originalLastError,10opaque submission-objectID,11observed lifetimegeneration,
12flags and13..23reservedzero. Entries have zero result/error/flags.

Successful create returns bind the returned opaque object to a new monotonically
allocated generation. Failure returns leave output identities zero. Flag1retains
an unreadable/null output on nominal success. Decref entries carry the current
observed generation or0forunknown. A zero original return retires a known
generation and setsflag2;nonzero refcount leaves it active. Reusing an address
after an observed final release allocates a fresh generation. No private object
layout,descbytes,data/sourcepixels or hostpointer crosses this channel.

Terminal carries reason inword2,totalcalls inword3,thread/tick inwords4/7 and
totalcreatedgenerations inword11;otherwordszero. Admission requires paired
entries/returns and exactgeneration transitions. Remainingactive objects are
retained at the explicit interval boundary,not silently called destroyed.

The observer covers only selected DirectDraw import calls. Internal Wine
creation/release,aliases,asyncworkers and incomplete interval lifetimes remain
open. Generation observation does not prove storage ownership or bind EGLframes.
