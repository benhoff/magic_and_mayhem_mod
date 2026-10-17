# Native objective campaign continuation (version 4)

This is a native format, separate from original game saves. Existing campaign
versions 1–3 and their byte layouts are unchanged; see
[native staged mission](native-staged-mission.md).

`MNMCAMP\0`, little-endian uint32 version **4**, retains every version-2 campaign
field through optional last result, then appends:

| Field | Encoding | Bound |
| --- | --- | --- |
| Group count | uint32 | 0–8 |
| Each group handle count | uint32 | 0–8; zero means admission pending |
| Each actor | uint32 slot, uint32 generation | nonzero generation, globally unique |
| Objective count | uint32 | 1–16 |
| Each completion | uint8 boolean | exactly 0 or 1 |
| Each elapsed counter | uint32 | 0–4096 |

The definition/progress version is implicitly 2. Legacy `stage`, `reinforcement`,
`initial` and `wave` fields are absent. An active nonzero battle attempt is
required. Empty group vectors preserve pending events; admitted full-generation
handles preserve one-shot events. Only survive objectives may carry elapsed
counters. Counts, truncation, booleans, unsupported versions and trailing bytes
are refused before resource-bound restore.

The envelope and `MNMARM03` battle metadata are unchanged. The exact definition
bytes are compiled into the application and included in campaign content identity;
objective/group arrays bind saved positional indexes to stable authored IDs.
Resource restore validates graph, geometry, typed progress, actors and the disabled
battle victory gate before the campaign is adopted. Old identities select their
old definitions and are never silently migrated to the new campaign.

Version-2 definition JSON has exactly `version`, `id`, `briefing`, `reward_spell`,
`groups` and `objectives`. Each group has `id`, XYZ `locations` and `after` objective
IDs. Each objective has `id`, `text`, `kind`, `after`, and either integer `ticks`
for `survive` or a `target` object for `eliminate`/`protect`. A target is exactly
`{"group":"ID"}` or `{"wizard":"player"}` / `{"wizard":"rival"}`. Protection
supports wizards only. Empty reward means no spell reward; `cure` is the supported
nonempty reward. Semantic graph validation follows parsing. JSON input is capped
at 64 KiB, each text at 512 bytes, and each dependency list at 16 IDs.
