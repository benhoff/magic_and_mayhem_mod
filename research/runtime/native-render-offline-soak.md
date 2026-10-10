# Offline native rendering soak and adversarial replay

This native policy tests retained renderer lifetime without launching gameplay.
It does not implement rolling World delivery or establish live takeover,
simulation cadence, physical input latency or a new original-engine comparison.

`tools/test-native-render-soak.py --corpus <successful-world-resource-reuse-run>`
builds a frozen RelWithDebInfo consumer and keeps one GlBlitter alive while
replaying complete owned 32-queue sessions. Each session constructs and retires
its producer, World resource bindings and GPU scene. All saved checkpoint
identities and RGB565 digests must match; World completions independently compare
GPU pixels against native CPU composition. Saved output hashes are assertions,
never native drawing inputs. Every session must retire all surface handles and
owned surface pixels. Source, asset, stream, marker and compiler dependency
fingerprints remain explicit. Immutable originals are verified before and after.

The default experiment runs20 sessions and40 seeded adversarial rounds. The
scalar RGB565 oracle covers ordered overlapping copies, three blend policies,
absolute-row displacement, additive channel wrap/saturation, palette ownership,
source mutation, signed-anchor clipping and fully hidden sprites. More than64
distinct palettes and more than512 opaque draws per batch exercise flush limits.
Undefined destination sampling, alias admission, invalidation and exceptions
must preserve refusal, ordering and caller GL context. Sixteen envelope mutations
exercise fail-closed header admission. Existing assertion-enabled cache/atlas
eviction and malformed batch fixtures run separately in Debug.

Budgets are declared before execution: warm native-work median50ms, p95 100ms,
maximum250ms, and at most32MiB post-warm session resident-memory growth. The first
session warms the allocator/driver; each session's first queue is reported
separately as cold resource upload. Native work includes CPU composition,
history adoption, resource preparation, GPU submission, readback and comparison.
It excludes checkpoint digest cost, transport, simulation and desktop display.
These host-specific offline limits are not a full-game FPS claim. Memory samples
at completed session teardown test retained growth, not peak allocation between
samples. Explicit surface/scratch/cache bounds supplement resident memory.

Use `--gl33` for the framebuffer snapshot fallback and `--seed` for independent
deterministic draw sequences. Raw logs, frozen sources and per-queue/session
reports are retained under `working/tests/native-render-soak/`. The [default result](native-render-offline-soak-default-20261010.json) passes
20 sessions:640 native CPU/GPU World completions,24,480 checkpoint hashes,28,000
seeded draws,360 scalar pixel comparisons,120 runtime refusals,16 malformed
envelopes and three assertion-enabled Debug fixtures. Warm native work is
39.22ms median,72.71ms p95 and116.15ms maximum; post-warm resident growth is
10.87MiB and plateaus at327.6MiB. Surface teardown and scratch storage are stable.
Cold first queues measure444.20ms median and471.27ms maximum, separately from
warm latency. GL3.3 fallback and deliberate budget refusal are next checks.

The committed-history review for01e160a..0415793 inspected50 file transitions,
matched each parent's and commit's SHA-256 to existing exact receipts, and found
no unresolved gaps. The review journal itself is excluded from its recursive
accounting. This review establishes accounting, not retrospective test execution.
