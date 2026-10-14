# Original Grimoire text helper and page flow

Build: No-CD40209ca7. Confidence: confirmed static field/branch discovery; full
page behavior and font/render equivalence remain provisional and unexecuted.
[UI72](grimoire-text-static-20261013.json) retains the source export and raw
`working/decompiled/grimoire-controls-dzgwwcna` artifacts.

Text helper `004c7960` resets its2048-byte output buffer and output/read pointers
at+0x800/+0x804,clears raw input+0x808,current horizontal position+0x80c,nested
comment depth+0x810,marker pending+0x814,flow flags+0x81c/+0x820/+0x824,
link flag/identity+0x828/+0x82c/+0x830,region list+0x834 and graphic name+0x854.
It releases a prior dynamic descriptor at+0x95c through004ca2e0 and the original
allocator. Resource ownership and complete initialization are not inferred.

Tokenizer004c9bd0 skips raw CR/LF and appends ordinary printable bytes/tabs.
A tilde starts a command outside comments. Commands are case-insensitive for
D,G,I,N,C,L,S; nested bracket comments use+0x810. `~NL` appends a newline and
sets a boundary;`~NB` sets a new-block boundary;`~NP` sets a new-page boundary
at+0x824. Graphic names and packed chapter/section links have separate state.
D creates a bar descriptor; see [dynamic fields](grimoire-dynamic-fields.md).
These are static mappings,not a complete safe markup parser or malformed-input
contract. The legacy output/name buffers have unchecked boundary cases.

Consumer004c7ab0 advances the original font/resource-defined text-region list,
places text/graphics/bars/links and consumes pending commands. New-page+0x824
clears region-active+0x848. 004c9fd0 repeatedly consumes/parse-fills while a
region remains available and reports remaining content. 004ca0c0 handles
illustration/resource region work. Existing Next004c6910 increments the leaf
cursor by2 within an article;Previous004c6db0 decrements by2 or scans a previous
article to its final spread. `~NP` source chunks therefore cannot simply be
identified with the native reader's single text pane or scrolling policy.

Native dynamic capture and two-leaf spread integration remain pending. The
existing experimental V22 reader still excludes dynamic/multipage articles.
No research purchase or knowledge mutation is discovered by these text helpers.
