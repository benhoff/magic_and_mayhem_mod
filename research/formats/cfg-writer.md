# Mode-0 CFG writer

## Purpose

`tools/encode-cfg.py` packages plaintext configuration data as a valid
encrypted CFG container using packing mode 0. This is the smallest writer
needed for durable data mods because startup gives `CFG/Encrypted/*.cfg`
precedence over plaintext siblings.

With no arguments, the tool reads runtime `tables.cfg`, encodes it in memory,
decodes it again, and reports the result without writing:

```bash
./tools/encode-cfg.py
```

An explicit output path enables writing:

```bash
./tools/encode-cfg.py path/to/plain.cfg --output path/to/container.cfg
```

Existing output requires interactive confirmation or `--force`. Output is
written to a temporary sibling and atomically renamed.

## Confirmed implementation details

Confidence: high from static analysis, local round-trip validation, and a
successful engine probe.

- The decoded header's packing mode is zero.
- The packed payload is the plaintext without compression.
- Packed and decoded lengths are identical.
- Packed and decoded checksums are identical.
- The existing 250-word generator applies the outer XOR transform.
- By default, the seed is the first four SHA-256 bytes interpreted as a
  little-endian integer. The engine uses the header value only to reconstruct
  the stream and does not require it to represent the current tick count.
- Identical input produces identical output, making patch artifacts
  reproducible. `--seed` permits an explicit alternative.

Every generated container is decoded in memory and compared byte-for-byte with
its input before it can be written.

## Confirmed engine acceptance

Run this from a normal graphical shell:

```bash
./tools/cfg-writer-experiment.py
```

The experiment adds only a comment marker, generates a mode-0 encrypted
`tables.cfg`, launches for 20 seconds, and checks whether startup materialized
the marker in plaintext. It restores the exact original plaintext and
encrypted runtime files even if launch fails. A new evidence directory is
created under `working/experiments/cfg-writer/`, so existing evidence is not
overwritten.

The probe succeeded on 2026-10-02. Evidence is retained at
`working/experiments/cfg-writer/run-20261002T044257Z-ygqfjl4h/`.

- Chaos.exe accepted the 942-byte generated mode-0 container.
- A clean shutdown removed the temporary plaintext file.
- The game repacked the configuration into an encrypted container.
- Decoding that post-launch container recovered the unique comment marker.
- The probe restored both runtime files afterward.

This confirms the whole data-mod pipeline: plaintext input, deterministic
mode-0 packaging, engine decode, engine load/shutdown, engine repack, and
independent decode of the result.
