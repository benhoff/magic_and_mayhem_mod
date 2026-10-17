# Native campaign graph persistence (version 5)

This is a native campaign format, separate from original saves. Existing versions
1–4 retain their layouts and selection rules. Version 5 is emitted whenever the
new optional graph continuation exists, including the initial idle checkpoint.

`MNMCAMP\0`, little-endian uint32 version **5**, retains all version-2 fields
through the optional last result, then appends:

| Field | Encoding | Bound |
| --- | --- | --- |
| Completed count | uint32 | 0–16 |
| Ordered completed mission indexes | count × uint32 | Unique indexes 0–15 |
| Choice count | uint32 | 0–16 |
| Committed alternative indexes | count × uint32 | Unique indexes 0–15 |
| Objective continuation present | uint8 boolean | Exactly 0 or 1 |
| Optional objective continuation | Version-4 groups/objectives body | Existing 8-group/16-objective bounds |

The objective block has implicit definition/progress version 2, with no nested
magic/version word. Its group/actor counts, handle generations, completion flags
and elapsed counters follow [the objective format](native-mission-objectives.md).
Legacy staged mission continuation is not allowed inside a graph save.

The initial graph record is allowed without a battle or last result. Any active
battle needs a nonzero attempt. Unsupported versions, duplicate/out-of-budget
indexes, malformed booleans, truncation and trailing bytes refuse. Definition-bound
restore additionally checks the actual node count, prerequisites, exclusive choices,
ordered completion/region ownership, selection, last result and derived spell roster.

The outer native snapshot envelope and battle/driver/effect wire formats are
unchanged. Saved indexes bind to the exact compiled definition through the existing
campaign content revision. This is not an integrity signature or a migration format.
Old native identities continue selecting their original definitions and encodings.

## Compiled chapter definition

The version-1 JSON object has exactly `version`, `id`, `realm`, `entry`, `finale`,
`initial_spells` and `missions`. `missions` contains 2–16 ordered objects with exactly
`id`, `map`, `requires_all`, `requires_any`, `choice`, `rewards` and `plan`.

`map` is `plains` or `arena`, resolved by the application resource adapter.
Prerequisites are mission-ID arrays. An empty `choice` means independent; equal
nonempty choice IDs identify exclusive alternatives. Supported spell IDs are
`summon-crocodile`, `arc-bolt`, `cure` and `magic-sphere`. A plan is null for an
ordinary battle, or an existing version-2 objective JSON object with empty
`reward_spell`. Rewards are declared on the campaign mission node instead.

The parser caps input at 256 KiB, strings at 255 bytes, initial spells/rewards at
four entries, and dependency lists at sixteen entries. The core caps choice IDs
at 64 bytes and rejects NUL, duplicate or unsupported fields/values. Entry/finale
must identify the first/last mission. Prerequisites refer only to earlier nodes;
all reachable completion states must have a continuation to the finale, and every
mission must be reachable on at least one route. These checks do not establish
combat solvability for arbitrary mission content.
