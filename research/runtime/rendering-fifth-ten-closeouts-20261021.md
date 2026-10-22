# Fifth native rendering batch — bounded adapters (2026-10-21)

Ten partially implemented adapter tasks are completed within their existing native scopes. Legacy and general bounded read mapping form one task, represented by two stable behavior IDs. The submission parser is a shared dependency and remains partial. All comparison, integration and replacement statuses remain independent.

| Task | Completion and executable controls |
| --- | --- |
| Texture-entry hooks | Null/overflow RVA refusal before mutation, exact prologue validation, owned create/release trampolines and address reuse. |
| Worker leases | Nonzero API/lease identities, exact early/delayed completion, missing/unknown/timeout refusal, inclusive 16,384-row and 2,048-command caps. |
| Storage candidates | Two free virtual regions cannot be classified as one allocation; require nonzero allocation base and committed/reserved regions. Live/reserved metadata remains a candidate, not ownership. |
| Resource-copy forwarding | Unknown resources retain zero associations even with stale empty slots; balanced parent depth, exact original forwarding and FP/error preservation. |
| Cursor metadata | Cross-DLL original callback/journal joins; mixed-protection output crossing pages refuses atomically, then accepts once writable. |
| Source descriptors | Exact lifecycle-prefix lookup using per-object transition indices; create/retire boundaries and full analysis with thousands of events under a 16 MiB traced-memory cap. |
| Read mapping (both modes) | Reject explicit empty/malformed/overflowing environment limits; validate dense owned payloads, exact forwarding, attempt/byte quotas, fatal unmap/reentry before forwarding. |
| Source write tracing | Strict armed/done phase parsing; actual foreign writes, partial aliases and page halos; shared second mapping remains explicit negative ownership evidence. |
| GPU fence adapters | Refuse null packet/zero worker before access; strict paired command/submission IDs, signal/no-context/failure/default-off/postclose/midwait responses. GL procedures are owned stand-ins. |
| Semantic state sampling | Require the actual nonzero menu owner thread before clock/storage reads; enforce cadence and retain read-only scalar protocol. |

`tools/test-rendering-fifth-ten.py` prospectively pins sources and claims, builds production drawing/scene/menu observers, compiles actual PE32 fixtures and runs them serially in an exclusively locked, marked reusable Wine prefix. Compiler dependency files must match the declaration. Python controls exercise malformed journals and the full descriptor/submission analysis pipeline. Reports are immutable SQLite objects; failed attempts and raw binaries/journals stay under pinned `working/tests/rendering-fifth-ten/`.

No original input or original executable is consumed. This closes bounded native helper implementation only. Real-game/driver installation, all internal paths and allocation lifetimes, alternate backing ownership, hardware fence completion, unobtrusive live operation and semantic/gameplay parity remain pending. In particular another virtual mapping can still write underlying source bytes; observed region identity does not establish immutable ownership.

The [successful combined execution](rendering-fifth-ten-results-20261021/controls-03.json) passes 90 build/execution/retirement commands with 284 stable prospectively declared inputs, 88 Python tests, 15 actual Win32 environment-limit cases, eight worker cases, both map modes plus two fatal-map guards, 12 write-trace cases and 16 GPU adapter cases on wine-11.16. Eleven helper implementation statuses have current source-bound execution proof. Two failed runner/fixture integration attempts remain immutable SQLite results (`controls-01.json` and `controls-02.json`); neither is claimed as passing. The GPU-only fixture explicitly counts its ownership hand-off stub; enabled ownership enforcement remains covered by its separate physical-owner suite.
