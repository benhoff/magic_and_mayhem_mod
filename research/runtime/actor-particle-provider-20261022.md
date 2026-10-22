# Actor emitter construction and ownership (2026-10-22)

Pinned NoCD build40209ca7, original493f20→597850(100)→546c20→5457e0,
with546cb0→546cd0→597870 on rejected or retired allocations. The unchanged
particle constructor kernel now has a PE32 value-address adapter; it reads and
writes exactly100 bytes, loads the original world and emitter period globals,
and preserves original wrap/clamp and owner customization. Manager5458b0
publishes cleared slots in original order and resets count/hint after callbacks.

Independently authored initialized inputs cover constructor byte preservation,
positive varying extents, negative and oversized coordinates, signed period
bits, deleting flags0–3, disabled/failing/successful actor allocation, optional
valid/null owner lookup, empty/capacity-minus-one/full manager prefixes,
rejection, distinct retained generations, and repeated clear. Prefixes have
initialized null holes, explicitly supported by original clear; this does not
claim1792 concurrently active emitters or updater/emission validation. Each
owned allocation has red-zone canaries, and teardown must reach zero live
allocations through real modeled manager/deleting routes, without fallback frees.

Fresh `particle-matrix-2.json` passes1024 cases and2048 before/after teardown snapshots per build. Each original32/native64/ASan+UBSan stream compares 1124887136 bytes; SHA-256`77d677005480094704cf53e079e6bfafcbab2148f8e6e4b1a4bddaa3ee7c2f5d`. All81 declared sources and compiler dependencies are stable, protected original bodies and original input manifests remain unchanged, canaries hold and no owned allocations remain. Per256 actor cases, outcomes are `{"admitted": 107, "enabled": 192, "failed": 32, "rejected": 53}`; disabled cases are the remaining actor inputs. Confidence is high within this bounded normal-return domain.

Actual emitter update/emission cadence, original planner/world production,
remaining special particle/task/scenario providers, malformed inputs, throwing
Windows SEH, platform and installed/live acceptance remain separate. The
Critical actor downstream umbrella remains partial. Existing isolated variant
evidence and its hashes remain unchanged.
