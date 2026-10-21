# Alternate canvas allocation, initialization and writer recovery

The alternate path remains an open recovery obligation. This investigation adds
one original function, a complete original initializer experiment, and an
independent model of the newly found copy. It does not invent a buffer allocator
or claim that an original session can reach the alternate route.

All addresses below refer only to the immutable No-CD executable, SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Original-consuming runs verify the manifest before and after. Confidence is high
within the stated static/owned execution boundaries. The REC8 reports remain
unchanged.

## Global identities and initialization

| Address | Confirmed role | Evidence and boundary |
| --- | --- | --- |
| `006e1f68` | Alternate rendering base pointer, whose nullness also selects the dispatcher branch. | `004fd438` loads ECX, which reaches the unchanged fastcall canvas setter `0057dc20` at `004fd47b`. It is not merely a Boolean flag. Five literal operands are reads. |
| `006e1f6c` | Source pointer for copies back to the locked ordinary canvas. | Readers at `00594514` and newly found `00594568`; the relationship between this pointer and the rendering base is unresolved. |
| `006e1f70/74` | Horizontal/vertical padding. | `004e4163/6d` overwrite them with `96/0` when the separate configuration flag is nonzero. |
| `006e1f78` | `DEBUG/UseMallocedBackBuffer` configuration flag. | Configuration writer at `004e4c4e`; initialization reader at `004e415a`. The name does not establish that alternate pixels are allocated. |

All five DWORDs lie in the PE `.data` virtual tail beyond its stored bytes and
start with loader-supplied zeros. This is initialization of the globals, not an
allocation or clear of a pixel buffer. The retained exhaustive raw literal scan
finds five rendering-base operands, **two** copy-source operands, eighteen
horizontal-padding operands, six vertical-padding operands and two configuration
operands. Each is attributed to byte-checked decoded instructions; all original
literal sites remain visible. The previous recursive frontier omitted the second
copy, so its absence was not evidence of absence of source readers.

The four sprite/shading readers at `0057df56`, `0057e2ff`, `005818a2` and
`00581c32` gate horizontal clipped slow paths; they do not publish either pointer.
They are not an inventory of every raster or upstream writer.

## Complete initializer experiment

`RS.alternate-canvas-initializer` executes the complete unchanged
`004e40d0..004e4263`, the original color-fill wrapper `0058bc10` and format wrapper
`0058d540`. Mode setup and surface creation are explicit owned callbacks;
successful Blt/GetPixelFormat calls are owned COM boundaries. No physical driver
allocation or clear pixels are claimed.

The 288 cases cover both video dimensions and mode choices, three zero/nonzero
configuration values, three existing horizontal and two vertical paddings, and
zero/nonzero rendering-base and copy-source values. They compare both complete
108-byte descriptors and the entire call sequence:

1. Mode setup, or the original window-mode branch's owned setup boundary.
2. Primary wrapper request at `006a4fe0`, flags `1`, caps `0x200`.
3. Offscreen wrapper request at `006a49e0`, flags `7`, caps `0x840`, dimensions
   `(640 or 800) + xPadding` by `(480 or 600) + yPadding`.
4. Original offscreen zero color-fill request: flags `0x01000400`, null source and
   rectangles, complete 100-byte FX with zero fill color.
5. Original primary pixel-format query and final return `1`.

Nonzero `006e1f78` first replaces padding with `96/0`. Both alternate pointers
retain their input values, and the entire authored alternate-storage canary
remains unchanged. The original initializer does not request an alternate
allocation through these boundaries. Its offscreen dimensions add padding once;
the later alternate binding adds it twice. This experiment does not extend to
arbitrary external callback effects, driver failures or whole-session startup.

## Newly discovered x87 copy

An exhaustive raw search for the source pointer found a second operand at
`0059456a`. Focused read-only Ghidra export and disassembly establish a separate
144-byte body, `0059455c..005945ec`, SHA-256
`5fa7b04519567f0cbefd9ec6550daaf01fa00e667914cb288334da3d56cc5585`.
It is disjoint from all prior functions and ends immediately before `005945ec`.
The new inventory retains every prior function, import, flow and unresolved edge.
Only this boundary is added; unrelated decompiler range changes are rejected.

The helper copies 32 WORDs per inner iteration. The high 32 bytes pass through
four integer DWORD pairs, interleaved with x87 loads of the low 32 bytes in
reverse QWORD order. The four x87 stores then write the low half in forward
order. Destination rows are contiguous. Source rows advance by the copied width
plus four times horizontal padding in bytes. Like `00594508`, the helper requires
positive height and a positive exact chunk multiple; zero or malformed original
counts are unsafe and are not executed.

It is **not a bit-preserving copy for every pixel pattern**. With masked x87
exceptions, a low-half signaling binary64 NaN is quieted, changing a pixel bit
and setting invalid-operation status. Denormal operands set the denormal status
flag. High-half integer copies do not perform that conversion. Overlap can alter
which values later loads see; a whole-chunk snapshot or `memmove` is incorrect.

`RS.alternate-canvas-x87-copy` compares 10,584 cases against the unchanged original
body: widths 32/64/96, heights 1/2/5, seven signed padding values, twelve physical
overlap/separation offsets and fourteen binary64/mixed patterns. Every case
compares all 2,048 physical WORDs and the six exception bits, and verifies the
original control word and stack TOP. The owned native bit model does not consume
original pixels. Eight invalid native requests, including extreme dimensions and
row steps, refuse before mutation. The admitted original FPU state has control
word `0x037f` and an empty stack; unmasked exceptions, arbitrary stack occupancy
and asynchronous FPU use remain outside the comparison.

## Remaining allocation and writer boundary

The known conditional writer chain is:

`006e1f68 -> 0057dc20 -> active canvas global 00658174 -> World rasters`

followed by a normal-surface Lock/rebind and `00594508` from `006e1f6c`. The new
`0059455c` also reads `006e1f6c`, but no original incoming caller is established.
Neither pointer's original allocator, publication, relationship, initial pixels
or release has been identified. A width/height formula cannot supply that
ownership contract, and an owned initializer callback is not a driver allocator.

Nearby address constructions and the previous **2,948 unresolved indirect
edges** remain retained search frontiers. No direct-store or literal-address
argument excludes register-relative, bulk, imported or external writes. Current
understanding therefore supports neither reachability nor unreachability of the
actual alternate route. Next original-session observation must identify a write
or nonzero publication of the two globals, associate backing generations, and
trace clear/writer/retirement boundaries before any native allocation policy or
live replacement can be admitted. The broad `RS.canvas-bind-clip` remains partial.

## Reproduction and evidence

Run from the repository root, using a new output directory:

```sh
python3 tools/working-retention.py run --reserve-gib 0.2 -- \
  python3 tools/test-original-alternate-canvas.py \
  --output working/tests/alternate-canvas-next
```

The runner prospectively declares behavior identities and complete repository
compiler inputs, pins the unchanged executable, checks full records, and retains
raw compressed native/original output, binaries, logs and negative outcomes.
Durable reports and decompilation are under
[alternate-canvas-results-20261020](alternate-canvas-results-20261020/).
The original discovery frontier must be local; use the retention restore workflow
before reading an archived input. No game installation or Wine prefix is created.
