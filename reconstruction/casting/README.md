# Recovered casting contracts

Build-specific C++17 projections, independent of Qt, Wine and host pointers:

- `descriptor.*`: packed cast-command decoding.
- `admission.*`: cast admission and compressed dispatch with supplied lower services.
- `cure_effect.*`: spell41's selected effect-dispatch arm, including deferred
  delivery, failure/refund and visible feedback with explicit lower services.

The Cure library has a standalone build:

```sh
python3 tools/working-retention.py run --reserve-gib 0.25 -- \
  cmake -S reconstruction/casting/cure -B working/build/cure-effect
python3 tools/working-retention.py run --reserve-gib 0.25 -- \
  cmake --build working/build/cure-effect -j2
ctest --test-dir working/build/cure-effect --output-on-failure
```

For a source/scope-bound original comparison, first use
`tools/draft-coverage-claims.py` with behaviors `CAST.cure-effect`,
`NP.cure-effect-admission` and `SP.041`, plus scenario `CURE01.effect-dispatch`.
Then run `tools/test-cure-effect.py --claims DECLARATION --output NEW_DIRECTORY`
under the storage guard with a 0.5 GiB reservation. The reference requires
Linux/i386 execution and a 32-bit C++ toolchain. Original inputs stay read-only;
the runner verifies their manifest before and after execution.

The [Cure research](../../research/runtime/cure-effect.md) owns the precise
comparison domain and remaining lower services. These libraries do not yet
compose a complete native spell lifecycle or replace original campaign work.
