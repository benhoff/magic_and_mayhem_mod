# Deterministic native World replay, 2026-11-13

The new [replay tool](../../tools/render-replay.py) stores a complete ordered
World drawing stream in SQLite, then restores producer/canvas history from
generation 1 before viewing, hashing or timing a selected range. The original
engine runs only during acquisition. AI, simulation, combat, animation clocks,
caller admission and balance are unchanged. See [usage](../../docs/render-replay.md).

Confirmed bounded results:

- Eleven controls pass on llvmpipe and Apple M1: compressed admission and
  corruption refusal, 32 moving authored sprite instances, fresh-repeat hashes,
  selected-range history, source-only initialization, actual viewer backward
  seek, baseline comparison and a damaged baseline refusing at frame 8.
- Existing capture controls pass, including native/Python prefix compatibility,
  excluded-tail validation, the opt-in two-World recording cap, and invalid caps
  refusing before output creation. The normal recording mode remains unchanged.
- An owned four-owner battle acquisition retained 868 complete World updates.
  Actor observations reached 38 living actors; 35 actor slots changed position,
  and 57 cross-owner melee damage events were retained. These are acquisition
  observations, not counts of sprites visible in every selected frame.
- Actual captured frames 700–820 have identical native pixel hashes across two
  fresh M1 replays. Three benchmark repeats end with the same pixel hash.
  Ordinary World readbacks are zero; verification uses 121 diagnostic reads per
  repeat and benchmarking one final diagnostic read per repeat.
- Across 363 benchmark samples, native serial completion is median 11.760 ms,
  mean 19.551 ms, p95 83.676 ms and maximum 111.383 ms. Thirty samples exceed
  the 50 ms budget for 20 updates/second. Frame 817 has median 102.977 ms
  completion / 102.360 ms submission and 943 drawing calls; frame 820 has
  98.971 / 98.209 ms and 891 drawing calls. The same late frames remain slow
  across repeats. This supplies fixed input for subsequent native optimization;
  it does not identify individual CPU functions or GPU busy time.

Timing excludes capture decode/I/O, diagnostic readback and pixel hashing. A
completion wait drains earlier warmup work before the first selected frame and
serializes selected frames. These measurements are isolated renderer service
times, not live FPS, input latency, Qt pacing or physical scanout. Repeatable
native pixels are a regression reference, not original-game pixel equivalence.

The acquisition did **not** finish its requested input script or normal Quit.
The next World, generation 869, contains the existing guarded indexed-raster
failure: reason 43, tag 14, caller `0x54a563`. The guard was not widened. The
valid complete prefix was explicitly imported; failure diagnostics remain
retained separately. No full-campaign, long-session or visual artifact-free
acceptance is claimed. A manual normal-Quit recording through the new convenience
CLI remains a separate end-to-end validation boundary.

The retained case is
`working/deterministic-render-replay-20261113/crowd.sqlite3` in the owned task
checkout. Its format-4 acquisition SHA-256 is
`a7ebf7af62e356b944f3a1e82fe40111bf94e6486f7c2331b8ade5167287f5e9`.
The case stores 85,011,755 compressed payload bytes for 1,792,255,096 raw bytes.
All referenced assets are pinned by size/hash and shared read-only.

Evidence and exact source contracts are registered under
`NR.deterministic-world-stream-replay`. `NATIVE13.render-replay01` retains the
initial observations above. `NATIVE13.render-replay02` repeats the controls and
captured-case execution after the final viewer pacing correction: playback
schedules its next deadline from frame start, so drawing time is not added to
the requested period. The corrected executable matches the initial captured
pixel baseline. Neither report replaces an older source hash or failure.
SQLite results retain tested ELF/tool bytes, per-frame timings/hashes, capture
inputs, commands, failures and original-manifest checks. Verified local SQLite
backups are required before finish. Compiler intermediates and exported replay
tapes are reconstructible temporary output; captures, cases, exact tested
binaries and failure inputs remain durable evidence.
