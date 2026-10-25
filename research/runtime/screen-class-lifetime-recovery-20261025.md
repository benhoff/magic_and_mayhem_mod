# Concrete screen dispatch and callback ownership

This recovery extends the selection/cursor/control work adopted from local
commit `4eca3eb7` onto master `5ea14a4a`. The original image is NoCD SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
The models describe original wire storage and synchronous calls; they install
no native screen or cursor hook.

## Concrete dispatch witnesses

`tools/test-screen-lifetime.py` enumerates the 22 aligned non-executable tables
with shared tick `5595d0` at slot `+10`, retains their first twelve entries and
links every table to direct immediate vtable-store instruction witnesses.
Some store into an embedded object at parent+4; a parent pointer cannot be used
as that screen receiver. The report associates stores with existing inventory
function ranges when available. These static witnesses establish concrete
tables and construction/teardown assignments; they do not establish all dynamic
types, callers, indirect stores or runtime reachability. The global inventory
and its unknown ranges remain unchanged.

The six transition/draw/final targets at slots `00/04/08/0c/28/2c` are now
explicit for each table. Each target retains its own recovery boundary. A
resolved target is not equivalent to recovered callee semantics.

## Class 11 storage and resource lifecycle

The class11 constructor `4a8430..4a847e` sets table `5c6454`, class DWORD+4 to11,
clears active/fade/count/request/surface fields and callback+47, and initializes
the pointer vector at+1f. It allocates nothing. Unwritten bytes stay unchanged.
The shared vector initializer `4cab50..4cab69` copies the highest byte of its
original ECX receiver into vector byte0, then clears DWORDs+4/+8/+0c/+10. The
literal `push ecx; mov cl,[esp+3]` determines this byte; it is not invented zero
initialization or an unspecified host pointer conversion.

The destructor `4a8480..4a848e` resets the class table and tail-calls vector
retirement `4cab70..4cabcc`. Retirement collapses the admitted vector's end to
begin, clears+10, passes begin (including null) to `597870`, and clears the
three pointer fields after that callback. It does not destroy pointed-to
elements. The class destructor does not release callback+47 or clear the
surface alias. Full enclosing destruction orchestration remains separate.

Slot+24, `4a8500..4a8540`, creates a callback only if owner+47 is null. The
12-byte object has vtable `5c6484`, function `4a8550` and the screen receiver.
It rereads owner+13 after allocation and publishes the result both at owner+47
and surface+28. A null allocation publishes null in both places; no local retry,
unwind or alternate allocation exists. An existing callback skips allocation
and does not repair or republish the surface alias.

Slot+20, `4a84e0..4a84fa`, invokes callback virtual+4 with argument1 and clears
owner+47 after return. It does not clear surface+28. The concrete callback
vtable `5c6484` resolves invocation to `4bc790` and disposal to `4bc980`.
Invocation forwards one argument and the stored receiver to the stored function;
the class11 function `4a8550` returns zero. Disposal calls `4bcc00`, which resets
the callback vtable to `5c5c40`, then frees the callback through `597870` exactly
when the low bit of the disposal flags is set, and returns the original callback
address. Its original body and the base destructor run unchanged in the fixture.
The release caller's argument1 therefore requests freeing the callback. Its
unchanged surface+28 alias remains a separate later-use question; a dangling
stored value alone does not prove that original control dereferences it.
The six tick transition/draw/final callbacks are complete
no-ops for this concrete class. `4a8550` returns zero and consumes one argument.
Those slots perform no screen destruction or ownership transfer.

## Validation and remaining boundary

The original-body fixture compares complete owned storage and complete storage
snapshots at every allocation/free/disposal callback against an independent
portable model. It covers patterned untouched bytes, empty/nonempty vectors,
existing/absent callbacks, allocation failure, repeated prepare/release,
separate destruction, disposal-flag bits and allocation/free/disposal mutations.
Both constructor and vector retirement run unchanged in the original mapping.
The fixture tests authored disposal callbacks as well as the real unchanged
invocation, deleting destructor and base destructor. Allocator/free internals
remain authored endpoints; no actual original heap manager is exercised.

The retained inventory starts this constructor at4a8431, omitting its initial
`push ebx` at4a8430. The complete unchanged caller comparison includes that byte;
an explicit one-byte recovered-range exception records the correction without
rewriting the historical inventory or its source hashes.

Original-manifest verification brackets original-consuming experiments; original
files and compared function bodies remain unchanged. Confidence is high for
these local semantics within valid owned storage and synchronous endpoints.
Real allocator reachability, fault unwinding,
asynchronous retirement, other classes' full resource orchestration and installed
live semantic cursor takeover remain separate recovery/integration work.

The final [lifetime comparison](screen-services-results-20261025/lifetime05.json)
passes4,608 cases in optimized i386,i386 ASan/UBSan and native64 ASan/UBSan,with
identical complete-state/endpoint checksums. Its22 tables retain47 direct-store
witnesses and132 named shared-tick slots. The fresh [service comparison](screen-services-results-20261025/current01.json)
passes192 selection,9,984 cursor,18,432 control and110 tick-transition cases in
each build on the current master-based source tree.

The first lifetime attempt retains an optimized pass followed by an i386 ASan
stack-alignment failure at the authored allocator boundary. Explicit
force_align_arg_pointer on the three Linux adapters fixes that fixture ABI;
no original instructions change. lifetime02 retains the narrower2,016-case
pass;lifetime03 adds real callback invocation/deletion and4,608 cases;the final
lifetime04 additionally pins and verifies all exercised no-op endpoint bytes.
No earlier result,source fingerprint or failed attempt is overwritten.

The final lifetime05 repeats the unchanged4,608-case matrix after binding explicit
original-reference/range IDs for previously uninventoried callback bodies. Its
source bytes match lifetime04;its prospective contract includes those accounting
links. Earlier failed audit snapshots remain immutable.
