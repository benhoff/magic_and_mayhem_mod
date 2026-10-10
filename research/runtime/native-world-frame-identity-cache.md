# Native World frame identity cache

World preparation used to normalize and SHA-256 hash an encoded sprite once for
catalogue lookup and again for binding on every draw. The native consumer now
shares validated identities through an exact indexed-tag/owned-byte LRU. This is
an intentional native preparation policy, separate from recovered raster behavior.

`SnapshotFrameIdentity` has no arbitrary digest constructor/setter: construction
uses the existing frame extent validation and normalized hash. Cached keys own
copied bytes, including input supplied through `QByteArray::fromRawData`. The cache
admits at most 4,096 identities and 16 MiB of encoded allocation capacity; object,
map/list and token overhead are additional. Oversized valid frames bypass caching,
malformed frames cannot enter it, and token values survive eviction. Primitives
keep their positions among sprites. Binding still loads the selected resource and
checks palette ambiguity and decoded pixels after resource revision changes.
Unmapped and rejected inputs never become successful bindings.

Validation is scoped to the headless native consumer and its complete actual
compiler/build/test dependency closure. Synthetic tests cover borrowed-buffer
mutation, indexed-tag separation, malformed extent, count/byte bounds, LRU eviction,
oversized bypass, token lifetime, mixed primitive order, unmapped retries,
resource reload/adoption and palette refusal. The reproducible replay compares
all checkpoints from the same owned default V1 first-16 map-2/zero-items inputs
with the prechange consumer. Both consumers read only owned stream/assets, not
original canvas oracles. Debug/software GL preparation timings exclude original
simulation and pre-entry work and do not establish hardware-general frame rate.

`tools/test-world-resource-reuse.py --prepared-inputs <prior-run>` checks every
owned asset against the prior input manifest and checks stream/done bytes against
the same captured fixture. It does not execute the capture decoder. A prior run's
input provenance can remain valid when its code validation becomes stale; this
mode verifies actual input hashes before using it. Optional `--prepared <build-run>`
reuses a build only when all actual compiler dependencies match the current
prospective source declaration. A new declaration and execution are always made.

[Current native replay](native-world-frame-identity-cache-native-20261009.json)
passed five native tests and all 1,058 checkpoints against the prechange
consumer, with stable source/input hashes and complete compiler dependency review.
Warm median preparation fell from 116.365 ms to 7.185 ms (93.83%).
There were 274 successful hashes and 18,188 hits;
the final queue had 0 hashes and 1,154 hits. The cache held
274 identities/516,720 encoded-capacity bytes, with no eviction
or oversized bypass in this fixture. Eviction/bypass/refusal cases pass separately
in the synthetic tests. GPU timings fluctuate and establish no GPU improvement.
This promotes scoped native implementation and headless integration only. Public hook lifecycle, precise original
intermediate/AX comparison and live replacement use separate complete pipeline
claims. Combined-workspace observations retain their own source versions and do not
establish current validation of this isolated commit or of new minimap branches. No gameplay balance or original simulation is changed.
