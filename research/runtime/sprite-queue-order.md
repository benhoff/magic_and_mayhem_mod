# NoCD sprite queue depth and ordering

Offline evidence for executable SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
No Wine, live hooks, executable patches or game launches were used.

## Confirmed contract

Builder `0x004ffd30` takes a queue object in ECX and fourteen stack arguments
(`ret 0x38`). Queue fields are record base +0, next write +4, count +8,
capacity +12 and view +20. Records are 36 bytes. First DWORD is the signed
sort key; +4 holds an opaque sprite pointer, +8/+12 draw coordinates. Sorting
swaps all nine DWORDs. Draw coordinates, SPR origins and frame numbers are not
tie breakers. Record +32 is untouched by the builder and still moves with its
record during sorting.

Arguments 2/3/4 provide x/y/height; argument 7 adds priority after rotation.
For height 0..499, table `0x006e818c` contributes floor(82*height/100).
Initialization at `0x004fd063..0x004fd091` establishes that table; the fixture
initializes it from this statically recovered formula. View contributions are:

| Raw view | Coordinate contribution, height <500 |
| --- | --- |
| 0 | x+y |
| 1 | -x+y |
| 2 | -x-y |
| 3 | x-y |

At height >=500, **every view uses x+y**. The height multiplication wraps to
32 bits, is interpreted as signed and divided by 100 with truncation toward
zero. Coordinate and priority additions also wrap. The native implementation
preserves these quirks using unsigned arithmetic and bit-preserving conversion.
Negative heights index before the table; views outside 0..3 leave the key
uninitialized. These unsafe cases are rejected by the native model.

Sorter `0x004fff60..0x00500232` is an iterative quicksort comparing signed keys
in ascending order. It swaps the middle record into the first position, scans
left while key <= pivot and right while key >= pivot, swaps crossing candidates,
then places the pivot. Equal keys have **unstable, reproducible permutations**;
four identical keys with payloads 0,1,2,3 become 1,0,2,3. Native reconstruction
uses owned entries and a bounded dynamic range stack, preserving permutations
without carrying original pointers or uninitialized fields into native code.

Draw consumer `0x005002a0` starts at record base and advances by 36 bytes
(`0x00500d32..0x00500d47`, alternate path `0x00501126..0x0050113b`), establishing
ascending queue traversal. This consumer is statically traced, not executed.
The optional preceding pass `0x005015f0` traverses backwards and changes draw
kinds/visibility; its world-dependent occlusion behavior is outside this model.
Creature body/selected mode-one attachment submissions at `0x004fa5b5` and
`0x004fab3c` use distinct priority contributions (body +6, attachment +8),
but complete per-entity base priority/admission production remains outside scope.

## Validation and confidence

`python3 tools/test-sprite-queue.py` verifies immutable input manifests before
and after, verifies the pinned executable hash, maps the unchanged PE privately
and executes only the builder and sorter on owned synthetic queues. All Win32
and original draw paths remain uncalled. The mapping's scratch data is initialized;
executable instructions and files remain unchanged.

[Machine-readable comparison](sprite-queue-order.json) records 4,096 original
builder/native key matches across all four views, heights around the 499/500
boundary and extreme 32-bit inputs. It also records 1,285 sorting comparisons,
164,480 complete 36-byte records, lengths 0..256, ascending/descending/random
inputs and dense/all-equal ties. Twenty-four additional original-built/sorted
12-record fixtures supply scene orders for
[native overlap validation](native-sprite-queue-scene.md).

Evidence artifacts are under `working/tests/sprite-queue/run-vp9wxxa2/`:
unchanged-input logs, compiler log, original assembly ranges, reference output
and report with source/input/helper hashes. Confidence is high for the selected
key and permutation contracts, with table initialization and traversal supported
by static evidence. This does not establish original full-world rendering,
visibility/lighting, terrain ordering, producer coverage or live integration.
