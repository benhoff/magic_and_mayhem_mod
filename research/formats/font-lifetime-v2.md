# Font lifecycle journal version 2

The separate `font-lifetime-v2.bin` uses `MNMFLT02` and header version2.
All v1 entry, return, load, resource and terminal fields remain as documented
in [version1](font-lifetime-v1.md). Historical v1 raw bytes are unchanged.

Glyph word23=2 means exactly one current live raw font allocation contains the
entire observed frame. Words10/13 are zero: no byte-draw parent is invented.
Word23=0 retains the original byte-parent binding;1 remains unbound and earns
no resource-use credit. Ambiguous live raw extents permanently mark loss.

For the first glyph in each resource-generation/frame-offset pair, word24=1
and the record body contains the complete original frame, with only its palette
pointer bytes28..31 normalized to zero. Extent is128+framebytes. Subsequent uses
have word24=0 and require a prior complete snapshot for that exact pair.
There are at most4096 distinct source pairs. The16MiB total/65536record bounds
remain unchanged. Old v1 readers refuse this version.

Readers compare the retained complete first-source bytes against independent
installed SFT bytes. Later per-use FNV values remain diagnostic checks and do
not establish byte equality under arbitrary mutations or hash collisions.
Source custody alone does not establish independently checked presented pixels,
DC selection/restore, hidden overwritten-allocation frees or full ABI parity.
