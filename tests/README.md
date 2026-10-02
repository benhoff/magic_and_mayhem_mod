# Tests

Store automated checks and repeatable manual test protocols here. Each gameplay
experiment should state its clean baseline, single variable under test, expected
observation, and rollback procedure.

`./tests/test-tooling.sh` includes baseline integrity checks and the readable
route-request model. Run `./tests/test-reconstruction.sh` alone for the C++17
model with address/undefined-behavior sanitizers, or
`python3 tests/test-decompilation-baseline.py` for checksum/PE-byte verification
and tamper rejection. These are host/static tests, not live-game equivalence.

`python3 tests/test-route-trace.py` builds the trace replay and checks synthetic
zero/nonzero route counts, normalized arguments, full snapshot comparison,
pointer/flag mismatch detection, malformed data and architecture fail-closed
behavior. It is included in the tooling suite. For opt-in live capture and its
current ARM64 Wine limitation, see the
[route capture workflow](../reconstruction/pathfinding/README.md#route-capture-and-replay).
