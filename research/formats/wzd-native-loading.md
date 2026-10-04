# WZD wizard definitions and native loading

Reviewed 2026-10-04. This milestone is an **offline read-only loader**, not a
native wizard factory, campaign initialization service or AI action executor.
Schema evidence comes from all 101 installed `working/game-clean/Wizards/*.wzd`
files (159,665 bytes). Confidence is high for their text syntax, keys, observed
values and typed byte comparisons. Gameplay meanings suggested by key names and
comments are descriptive; original reader defaults, clamping and runtime
application remain unverified.

Representative input SHA-256 values:

| File | Bytes | SHA-256 |
| --- | ---: | --- |
| `Celtic11.wzd` | 437 | `319fd9ddb03ef2c4205f51b005ab84e90d26c2cdd0f3ad23314efe933a072252` |
| `_default.wzd` | 10,798 | `d1278c7cdb607fef86236b8886236f6d3f41b251947a18ede65ff9fb645e37fa` |
| `_Test2.wzd` | 14,202 | `893fc5cf3f6d010bed7d2e47b60fe373e3a9df9d9f4ef2c1a0aea108898438fa` |

## Installed schema

WZD is section/key/value text, with CRLF/LF, whitespace, `;` comments and quoted
names. Values can have inline comments. Section and key matching in the native
reader folds ASCII case. Name quotes are removed in the typed result; raw source
and comment-normalized configuration properties remain available separately.

| Section | Fields and native representation |
| --- | --- |
| `HEADER` | Optional `ValidWizardFile` boolean; present in 10 files, absent in 91 |
| `GENERAL` | Required `Name` and `MaxMana`, present in all 101 files |
| `GENERAL` stats | Optional signed integers `MaxHealth`, `Intelligence`, `MagicResistance`, `ControlLimit`, `CombatModifier`, `DifficultyModifier` |
| `GENERAL` AI weights | Optional signed integers `AggressiveAttack`, `CautiousAttack`, `OccupyPowerPoint`, `CollectMana`, `EvadeDetection`, `Scout`, `Retreat`, `CollectFood`, `ProtectWizard` |
| `GENERAL` resources | Optional string `WizardAnimFile`; optional signed integers `StartTalismans_Law`, `_Neutral`, `_Chaos` |
| `ITEM_<decimal>` | Optional `HasIt` and `Researched` boolean fields, keyed by numeric item ID |
| `START_SPELLS` | Sparse `SPELL_<decimal>` boolean map |
| `START_OBJECTS` | Sparse `OBJECT_<decimal>` boolean map |
| `START_MAGIC_ITEMS` | Sparse `MITEM_<decimal>` boolean map, distinct from `ITEM_*` records |
| `ACTION_<decimal>` | Optional typed fields below, keyed by numeric action ID |

The files contain 19 distinct numeric GENERAL keys plus Name and WizardAnimFile.
MaxHealth appears in 98 files, Intelligence/MagicResistance/ControlLimit in 92,
CombatModifier/DifficultyModifier/WizardAnimFile in 91, the nine AI weights in
90, and each talisman key in four. Six files have 36 ITEM sections each; ten have
START_OBJECTS and four have START_MAGIC_ITEMS. All 101 have START_SPELLS, but
sections can be empty. Commented-out spell/object lists in default files are
examples, not active grants. No active duplicate properties or non-property
lines were observed.

Only `_Test2.wzd` supplies ACTION sections (01, 02 and 03). Their fields are:

- Boolean `Valid`.
- Signed integers `BeginTime`, `EndTime`, `Dependancy` (source spelling),
  `WaitingTime`, `Rating`, `NumberOfActions`, `Health`, `Mana`, `SectionID`,
  `SpellType`, `SpellTargetID`, `XPosition`, `YPosition`, `ZPosition`,
  `XTargetPosition`, `YTargetPosition`, `ZTargetPosition`.
- Byte strings `NodeType`, `TargetNodeType`, `Destination`, `SpellTarget`.

`Dependancy` becomes the native member `dependency`. Symbolic action strings
are retained rather than assigned speculative enum values. `_Test2` has
SpellType -1; shipped DifficultyModifier values include -50 and other numeric
fields exceed 100. The loader therefore preserves signed numbers without
applying gameplay ranges, inferred percentages or balance changes.

## Native API and policies

[assets/wizard.hpp](../../assets/wizard.hpp) provides `WizardDefinition`,
`WizardStats`, `WizardItem`, `WizardAction`, `WizardLimits`, `decodeWizard(bytes)`
and `loadWizard(file)`. Link `mnm-wizard-loader`; it reuses the native CFG parser
through `mnm-persistence-loader` and has no dependency on widgets, legacy hooks
or build-specific reconstruction. Public types use the standard library.

`WizardResult` reuses the existing value/error variant with `PersistenceError`.
Check the error alternative before extracting a definition. All strings, maps,
configuration properties and exact source bytes are owned. A successful result
survives destruction of its input buffer, file handle and store. The inspector
closes the file before emitting decoded fields.

Missing optional values remain `std::nullopt`; absent indexed entries remain
absent. Explicit zero/false remain present values. This preserves sparse
configuration for a future, separately validated defaults/application layer.
The loader does not load WizardAnimFile's SPR/ANI, resolve numeric IDs against
other tables, grant resources, schedule actions or mutate the simulation.
Unknown properties/sections are preserved in `config`; comments and exact
formatting survive in `source`.

Native policies (not claims about original parser failure behavior):

- Require a nonempty GENERAL/Name and a valid signed-32-bit GENERAL/MaxMana.
  Accept missing HEADER; if ValidWizardFile is present, require true.
- Accept booleans TRUE/FALSE case-insensitively or numeric 1/0. Integers are
  strict signed decimal; reject overflow, trailing junk and leading `+`.
- Strip semicolon comments outside paired double quotes; retain quoted
  semicolons. Reject unterminated/multiline quotes and embedded NUL, including
  inside comments. No backslash/doubled-quote escape grammar is asserted.
- Reject malformed CFG sections, duplicate case-folded properties and unexpected
  non-property lines. Reject malformed suffixes in recognized indexed names.
  Numeric aliases such as SPELL_01/SPELL_1 cannot produce two records for one ID.
- Default limits: 256 KiB input, 8,192 lines, 4,096 total recognized indexed
  entries, maximum ID 65,535 and 1,024 bytes per typed string. These are native
  resource limits, not recovered original table capacities. Sparse maps avoid
  allocation proportional to the largest ID. Unknown fields remain bounded by
  the file/line limits.
- Byte offsets accompany syntax failures; semantic field errors use offset zero
  and identify the field. File failures retain the underlying AssetFile error.
  Allocation/length failures become limit errors rather than partial results.

## Reproduce and inspect

```bash
cmake -S assets -B working/build/wizard -DBUILD_TESTING=ON
cmake --build working/build/wizard --parallel 4
ctest --test-dir working/build/wizard --output-on-failure
working/build/wizard/mnm-wizard-inspect working/game-clean Wizards/Celtic11.wzd
python3 tests/test-wizard-loader.py working/build/wizard/mnm-wizard-inspect \
    --installation working/game-clean \
    --report working/tests/wizard-loader/installed-comparison.json
```

The inspector accepts ROOT/PATH, resolves through AssetStore and emits JSON for
all typed values, normalized configuration and exact source SHA-256. Missing
optional fields are JSON null. It returns 2 on failure. Qt is used only by the
inspector/backend, not in wizard data types or parsing.

The installed comparison report inventory SHA-256 is
`e44ff0b92e04aebf9130738db821469c4d878b37eb307951ac4dec7bfe5294f6`
(canonical JSON of its ordered per-file records). These are installed extracted
inputs; the original manifest verifies source-media preservation separately.

The comparison runner verifies the immutable original manifest before/after,
including on failure. It compares every normalized property and typed value
against Python ConfigParser plus an independent quote-aware comment pass. Its
report records each file's path, size, source hash, decoded hash and indexed
counts. Omit `--installation` for fixtures without game artifacts.

Validation: nine asset CTests pass, including native sparse/full/malformed,
limits, aliases, byte ownership and file lifetime fixtures, and three independent
text comparisons. All 101 installed WZDs match the Python reference, including
all three action sections and the default/start variants. AddressSanitizer and
UndefinedBehaviorSanitizer pass for both wizard tests; leak detection is disabled
because LeakSanitizer cannot run under this environment's tracing. The original
manifest remains unchanged (2,927 files).

No original-engine execution or live integration was performed. Next trace the
original WZD reader's field defaults, indexing, ID/resource binding and context
selection before applying these definitions to native gameplay. Compare a
bounded original/native wizard initialization independently of parser evidence.
