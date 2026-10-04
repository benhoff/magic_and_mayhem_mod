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
- [Read-only asset file interface contract](asset-file-interface.md)
- [Raw asset interface byte comparison](asset-file-comparison.md)
- [Pinned MMSprite rendering-asset evaluation](mmsprite-evaluation.md)
- [Native indexed/direct-colour SPR loading](spr-native-loading.md)
- [Native version-5 ANI tables](ani-native-loading.md)

- [Native persistence/progression readers](persistence-native-loading.md): owned CFG, realm, names and save readers; offline evidence and strict parsing policies.

- [WZD wizard definitions and native loading](wzd-native-loading.md): typed stats, sparse resource selections and action records; offline comparison of all 101 installed files.
