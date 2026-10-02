# CFG plaintext/encrypted precedence

## Question

Can a plaintext file such as `CFG/tables.cfg` be edited before launch while a
matching `CFG/Encrypted/tables.cfg` exists, or does the encrypted copy replace
it?

## Reproducible live probe

Run the following from a normal graphical shell:

```bash
./tools/cfg-precedence-experiment.py
```

The tool appends a comment-only unique marker to the runtime plaintext file,
runs the game in a bounded Wine desktop, checks both the plaintext and decoded
encrypted copy, and restores the exact two original runtime files in a
`finally` block. It never touches `original/`. Evidence and a report are saved
under `working/experiments/cfg-precedence/` in a new directory, so an existing
report is never overwritten.

## Static evidence

Confidence: high for both the control flow and the live precedence result.

The clean `Chaos.exe` with SHA-256
`124a0601759f6b0607d6f81c94c47b1bde05e747456bfaba68ab9da84d800214`
contains two loops over `Cfg/Encrypted/*.cfg`:

- Startup routine `0x004d9bf0` calls `FindFirstFileA` at `0x004d9c23`, builds
  `Cfg/Encrypted/<name>`, decodes it through the container routines near
  `0x00498380`, then builds `cfg/<name>` and passes the decoded buffer to the
  file-writing path near `0x004982f0`.
- Shutdown routine `0x004da8a0` calls `FindFirstFileA` at `0x004da99f`, reads
  `cfg/<name>`, writes `Cfg/Encrypted/<name>` through the container writer near
  `0x00498320`, and calls `DeleteFileA` on the plaintext path at `0x004dab60`.

This explains the observed plaintext siblings after bounded or interrupted
launches: startup materializes them, while a clean shutdown is intended to
repack and remove them.

## Confirmed live result

The live probe completed successfully on 2026-10-02. Its report is retained at
`working/experiments/cfg-precedence/run-20261002T042714Z-bdj4jygp/report.md`.

- The bounded launch returned success.
- The marker was absent from plaintext after startup.
- The encrypted container did not change.
- Decoding the encrypted container did not produce the marker.
- The experiment restored both runtime files to their original hashes.

This confirms that when both forms exist before startup, the encrypted copy
wins and replaces the plaintext copy. A pre-launch plaintext-only edit is not
a durable mod while its encrypted counterpart exists.

Durable data mods should therefore be packaged as valid encrypted CFG
containers. Installing plaintext after startup is theoretically possible, but
the configuration may already have been parsed and is not a reliable packaging
strategy.

The earlier sandbox-blocked attempt is retained at
`working/experiments/cfg-precedence/run-20261002T042208Z-6qbb8gx1/` as negative
test evidence; it made no runtime changes.
