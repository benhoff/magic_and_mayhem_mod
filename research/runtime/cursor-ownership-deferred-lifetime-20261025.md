# Cursor clients and deferred resource lifetime

Build: NoCD `40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Confidence: high within the serial, valid-memory, scripted-endpoint domain below.
This work complements the screen-services worktree's `4a4370` invalidation and
`4a4380` background restore. Neither helper is duplicated here.

## Recovered contracts

`reconstruction/rendering/cursor_clients.hpp` implements foreground `4a45b0`,
timer `4a4910`, and disposal caller `4a37d0` against value-address memory and
explicit service boundaries. Addresses are build bindings, not stable host
pointers or portable native resource IDs. Descriptors at `642678 + type*68`
contain hotspot x/y, frame count, thirteen frame indices and countdown reload.
Frame indices use signed division/remainder by `641a8c`, multiplied by 40.
The model reports the original IDIV fault as an exception; malformed descriptors,
invalid pointers, OS failure output bytes and arbitrary concurrency are excluded.

Foreground gates on `642c64`, active screen and class other than 22, obtains and
transforms a cursor point, subtracts the hotspot, and waits 10 ms on `642c58`.
Every nonzero wait result exits without release. Intersection/save-under and
animation updates precede release. Sprite copy and published dirty rectangle
follow release. Deferred mode enqueues save-under type2 and keyed sprite type0
at priority1. A full queue silently skips publication of each copy; caller state
still advances. Saving copies source receiver `6a49e0` into destination `641fc8`;
painting copies `642000` into `6a49e0`.

Timer admits types0..21, waits10 on `6e1ec0`, decrements a positive countdown on
an active screen and gates on `642c60`. Unchanged position can skip repaint for
a single frame or a positive countdown. Repaint waits INFINITE on `642c58` and
ignores the raw result. It restores saved background from `641fc8` to `6a4fe0`,
saves from `6a4fe0` to `641fc8`, releases the inner mutex, paints from `642000` to
`6a4fe0`, then advances animation. These are immediate copies even when deferred
mode is set. The outer mutex is released last. Neither painter establishes
allocation alias ownership or real mutex ownership for abandoned/failed waits.

`deferred_surface_copy.hpp` implements `58d5b0` enqueue and `58d660` drain.
There are128 records of44 bytes at `6f6b50`: occupied, priority, source wrapper,
destination wrapper, kind, four rectangle values, x and y. Enqueue owns rectangle
values but borrows both wrapper addresses. A null rectangle retains old slot
words. There is no AddRef, byte snapshot, mutex or ownership transfer at enqueue.
Drain resolves the destination wrapper's current interface for GetBltStatus(1),
accepts0 or8876023f, sets `6f98d8` during dispatch, and visits priorities0..2 in
slot order. Busy records and unsupported priorities/kinds remain pending. Copy
results are ignored; supported dispatched records retire. Wrapper/interface
rebinding between enqueue and drain changes the resources actually used.
The model refuses an inconsistent count before reading outside128 slots; that
refusal is a native bound, not a recovered corruption-handling branch.

Disposal closes the foreground mutex if nonzero, ignores CloseHandle's result,
clears the global and calls `58b3e0` for sprite `642000` then saved background
`641fc8`. It neither drains nor cancels deferred records. The fixture executes
the original disposer and original surface release body with synthetic COM
objects: queued records/count survive while both wrapper interfaces become zero.
It deliberately does not dereference those dead destinations in a later drain.
Thus deferred consumers require valid wrappers/interfaces until retirement;
the diagnostic gate's synchronization cannot supply that lifetime guarantee.

## Validation and limits

`tools/test-cursor-ownership.py` compares complete selected state, every queue
word and ordered OS/COM endpoint arguments including callback-visible state.
The fixture privately maps the pinned original image. Selected original client,
queue, copy and surface disposal instructions are unchanged. OS calls and COM
endpoints are scripted; unchanged original copy/disposal bodies are shared
external dependencies of the comparison, not independent native pixel evidence.
The test domains include both resolutions, clipping inside/outside/negative and
wrapped coordinates, hotspot transforms, frame wrap, negative frame arithmetic,
countdowns, ignored API results, all four raw wait outcomes, synchronous wait and
release mutations, queue saturation, retained null-rectangle words, supported and
unknown kinds/priorities, source/destination rebinding, sparse holes, copied
rectangle storage destruction, busy retry, and disposal with0/1/128 pending copies.
Both optimized and ASan/UBSan32 execution pass10,368 client cases,1,280 queue
cases,12 disposal cases and6 sparse/retry cases. They retain prospective scope/source declarations,
exact compiler dependencies, original function hashes and manifest bookends in
[SQLite execution evidence](cursor-ownership-results-20261025/execution02.json).

This is offline recovered control and lifetime-precondition validation. It does
not install a native painter/owning queue, prove every resource allocator/writer,
recover original setup or surface-release internals, establish safe concurrent
shutdown, compare actual pixels/physical drivers or claim live replacement.
The screen-services background helpers retain their own evidence and status.
`runtime/drawing/source_cursor_gate.h` remains correctly marked diagnostic.

The first prospective run retained optimized agreement but the sanitizer stopped
on a test-harness reference to the original unaligned resolution DWORD. That
failed report remains immutable as execution01; byte-copy host access replaces
the invalid host reference before the second prospective run. This is a harness
correction, not a change to original cursor arithmetic or ownership semantics.
