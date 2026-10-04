# NoCD sprite visibility reconstruction

Offline selected contract, pinned to executable SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
The original PE is mapped privately without instruction changes; no Win32,
allocator, draw backend, Wine session or live game is called.

## Mask helper and grid

`0x005013c0` is thiscall with frame pointer, X/Y draw anchors and draw kind
(`ret 0x10`). Kind 8/9 bypasses the pass; absent queue grid, kind 1, or an
absent/out-of-frame test-plane offset returns false. Supported frame planes
are [documented separately](../formats/spr-visibility-planes.md).

The origin is frame origin plus unsigned header bytes 2/3; subtract it from
draw anchor with 32-bit wrap. The DWORD viewport flag at `0x006de6d5` selects
W=640,H=512 when zero, or W=800,H=632 when nonzero. Anchors outside x=[-32,W+32), y=[-32,H) return
false without reading/updating the grid.

For bounded anchors x,y, signed division truncates toward zero:

```
column = (2*y + x)/8 + 24
row    = (2*y - x + W + 7)/8 + 24
byte   = column/8
shift  = column%8
```

Each test DWORD is logically shifted right by shift, then byte-swapped.
Compare `(gridDWORD & transformedTest) == transformedTest` for every row;
the first mismatch ends testing, but still permits coverage updates. All
matched rows returns true, including a valid zero-row plane. Only kinds 0/33
OR transformed coverage DWORD rows into the grid, irrespective of the test
result. Other kinds can be hidden but do not contribute coverage.

Grid initialization `0x00501350` sets 344 bits by 339 rows, byte stride 43,
14,577 bytes. The clear routine `0x00501330` zeros this allocation. The native
model owns bytes and checks complete four-byte row accesses; it rejects
truncated rows, unequal test/cover extents, rows >255 and out-of-grid footprints
before accessing memory. These safety limits do not assert safe original
handling for malformed assets. Scratch grid dimensions are shared between
viewport modes; expanded mode changes bounds/projection, not allocation.

## Reverse pass and owner flags

`0x005015f0..0x005017d9` visits sorted entries from last down to index 1.
**Index zero is never visited**; zero/one-entry queues do nothing. Fully covered
entries replace DWORD +24 draw kind with -2. Record +20 is an optional terrain
owner pointer, not a generic creature owner.

For a linked owner, selected sprite role identity chooses the following flags:

| Resolved role | Hidden | Visible |
| --- | --- | --- |
| Body | WORD owner +10 OR 0x0040 | AND 0xffbf |
| First additional role | WORD owner +8 OR 0x0004 | AND 0xfffb |
| Second additional role | WORD owner +8 OR 0x8000 | AND 0x7fff |

The original follows owner WORD +0 into a 356-byte definition table at global
`0x0065660c`, adjusted by orientation/global `0x00689961`, then reads frame
indices at definition +0x84/+0xd0/+0xe0. SPR collection global `0x0068995d`
resolves roles, falling back to index zero when a requested index exceeds the
header bound. Helper `0x005017e0` supplies the body lookup. The model accepts
already resolved numeric identities and owned flag words, retaining the
body/first/second priority when identities alias. Caller-side world table
ownership/orientation/role resolution remains separate from native integration.

The main caller at `0x004fd3d1..0x004fd428` gates execution with byte
`0x005e140c` and fields at world object +0x66ca/+0x66ce. It clears the grid
before calling the reverse pass. Those activation/lifecycle producers are
static evidence; the Qt preview uses explicit admission.

## Offline evidence

`python3 tools/test-sprite-visibility.py` verifies immutable input manifests
before/after, executable hash and entry bytes, compiles a 32-bit helper and
executes unchanged routines on owned fixtures. [Report](sprite-visibility.json)
records 8,192 direct helper matches and 1,024 complete reverse-pass matches,
including every coverage byte, all nine DWORDs in each queue record, optional
owner links and complete flag words. Fixtures cover both viewport modes,
clipping/boundary anchors, wrapped origins, random/full/zero masks and grid
seeds, queue lengths 0..16, zero rows, bypass kinds and covering kinds 0/33.

Artifact directory: `working/tests/sprite-visibility/run-vg10fd5k/` contains
assembly, compiler log, reference output, immutable-input logs and source/input/
helper hashes. Installed frame fixtures use this same helper's bounded queue
input mode, documented in [native integration](native-sprite-visibility-scene.md).

Confidence is high for the selected helper/reverse-pass transformations and
bounded fixture behavior. Complete mask generation, terrain producers and
orientation mapping, runtime activation and original whole-scene drawing/live
equivalence remain unverified. The model is not injected into the game.
