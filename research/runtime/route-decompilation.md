# No-CD route routines: manual reconstruction

2026-10-02. This is a **partial manual decompilation from objdump disassembly**,
not Ghidra output, original source, or a buildable algorithm replacement.
Ghidra was absent; sandbox DNS prevented downloading it. No game binary was
changed or executed. Build SHA-256:
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
All addresses assume preferred image base `0x00400000`.

## Successful automated run

Follow-up on 2026-10-02: Ghidra 12.1.4 was found through the user's
`~/.local/bin/ghidra` symlink. A workspace-local ARM64 decompiler was built
from the release's bundled source. Headless analysis completed in 88 seconds;
the Java exporter compiled and exported all seven selected functions with
zero failures. Manifest status is `decompiled`, exit status is zero, and the
input executable SHA-256 was unchanged.

Evidence directory: `working/decompiled/nocd-nn0_667n/`.

- `functions.tsv`: entry addresses and export status.
- `c/00512800.c`: route request.
- `c/0054b800.c`: search.
- `c/004ebae0.c`: neighbor expansion.
- `c/004ec780.c`: heuristic.
- Remaining C files: coordinate helpers.
- `project/MagicMayhem.gpr`: analyzed project for further work in Ghidra.
- `manifest.json` and `ghidra.log`: reproducibility and analysis evidence.

This is the selected-routine export, not a whole-executable C export. Ghidra
analyzed the executable globally; `--all` is needed to export every discovered
function. Raw inferred helper prototypes and some call arguments still need
correction: the wrapper output includes `extraout_ECX` placeholders. Treat
the C as analysis output, not a hook ABI specification or buildable code.
Analysis warned about unavailable bytes in unrelated function `0x0046c070`;
none of the seven selected exports failed. No live hook was installed or tested.

Names below are analyst-assigned. Static control flow is stronger evidence
than inferred gameplay meaning. Live confirmation remains necessary.

## Route request: 0x00512800

Follow-up alias finding: `0x00690354` equals route context base `0x00690148`
plus `0x20c`. The wrapper's pre-search store is the context initialization byte,
not an independent global. The readable model preserves this alias; search can
change it. See `reconstruction/pathfinding/README.md` for the tested milestone.

The entire wrapper (`0x00512800..0x00512894`) was inspected, along with
its coordinate constructors. ECX supplies the object pointer; four DWORD
stack arguments are removed by `ret 0x10`. Inferred signature:

```c
// Inferred thiscall-like ABI, not a verified C declaration.
bool route_request(Object *self /* ECX */,
                   int x, int y, int z, uint32_t arg4_unknown)
{
    int remaining = load32(0x005e174c); // MaxNodesForRouteFinding
    store8(0x00690354, 1);              // unknown purpose; not reset here
    int wrapped_y = wrap(y, load32(0x006c5498)); // helper 0x0040e8a0
    int wrapped_x = wrap(x, load32(0x006c5494)); // helper 0x0040e290
    // helper 0x0040eeb0 copies z unchanged
    route_search((void *)0x00690148 /* ECX */, self,
                 arg4_unknown, wrapped_x, wrapped_y, z, &remaining);
    memcpy((uint8_t *)self + 0x96b, (void *)0x00690148, 0x20c);
    uint32_t count = load32((uint8_t *)self + 0x97b);
    store32((uint8_t *)self + 0xb8b, count != 0);
    store32((uint8_t *)self + 0xd03, 0);
    return count != 0;
}
```

`wrap` represents repeated addition/subtraction until the coordinate lies in
`[0, dimension)` (valid positive dimensions assumed). This is observed code,
not a proposal to change map wrapping. X/Y/Z names are inferred from shared
coordinate lookup tables, not recovered symbols.

Confidence: high for copy length, fields, argument order, helper behavior and
return test; medium for coordinate labels and object identity. Argument four
has not been identified; do not call it flags without additional evidence.

## Search: 0x0054b800

The whole function through `ret 0x18` at `0x0054be45` was inspected. ECX is
the output/scratch context, with six DWORD-sized stack arguments:
object, unknown argument, wrapped x, wrapped y, z, budget pointer.
It copies the object's previous `0x20c` bytes into the context before searching.
Context fields beyond `+0x20c` include persistent container/search state not
covered by that copy.

The following is a **semantic sketch**, omitting container implementations,
initialization details and waypoint attribute calculations. Unknown helper
names do not imply verified data structures:

```c
void route_search(Context *ctx /* ECX */, Object *self, uint32_t unknown,
                  int x, int y, int z, int *remaining)
{
    memcpy(ctx, (uint8_t *)self + 0x96b, 0x20c);
    // Cell records have stride 12. Offset tables supply y/z contributions.
    Node *target = cell_base + 12 * (x + y_offsets[y] + z_offsets[z]);
    Node *start = cell_from_fields(self, 0x08, 0x0c, 0x10);

    if (ctx->byte_20c != 0) {
        // Clear/reinitialize queue and node records through container helpers.
        ctx->byte_20c = 0;
        initialize_containers_omitted(ctx, start);
        ctx->best_h_211 = heuristic_004ec780(start, target);
        ctx->best_node_215 = start;
    }

    while (ctx->field_22d != 0) {
        Entry *entry = peek_0043ff50(ctx + 0x221);
        Record *target_record = lookup_00440080(ctx + 0x231, target);
        if (entry->field_0c >= target_record->field_00)
            break; // signed comparison, 0x0054b9f8

        int priority = entry->field_0c;
        Node *current = entry->field_10;
        reset_neighbor_buffer_omitted(ctx + 0x241);
        expand_004ebae0(current, ctx + 0x241, &current,
                       lookup_00440080(ctx + 0x231, current) + 8, remaining);
        if (*remaining <= 0)
            goto build_result; // 0x0054ba77, BEFORE removing queue entry

        remove_front_omitted(ctx + 0x221);
        int current_h = heuristic_004ec780(current, target);
        for (Candidate *c : neighbor_records_of_stride_36(ctx + 0x241)) {
            int neighbor_h = heuristic_004ec780(c->node, target);
            int tentative = priority - current_h + c->field_00 + neighbor_h;
            Record *r = lookup_00440080(ctx + 0x231, c->node);
            if (tentative < r->field_00) { // signed comparison
                r->field_00 = tentative;
                r->field_04 = current; // predecessor
                memcpy(r + 8, c + 8, 28); // byte offsets, not typed C arithmetic
                insert_queue_entry_omitted(ctx + 0x221, tentative, c->node);
                if (neighbor_h < ctx->best_h_211) {
                    ctx->best_h_211 = neighbor_h;
                    ctx->best_node_215 = c->node;
                }
            }
        }
    }
    ctx->byte_20c = 1; // exhaustion/termination; budget exit bypasses this

build_result:
    // Follow records' +4 predecessor links from best_node_215 to start.
    // Insert copied 28-byte payloads into a path container at ctx+0x251.
    reconstruct_predecessors_omitted(ctx, start);
    ctx->field_261 = path_iterator_omitted(ctx + 0x251);
    ctx->field_265 = lookup_00440080(ctx + 0x231, ctx->best_node_215)->field_00;
    ctx->field_00 = x;
    ctx->field_04 = y;
    ctx->field_08 = z;
    ctx->field_10 = 0;
    while (has_path_entry_omitted(ctx) && ctx->field_10 < 16) {
        // Decode cell coordinates, movement values and category flag.
        emit_waypoint_omitted(ctx + 0x14 + 28 * ctx->field_10, self);
        ++ctx->field_10;
    }
    ctx->field_0c = 0;
}
```

Interpretation: the priority update has the A*-style form
`f(next) = f(current) - h(current) + edge_cost + h(next)`. Tracking the
lowest heuristic node and reconstructing from it supports partial-path
fallback. Confidence: medium-high for this interpretation, high for the
arithmetic, branch and predecessor-copy observations. The heuristic's full
meaning, admissibility, queue behavior and correctness are not established.

Notable evidence:

- `0x0054bac7..0x0054bb15`: candidate scoring, better-score check,
  predecessor store and seven-DWORD payload copy.
- `0x0054bb48..0x0054bb5e`: update closest-to-target candidate.
- `0x0054bbb8..0x0054bc1b`: predecessor traversal.
- `0x0054bd5b`: **16-entry output limit**; each output entry advances by
  `0x1c` bytes at `0x0054be22`. Increasing node budget does not increase this
  output limit automatically.
- `0x0054be35`: resets context `+0x0c` to zero.

## Neighbor expansion: 0x004ebae0

Partial interpretation only. Its entry checks the pointed-to budget and its
end decrements that budget once (`0x004ec76a`). A loop ends after index 26
(`0x004ec755..0x004ec75d`), consistent with a 3D adjacent-cell neighborhood,
but valid neighbors, movement rules and collision checks have not been fully
decoded. Neighbor records have a 36-byte stride. Confidence: high for budget
and stride; medium for the neighborhood interpretation.

## Automated export

Run from any directory, with no arguments:

```bash
./tools/decompile-game.py
```

It verifies the no-CD hash, creates a fresh directory under
`working/decompiled/`, saves assembly for the seven selected routines, then
uses Ghidra if found. With no Ghidra, it exits with an explicit error; assembly
output must not be mistaken for automatic decompilation. It does not overwrite
previous runs, modify the input, download dependencies, or upload game data.
`--all` exports all functions discovered by Ghidra; `--disassembly-only` needs
only objdump. Projects/logs and inferred C are kept together for inspection.

Ghidra discovery: `--ghidra DIRECTORY`, `GHIDRA_HOME`, `analyzeHeadless` on
PATH, a `ghidra`/`ghidraRun` launcher symlink on PATH, or an extracted
`ghidra*` directory in `working/toolchain/`, Downloads, or `/opt`.
Workspace-local native builds are reused. Get an official release from
[NSA's Ghidra releases](https://github.com/NationalSecurityAgency/ghidra/releases).
This machine is ARM64. If a compatible native decompiler is missing, the
exporter copies the installation into a fresh workspace-local directory and
builds the bundled C++ decompiler using `make -j2 ARCH_TYPE= ghidra_opt`, then
installs it as `os/linux_arm_64/decompile` in that copy. No external download
or changes to the user's Ghidra installation are needed. This consumes roughly
1 GB plus compiler output on the first run. It requires `make` and `g++`.
The empty `ARCH_TYPE` overrides the bundled Makefile's unsupported ARM64
fallback to `-m32`. For general setup, see Ghidra's
[native-component build instructions](https://github.com/NationalSecurityAgency/ghidra/blob/master/GhidraDocs/GettingStarted.md#building-native-components).
The follow-up run found Java 25 and `javac`; the initial inspection did not
find `javac`. Ghidra settings and caches are kept inside the run directory.

The exporter uses Ghidra's documented
[DecompInterface](https://ghidra.re/ghidra_docs/api/ghidra/app/decompiler/DecompInterface.html).
Disassembly export, executable-hash rejection, launcher symlink discovery and
isolated native-build behavior have automated tests independent of Ghidra.
