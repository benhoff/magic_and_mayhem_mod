# Per-actor profiles in the native terrain campaign

This bounded native policy binds wizard type0 and Crocodile type17 geometry to
immutable installed CFG bytes. The installed wizard has TileSizeXY1/TileHeight4;
the Crocodile has TileSizeXY2/TileHeight2. Native wizard health, stationary
casting and rival AI remain authored. No wizard movement ANI or original wizard
AI/melee/casting equivalence is claimed.

Navigation resolves geometry using each checkpoint-owned Entity.type. Mixed
logical occupancy and conservative movement reservations retain full-generation
identity. Terrain MTER02 owns a second frozen profile with exactly the same
terrain and selected type0 CFG fields; rebinding checks it against the raw CFG
owned by the installed melee profile. CRB3 and SHP4 bind this mixed policy. Sphere
uses each actor's body and aim offsets; Qt picking uses its horizontal footprint.

The optional mixed terrain campaign has identity native-plains-species-campaign.
New installed-creature map campaigns in Qt select it; CLI create-species selects
it explicitly. The arena finale deliberately retains its existing native arena
profile; that fixture is only three layers high and does not claim installed
wizard geometry. Next/retry must retain each mission's explicit profile. Prior
MTER01/CRB1/CRB2/SHP1/SHP2/SHP3 campaigns remain independently loadable without
migration or rewritten historical evidence.

Fresh ACTOR01.native-window execution pins607 sources, passes117 normal CTests,
six sanitizer contracts, matching native installed contracts and five Qt journey
suites. It checks24 per-phase mixed-profile campaign continuations, exact fresh
Qt/CLI/event-process attack and pursuit resumes, player/AI summons and once-only
victory/defeat with next/retry resource retention. Malformed wizard physical
worlds, selected CFG disagreement and empty CRB3 profiles refuse explicitly.
ACTOR01.retained-native separately verifies exact round-trip and continuation
against three retained pre-change campaign profiles, preserving their original
artifact/report hashes. These are native integration results; earlier shared
source results remain historical where their fingerprints changed.

Unknown species,
forged type/role/resource combinations, overlaps and malformed new bundles must
refuse transactionally. Full map entities, mixed-species creature melee beyond
type17, wizard motion, original scheduling/search/speed, pixels/audio and live
replacement remain outside this change.
