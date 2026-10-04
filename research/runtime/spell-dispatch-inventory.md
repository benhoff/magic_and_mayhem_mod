# Spell dispatch inventory (No-CD build)

Reviewed 2026-10-04. Companion to [creature AI, combat and spells](creature-ai-combat-spells.md),
using the same pinned executable and static evidence/confidence limits.

This is a research checklist, **not an implementation or equivalence claim**.
Names come from decoded `working/decoded-cfg/spells.cfg`; that derivative's SHA-256
is `6c81fc699ec3e37de83c1837570f6c1fb1c910b6c6263f5bfd8b88883df7738a`. Original encoding/loader mapping remains separate work.

“Case” means an explicit case label in read-only Ghidra output for cast admission
`0x0057b710` or effect dispatch `0x0048b0f0`. Those complete assembly ranges are
included in `tools/export-creature-behavior.py`; reproduce pseudocode using
`tools/ghidra/ExportCreatureBehavior.java` with entries `57b710 48b0f0` in a
read-only analyzed project. Shared case labels do not imply identical semantics:
delivery mode, owner, target, type, status and terrain gates also participate.
A dash means no explicit label in the inspected switch, **not proof of an unused
or unreachable spell**. Secondary update type dispatch and direct effects can
supply other paths. Configuration entries named Unused are retained deliberately.

Every row requires effect/resistance/stacking/duration/cleanup and failure-path
validation before native replacement. Selected rows have an initial contract in
the main document; the rest have dispatch evidence only.

| ID decimal / hex | Configuration name | Cast case | Effect case | Detailed contract status |
| --- | --- | --- | --- | --- |
| 0 / `0x00` | Noname | Case | Case | Summon admission path; full placement/creation pending |
| 1 / `0x01` | Brownie | Case | Case | Summon admission path; full placement/creation pending |
| 2 / `0x02` | Centaur | Case | Case | Summon admission path; full placement/creation pending |
| 3 / `0x03` | Elf | Case | Case | Summon admission path; full placement/creation pending |
| 4 / `0x04` | Griffin | Case | Case | Summon admission path; full placement/creation pending |
| 5 / `0x05` | Hero | Case | Case | Summon admission path; full placement/creation pending |
| 6 / `0x06` | Phoenix | Case | Case | Summon admission path; full placement/creation pending |
| 7 / `0x07` | Unicorn | Case | Case | Summon admission path; full placement/creation pending |
| 8 / `0x08` | Hell hound | Case | Case | Summon admission path; full placement/creation pending |
| 9 / `0x09` | Manticore | Case | Case | Summon admission path; full placement/creation pending |
| 10 / `0x0a` | Redcap | Case | Case | Summon admission path; full placement/creation pending |
| 11 / `0x0b` | Skeleton | Case | Case | Summon admission path; full placement/creation pending |
| 12 / `0x0c` | Vampire | Case | Case | Summon admission path; full placement/creation pending |
| 13 / `0x0d` | Wraith | Case | Case | Summon admission path; full placement/creation pending |
| 14 / `0x0e` | Zombie | Case | Case | Summon admission path; full placement/creation pending |
| 15 / `0x0f` | Bat | Case | Case | Summon admission path; full placement/creation pending |
| 16 / `0x10` | Basilisk | Case | Case | Summon admission path; full placement/creation pending |
| 17 / `0x11` | Crocodile | Case | Case | Summon admission path; full placement/creation pending |
| 18 / `0x12` | Dragon | Case | Case | Summon admission path; full placement/creation pending |
| 19 / `0x13` | Faun | Case | Case | Summon admission path; full placement/creation pending |
| 20 / `0x14` | Champ of Law | Case | Case | Summon admission path; full placement/creation pending |
| 21 / `0x15` | Troll | Case | Case | Summon admission path; full placement/creation pending |
| 22 / `0x16` | Spying Eye | Case | Case | Summon admission path; full placement/creation pending |
| 23 / `0x17` | Champ of Chaos | Case | Case | Summon admission path; full placement/creation pending |
| 24 / `0x18` | Unused | — | Case | Summon admission path; full placement/creation pending |
| 25 / `0x19` | Unused | — | Case | Summon admission path; full placement/creation pending |
| 26 / `0x1a` | Unused | — | Case | Summon admission path; full placement/creation pending |
| 27 / `0x1b` | Unused | — | — | Pending |
| 28 / `0x1c` | Unused | — | — | Pending |
| 29 / `0x1d` | Unused | — | — | Pending |
| 30 / `0x1e` | Unused | — | — | Pending |
| 31 / `0x1f` | Unused | — | — | Pending |
| 32 / `0x20` | Unused | — | — | Pending |
| 33 / `0x21` | Gooey Blob | Case | Case | Pending |
| 34 / `0x22` | Meteor Shower | Case | Case | Pending |
| 35 / `0x23` | Tangle Vine | Case | Case | Pending |
| 36 / `0x24` | Unsummon delay | — | — | Pending |
| 37 / `0x25` | Unused | — | — | Pending |
| 38 / `0x26` | Bury | Case | Case | Pending |
| 39 / `0x27` | Unused | — | — | Pending |
| 40 / `0x28` | Unused | — | — | Pending |
| 41 / `0x29` | Cure | Case | Case | Cure initial map; cleansing/refund pending |
| 42 / `0x2a` | Blood Lust | Case | Case | Blood Lust initial map; expiry/interaction pending |
| 43 / `0x2b` | Disenchant | Case | Case | Pending |
| 44 / `0x2c` | Earthbind | Case | Case | Pending |
| 45 / `0x2d` | Unused | — | — | Pending |
| 46 / `0x2e` | Gorgon Stare | Case | Case | Pending |
| 47 / `0x2f` | Unused | — | — | Pending |
| 48 / `0x30` | Unused | — | — | Pending |
| 49 / `0x31` | Unused | — | — | Pending |
| 50 / `0x32` | Lure | Case | Case | Pending |
| 51 / `0x33` | Unused | — | — | Pending |
| 52 / `0x34` | Raise Dead | Case | Case | Pending |
| 53 / `0x35` | Unused | — | — | Pending |
| 54 / `0x36` | Subversion | Case | Case | Pending |
| 55 / `0x37` | Teleport | Case | — | Pending |
| 56 / `0x38` | Unused | — | — | Pending |
| 57 / `0x39` | Unused | — | — | Pending |
| 58 / `0x3a` | Fire | Case | Case | Pending |
| 59 / `0x3b` | Unused | — | — | Pending |
| 60 / `0x3c` | Unsummon | Case | Case | Pending |
| 61 / `0x3d` | Unused | — | — | Pending |
| 62 / `0x3e` | Super Fireball | Case | Case | Pending |
| 63 / `0x3f` | Unused | — | — | Pending |
| 64 / `0x40` | Unused | — | — | Pending |
| 65 / `0x41` | Pestilence | Case | Case | Pending |
| 66 / `0x42` | Unused | — | — | Pending |
| 67 / `0x43` | Unused | — | — | Pending |
| 68 / `0x44` | Sloth | Case | Case | Pending |
| 69 / `0x45` | Totem of Life | Case | Case | Pending |
| 70 / `0x46` | Guardian | Case | Case | Pending |
| 71 / `0x47` | Fireball | Case | Case | Pending |
| 72 / `0x48` | Lightning | Case | Case | Pending |
| 73 / `0x49` | Magic Sphere | Case | — | Pending |
| 74 / `0x4a` | Iron Skin | Case | Case | Pending |
| 75 / `0x4b` | Tornado | Case | Case | Pending |
| 76 / `0x4c` | Terror | Case | Case | Pending |
| 77 / `0x4d` | Magic Mist | Case | Case | Pending |
| 78 / `0x4e` | Scythian Bow | Case | Case | Pending |
| 79 / `0x4f` | Lucifers Farewell | Case | Case | Pending |
| 80 / `0x50` | Levitate | Case | Case | Pending |
| 81 / `0x51` | Judgement | Case | Case | Pending |
| 82 / `0x52` | Haste | Case | Case | Pending |
| 83 / `0x53` | Excalibur | Case | Case | Pending |
| 84 / `0x54` | Ornithopter | Case | Case | Pending |
| 85 / `0x55` | Invisibility | Case | Case | Pending |
| 86 / `0x56` | Illusion | Case | Case | Pending |
| 87 / `0x57` | Chain Lightning | Case | Case | Pending |
| 88 / `0x58` | Storm Lightning | Case | Case | Pending |
| 89 / `0x59` | Super Fireball | Case | Case | Pending |
| 90 / `0x5a` | Creature Explode | Case | — | Direct explosion path; parameters pending |
| 91 / `0x5b` | Totem of Fear | Case | Case | Pending |
| 92 / `0x5c` | Lorelei | Case | Case | Pending |
| 93 / `0x5d` | Arrow | Case | Case | Pending |
| 94 / `0x5e` | Stone | Case | Case | Pending |
| 95 / `0x5f` | Spear | Case | Case | Pending |
| 96 / `0x60` | Quill | Case | Case | Pending |
| 97 / `0x61` | Hypnotic Gaze | Case | Case | Pending |
| 98 / `0x62` | Magic Arrow | — | Case | Pending |
| 99 / `0x63` | Fire Attack | Case | Case | Pending |
| 100 / `0x64` | Bolt Lightning | Case | — | Pending |
| 101 / `0x65` | Unused | Case | Case | Pending |
| 102 / `0x66` | Unused | Case | Case | Pending |
| 103 / `0x67` | Unused | — | — | Pending |
