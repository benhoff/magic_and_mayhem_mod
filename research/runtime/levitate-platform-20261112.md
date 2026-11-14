# Levitate's floating platform

The user identified the upper-right zombie in the fiery `run-1w9tyfs_` recording
as the reported “magic carpet” case. A separate controlled reproduction confirms
that this platform is supplied original effect artwork. A rendering change to
remove it would change the game's intended visual content.

## Evidence chain

`COMBAT12.levitate-platform01` / `SP.080.platform-artwork` retains the source,
configuration, original art, failed acquisition and controlled reproduction.

1. Original spell80 dispatch at`0x48b0f0`, case`0x50`, calls`0x522880`.
   The helper reads effect-catalog offsets`0x5a0/0x5a4`: entry90 at16bytes/entry,
   then binds/starts the sprite controller. This is static call/catalog evidence,
   not recovery of the whole spell, helper, weight test or duration formula.
2. The installed decoded`effectani.cfg` entry`ANI_90` specifies`EFFECTS11`,
   animation2 and`NORMAL`. `Effects11.ani` sequence2 includes frames82–107.
3. Independent RLE decoding of original`Sprites/Effects11.spr` frames95/99
   shows the raised brown platform visible beneath the zombie:55×32 pixels,
   origin29,17. The SPR SHA-256 is
   `5b6cfda83380a2543dd01e4192a900a6935ee00f56b53d72c21f30bd5eee483e`.
4. The controlled native-menu fixture uses an ordinary offered Neutral Levitate
   item and Chaos Zombie item. One summoned Zombie12 is selected from its exact
   retained sprite position. The original cast log records spell80 targeting12,
   caster mana47360→44288. Its observed height rises4→7 and XY then drifts.
5. The retained292 consecutive completed World packets start at1, verify source
   and request CRC32C without missing generations, and include original return
   records. Before the cast the platform family is absent at the target; after
   it the original raster requests contain exact frame83,95,99 bytes. Generation
   120 requests frame95; generations150/200 request99 at the floating platform.
   Both copy and projected-shadow requests remain original producer inputs.
6. Offline replay validates all292 actual returns and reconstructs native World
   history continuously from initial creation. The platform is present before
   Qt UI composition, including on llvmpipe. This is native replay and asset
   identification; it is not an independent original-raster/driver pixel oracle.

The preliminary controlled08 attempt selected the wizard instead of the zombie
and failed its required target condition. It remains negative evidence. The
corrected09 helper targets the sole zombie using exact retained source geometry;
active-window/focused-viewport input guards remain enforced. Each session uses
its own prefix and ends by diagnostic cancellation, not normal Quit.

The setup variant changes only the private smoke-test loadout under two explicit
probe flags. Exact baseline/variant sources, private tested binary, collectors,
logs, source histories, captures, videos and replay binary are retained in pinned
SQLite stores. Production source is restored, and the baseline native binary
again hashes`762cfb0ebfbbfa9ba2cd66469df1796cdccd895d2431bceb65f45575f5967e1e`.
No production renderer, input, spell or gameplay policy changes in this finding.

This closes the artwork-identification question for the reproduced carpet.
The earlier fiery recording's exact mid-frame source is unavailable; the user
identified the analogous visual and the controlled case supplies the direct
source/art match. Other visual reports, full Levitate rules/equivalence and
moving-combat19–20FPS remain separate. Historical `moving01` keeps its original
unresolved assessment and hashes; this is a new result, not refreshed old proof.
