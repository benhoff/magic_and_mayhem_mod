# Qt engine Preferences store (schema 1)

This is an intentional application persistence format, separate from the
[original profile format](preferences-config.md) and native menu sound settings.
The normal Qt live-menu application uses
`QStandardPaths::AppConfigLocation/engine-preferences.json` (normally
`$XDG_CONFIG_HOME/mnm-qt-shell/engine-preferences.json`). Tests redirect the config
home or supply a temporary store path.

```json
{
  "schema": 1,
  "source_sha256": "40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168",
  "values": [0, -500, 0, 0, 2, 0, 0]
}
```

The seven integers use the V6 semantic ordering: music level 0..15, effects
attenuation -2500..0, resolution High/Low 0/1, animation Full/Cut 0/1, dialogue
Fast/Medium/Slow 0/1/2, game speed Fast/Medium/Slow 0/1/2, border On/Off 1/0.
The importer validates the schema, exact build and integer bounds; JSON booleans,
floats, duplicate keys, malformed and oversized documents are rejected. Missing
stores use staged installation defaults. Invalid/unreadable stores are reported
and ignored on startup, retaining their bytes. A subsequent explicit OK may
repair a readable corrupt store using validated original output. Oversized or
unreadable stores cannot be overwritten by this path.

On staging, the importer updates only the seven corresponding keys in both
`CFG/prefs.cfg` and `CFG/Encrypted/prefs.cfg`. Both copies must pass missing-key,
duplicate-key and container round-trip checks before either is changed. It
preserves unrelated keys, comments and line endings. The existing CD-music/movie
staging overrides still apply after import and are never copied into this store.
The source working installation and immutable original media are not edited.

The Qt session saves only an accepted Preferences OK after original dispatch
and a ready Main return. It independently reads the plain original-written file,
requires all seven settings to equal the submitted values, and rejects ambiguous,
incomplete or out-of-range data. A callback acknowledgement alone cannot save.

A nonblocking `QLockFile` serializes cooperating application writers. Under that
lock, the existing store's raw SHA-256 must match the revision imported during
staging (`missing` when absent). Another session's write is preserved on conflict;
the new settings remain in the current game and its evidence directory. A
successful `QSaveFile` atomic commit updates the in-session revision. Direct-write
fallback is disabled. Failed persistence reports its reason and keeps the game
session active. Startup, audio previews, Cancel and window-close Cancel do not
write the store. Successful OK commits immediately; normal shutdown is not
required afterwards.

This is selected-setting persistence, not whole-profile copying, a new original
settings writer, or a gameplay change. Crash/power-loss durability, physical
hardware, unsupported caller paths and real resolution changes have separate
validation boundaries. See [runtime evidence](../runtime/preferences-persistence.md).
