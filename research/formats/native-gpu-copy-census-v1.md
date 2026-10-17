# Native GPU-copy diagnostic files v1

These are opt-in observation files, not rendering inputs or a game CPU-history
transfer. `MNM_NATIVE_COPY_DIAGNOSTICS` enables the PE32 census and names the native
host's diagnostic directory. Ordinary launches create neither. The caller census
has no per-copy file I/O; a bounded file mapping holds scalar rows until process
detach. Validate only after process retirement, alongside successful native END
and the independently checked game flow. A closed census alone is not success.

## `lock-capture/gpu-copy-census-v1.bin`

Little-endian DWORDs, total 1,048,704 bytes: 128-byte header followed by 8,192
128-byte rows. `native_gpu_copy_census_v1.h` asserts the layout on C/C++ hosts.

| Header DWORD | Meaning |
| --- | --- |
| 0–1 | ASCII `MNMGCN01` |
| 2–5 | Version 1, header bytes 128, row bytes 128, capacity 8192 |
| 6–9 | Published rows, overflow/exhaustion flag, total completed observations, dropped rows |
| 10–13 | Clipper-attached, self/alias, keyed, bootstrap counts |
| 14–17 | Admitted zero-piece plans, negative application results, error prefixes with writes, negative service admissions |
| 18–20 | Observed main-module base, PE image extent, closed flag (1 after detach) |
| 21–31 | Reserved zero |

All counters are bounded unsigned DWORDs. Total-counter exhaustion marks failure
without wrapping. Capacity overflow retains the earlier rows and records drops;
it does not change graphics admission. The validator refuses overflow, drops,
unclosed files, counter disagreement and nonzero unused rows.

| Row DWORD | Meaning |
| --- | --- |
| 0–3 | Ordinal, actual return PC, observed module base, RVA or `0xffffffff` for an external/unknown caller |
| 4–6 | Source, destination, temporary native command resource IDs |
| 7–9 | First/last COPY sequence (zero for no writes), session epoch |
| 10–11 | Opaque process-local source/destination object tokens; never dereferenced by the reader |
| 12–14 | Source generation, destination generation before/after |
| 15 | Bits: 1 physical self/alias, 2 destination clipper attached, 4 source key, 8 full opaque bootstrap |
| 16–18 | Plan piece count, application HRESULT, service admission HRESULT |
| 19–22 | Source width/height, destination width/height |
| 23–26 | Requested source rectangle, signed DWORD coordinates |
| 27–30 | Requested destination rectangle, signed DWORD coordinates |
| 31 | Destination GPU-authority flag after the operation |

The clipper bit records attachment, without asserting a nontrivial clipped
footprint. Self identity refers to physical owned storage, including distinct COM
interfaces. For self-copies the sequence range includes the full opaque snapshot
COPY and all successful pieces; temporary CREATE/DELETE are outside that range.
Other calls contain exactly their successful pieces. Error prefixes preserve
their writes and GPU authority. Service refusals have no admitted plan.

The module base/extent come from the observed process image. Never reuse object
tokens, process pointers or absolute module addresses across launches. Interpret
an RVA as original code only with the independently pinned executable build.

## Native host `copy-diagnostics/`

`copies.jsonl` records each checked COPY sequence, its 13-word scalar command
storage (ten COPY fields, three zero tail words), dimensions, complete destination
pixel/mismatch counts and SHA-256 hashes of source/before/actual/expected words.
Selection conservatively follows resident World ancestry through copies/swaps;
CPU overwrites may leave extra eligible cases. The PE32 census identifies actual
game GPU-route calls within that larger set.

Every eligible COPY gets independent CPU transformation and complete destination
comparison, bounded at 8,192 checks and 32 GiB total diagnostic transfer. The
budget is checked before readback. The first 16 distinct command shapes retain
four relative leaf files; a mismatch also retains its cohort and terminates the
diagnostic session. Each `.u32` contains top-to-bottom, unpadded little-endian
32-bit native pixel values, with dimensions in its JSON row. These are not RGBA
images. Expected bytes are computed after the GPU output is captured and never
fed back to rendering or the facade.

`report.json` records checked/seen copies, compared words, diagnostic readback
counts/bytes, retained cohorts, END, truncation and errors. It succeeds only after
END with at least one checked copy and no pending, truncated or failed comparison.
The recorder refuses an existing JSONL file. Ordinary source-provider readbacks,
explicit producer CPU reads and these host diagnostic reads remain separate.

`tools/native_gpu_copy_diagnostics.py` checks census completeness and counters,
joins each game GPU-copy range to its actual native records, verifies aliased
snapshot identities and independently recomputes every retained destination in
Python. Equal but wrong expected/actual files still fail against source/before
inputs. Native pixels are not an independent original World or physical display
oracle; those comparisons require their own evidence.
