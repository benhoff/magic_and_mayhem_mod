# Native World history timeline v1

This JSON is an offline native policy input to `mnm-world-history-preview`.
It groups closed World v1 request packets into an explicitly ordered native
canvas history. It does not recover an original allocation or initialization.
Original before/after pixels and raw host pointers are not accepted fields.

The top-level object has exactly `version: 1`, `native_clear_word: 0..65535`,
and `frames`, an array of 1..256 rows. Each row has exactly:

- `canvas`: a nonzero caller-owned logical identity, an unsigned 32-bit integer.
- `queue`: a nonzero source sequence, an unsigned 32-bit integer. This is the
  actual source order supplied by the caller, independent of World packet sample
  or channel publication counters.
- `reset`: Boolean. A reset begins from the explicitly selected native clear
  word. It cannot stand in for an unknown original initialization.
- `snapshot`: a relative path resolved beneath the timeline directory; absolute
  paths and escapes through parent traversal or symlinks are refused.
- `sha256`: the lowercase SHA-256 of that closed World v1 request packet.

The first frame must reset. Continuation requires the same canvas identity and
the immediately next source sequence. A repeated/regressed sequence on the same
canvas is refused even with reset. A new identity or a forward gap requires an
explicit native reset. All frames have the same dimensions. A resize requires
a new service with an explicitly owned background.

Packets total at most 256 MiB. Each packet is admitted by the World v1 decoder
and every frame must resolve through pinned native SPR bindings. Metadata and
resource resolution precede output creation. The tool writes tight native
RGB565 canvases and a report into a new directory outside the asset and timeline
roots. Export performs diagnostic native readbacks; this tool is not an
interactive presentation path.

Example:

```json
{
  "version": 1,
  "native_clear_word": 0,
  "frames": [
    {"canvas": 1, "queue": 1, "reset": true, "snapshot": "1.bin", "sha256": "<packet SHA-256>"},
    {"canvas": 1, "queue": 2, "reset": false, "snapshot": "2.bin", "sha256": "<packet SHA-256>"}
  ]
}
```
