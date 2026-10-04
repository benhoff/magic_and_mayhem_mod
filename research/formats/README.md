# File-format research

Document one format per Markdown file. Each finding should include:

- Source artifact and version/hash.
- Inspection method or reproducer.
- Confirmed fields and behavior.
- Hypotheses, clearly labeled as such.
- Confidence level and supporting evidence.
- Unknowns and the smallest useful next experiment.

Current findings:

- [Installed game file inventory](game-file-inventory.md)
- [Encrypted CFG container](encrypted-cfg.md)
- [Save game envelope and block inventory](save-game.md)
- [CFG plaintext/encrypted precedence](cfg-precedence.md)
- [Mode-0 CFG writer](cfg-writer.md)
- [Shared RGBA frame stream](render-frame-stream.md)
- [Native blit capture and draw events](render-draw-capture.md)

- [Qt media request channel](render-media-channel.md)
- [PCM WAV loading](pcm-wav-loading.md)
- [Native audio profile input](profile-native-loading.md): bounded ASCII snapshots, case/quote/capacity policies and Qt file-to-audio integration; synthetic evidence.
- [PE32 Wine profile API comparison](profile-api-comparison.md): 29 synthetic calls, measured whitespace/capacity fixes and explicit native policy differences.
- [Installed audio profile/catalog comparison](installed-audio-profile-comparison.md): Qt/PE32 Wine agreement for 499 installed calls and derived source/group tables; offline only.
- [Read-only asset file interface contract](asset-file-interface.md)
- [Raw asset interface byte comparison](asset-file-comparison.md)
- [Pinned MMSprite rendering-asset evaluation](mmsprite-evaluation.md)
- [Native indexed/direct-colour SPR loading](spr-native-loading.md)
- [Native version-5 ANI tables](ani-native-loading.md)

- [Native persistence/progression readers](persistence-native-loading.md): owned CFG, realm, names and save readers; offline evidence and strict parsing policies.

- [WZD wizard definitions and native loading](wzd-native-loading.md): typed stats, sparse resource selections and action records; offline comparison of all 101 installed files.

- [Native CUR cursor loading](cur-native-loading.md): owned indexed images, AND/XOR planes and hotspots; offline comparison of all 20 installed files/22 images.

- [Native PCX loading](pcx-native-loading.md): version 5 indexed RLE images, palette preservation and offline validation.

- [Native BMP loading](bmp-native-loading.md): bounded 24-bit BI_RGB decoding, orientation/padding conversion and offline validation.

- [Native JPEG loading](jpeg-native-loading.md): Qt JPEG backend with owned RGB output, explicit limits and installed corpus comparison.

- [Native MPS map placement loading](mps-native-loading.md): original-reader-confirmed 40-byte records, opaque size metadata and owned offline input.

- [Native EVT event-area loading](evt-native-loading.md): confirmed 72-byte records, writer size metadata, raw names and offline validation.

- [Native TAG sprite-name tables](tag-native-loading.md): headerless 12-byte records, complete companion SPR correlation and bounded native input.

- [Native FP flag-path loading](fp-native-loading.md): version-2 Realm Viewer paths, bounded eight-slot ranges, flag positions and complete installed comparisons.
