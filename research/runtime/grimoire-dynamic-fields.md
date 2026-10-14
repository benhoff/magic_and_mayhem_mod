# Original Grimoire dynamic bar descriptors

`004c8950` constructs a `0x38`-byte bar descriptor for `~D`; it does not emit a
formatted number paragraph. Original constructor `004dd970` supplies minimum0,
maximum400 and value/mirror200. The parser copies the text helper's font identity
from `+0x850`, allocates a byte label at descriptor `+0x2c`, and fills width/height
at+0x10/+0x14,minimum/maximum at+0x1c/+0x20,value/mirror at+0x24/+0x28 and
layout offsets at+0x30/+0x34. The consumer `004c7ab0` places/draws the label and
bar, then destroys it; full consumer drawing is static discovery only.

[Source grammar](../formats/grimoire-dynamic-fields.md) records tokens and cursor
behavior. Geometry uses8/10 integer scaling in low resolution; the original
`006e2058/006e205c` adjustments are added after scaling. Nonzero resolution mode
uses unscaled geometry. Six source colour values are parsed and discarded.

| Property | Value source | Maximum source |
| --- | --- | --- |
| HTH | thiscall00505c50(creature template,opponent) | max(006e981c,006e9820) |
| LRC | thiscall00505bf0(creature template,opponent) | max(006e981c,006e9820) |
| HEALTH | 006a5f98+type*0x5c9 | 006e9824 |
| MAGIC_RESIST | 006a5fa8+type*0x5c9 | 006e9828 |
| INTELLIGENCE | 006a5f94+type*0x5c9 | 006e982c |
| WEIGHT | 006a6531+type*0x5c9 | 006e9830 |
| VIEW_RANGE | 006a5fb8+type*0x5c9 | max(006e9834,006e9838) |
| FIRE_RANGE | 006a5fb4+type*0x5c9 | max(006e9834,006e9838,006f56a8) |
| GROUND_SPEED | 006a653d+type*0x5c9 | max(006e983c,006e9840) |
| FLYING_SPEED | 006a6541+type*0x5c9 | max(006e983c,006e9840) |
| BRAVERY | 006a6545+type*0x5c9 | 006e9844 |
| SPELL_RANGE | 006b1268+spell*0x2d1 | unsigned max(value,shared range maximum) |
| MANA_COST | 006b1270+spell*0x2d1 | 006f56a4 |

The combat getters receive `006a5f80+creature*0x5c9` as their object and an
opponent argument,cleaned by the callee. Zero LRC omits the descriptor; nonzero
LRC calls its getter twice. HTH calls once and retains zero values. Zero flying
speed also omits the descriptor. These mappings are confirmed for the tested
input domain; no complete combat algorithm or human-unit interpretation is
claimed. The native model accepts read-only typed values and backend callbacks,
with no original addresses or shared object layouts in its interface.

`UI74.grimoire-dynamic-comparison` records468 original parser/constructor versus
model cases in `working/tests/grimoire-dynamic/run-gv06p3dc`:13 properties,
three resolution modes,three selectors and four value/layout profiles,including
both maximum choices,optional vertical offsets and lowercase dynamic markers.
Eighteen zero-result omissions,one constructor case and one legacy unknown
property case are checked separately. ABI checks cover stack cleanup and
EBX/ESI/EDI/EBP preservation; a private Linux FS segment supplies the original
SEH chain,which is restored after each call. The original parser and constructor
bodies are unchanged. Allocator,CRT character/integer conversion and combat-value
getters are explicit fixture backends,not deployed replacements. Tables and
maxima are authored inputs. Before/after checks verified2927 original files;
prospective source claims stayed stable.

`UI76.grimoire-dynamic-guards` separately records ten intentional native refusals.
Unknown native properties are refused rather than displaying the legacy200/400
default. Guard policies are not recovered original input validation. The model
is linked into the private comparison; native menus and the V22 protocol do not
yet consume its descriptors. Live capture/projection,actual campaign tables,
font metrics,page flow and rendering remain pending. No progress or research
purchase is performed and the native drawing-lock/recovery failure is unchanged.

Static exports remain at `grimoire-controls-dzgwwcna`,`grimoire-controls-b0c3xm7j`
and `grimoire-controls-34dfs5di` under working/decompiled,with durable UI72/UI73/UI75
records. `run-dkbuufu2` retains the failed assumed property identity check;
`run-2rfb78t0` retains the failed cdecl combat fixture and GDB diagnosis. The
corrected thiscall receiver/argument contract passed in `run-wet6ncxe`,then the
expanded profiles passed in `run-_nb_gs54`;the final guard-separated declaration
passed in `run-gv06p3dc`. Failed/predecessor artifacts keep their original hashes.
