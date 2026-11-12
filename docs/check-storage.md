# Campaign check storage

Campaign runs default to a bounded **diagnostic** mode: 120 seconds plus process
cleanup grace, semantic/original observer records and normal source/resource
checks, with no viewport, command-history or World-source capture. The report
kind is `public_native_campaign_lightweight_diagnostic`; it cannot pass either
rendering comparison tool. A successful checkpoint diagnostic can still supply
an exact original save and selected-field restore proof to the byte-copy tool.
This does not establish full saved simulation or native parsing.

Use `--evidence-mode full` explicitly for rendering evidence. Full mode retains
the existing lossless capture formats, limits and independent pixel checks; its
default deadline remains 900 seconds. `--deadline-seconds` accepts 15 through
900. Preparation's generated acceptance commands include the explicit full
option. Existing historical full acquisitions remain readable without changing
their recorded hashes. The twelve-case acceptance matrix still requires full
captures and its existing assertions.

## Share inputs, isolate writes

In a checkout with verified SQLite evidence and the original-media mapping,
choose an existing owned source checkout containing a physical pristine No-CD
installation, prepared CD tracks and a stopped admitted prefix:

```sh
SOURCE_CHECKOUT=/path/to/existing/checkout
python3 tools/prepare-check-inputs.py --from-checkout "$SOURCE_CHECKOUT" \
  --catalog-only \
  --report research/runtime/check-storage-results-20261103/inputs-LOCAL.json
python3 tools/prepare-rendering-progression.py \
  --report research/runtime/rendering-progression-results-20261101/inputs-LOCAL.json
```

The project owns one fingerprinted set in the main checkout's ignored
`working/check-inputs/`. Each consumer has a small SQLite catalog. Identical
provisioning reuses existing copies. Catalog-only setup preserves existing local
installations and saves; fresh checkouts can omit that option to bind input
aliases. Campaign runs route game and music sources explicitly from the verified
catalog and borrow the canonical prefix through the existing exclusive two-slot
pool. Returned prefix changes remain retained in SQLite before reset/reuse.

Source files are never writable game instances. The launcher resolves a source
alias before copying and rejects a runtime that aliases its source. Original
patching, preferences and saves operate in private execution trees. Catalog
hash/membership drift, conflicting aliases and occupied prefix slots refuse
reuse. Neither pristine sources nor canonical/held prefixes are cleanup targets.

## Short checkpoint checks

Import the existing exact checkpoint using `tools/stage-campaign-checkpoint.py`
as described in [checkpoint preparation](../research/runtime/campaign-checkpoints-20261103.md).
Use distinct report names:

```sh
python3 tools/run-rendering-progression.py \
  --case victory --checkpoint restore --checkpoint-name checkpoint-camulos-05 \
  --checkpoint-reference research/runtime/rendering-progression-results-20261101/checkpoint-seeded-software-low-05.json \
  --preparation research/runtime/rendering-progression-results-20261101/inputs-LOCAL.json \
  --report research/runtime/rendering-progression-results-20261101/light-LOCAL.json \
  --software --size 640x480 --skip-intros --observe-input-admission
```

For a full independent comparison, repeat with `--evidence-mode full`, a new
acquisition name and then `tools/check-campaign-checkpoint.py`. Lightweight
reports are refused before the original pixel oracle starts. Source changes
require a new preparation; importing an old checkpoint does not refresh any
historical execution fingerprint or count as fresh campaign progression.

## Completed output handoff

The progression and checkpoint comparison tools own their subprocess groups
through completion and timeouts. After retaining every output artifact they
record stopped-producer proof, exact output membership and source fingerprints,
then queue a comparison handoff. The Wine handoff still requires its own stopped
Wine and returned-prefix proof. A comparison does not invent a Wine-stop field.

`tools/finish-work.py` handles both queues. It checks current consumers, stores
every member and original pin in both independent stores, performs an exact
restore, and only then replaces the physical tree with a pinned restore stub.
Launch itself does not delete output. Failed runs and failed comparisons retain
their results. Future physical consumers must restore their historical paths:

```sh
python3 tools/working-retention.py --root /historical/checkout \
  restore /historical/checkout/working/pilot/comparison-verification
```

Historical cleanup uses the same verified handoff after an explicit dependency
and activity review. It preserves source checkouts, current inputs, historical
report hashes, archive catalogs and the newest generic runs. Reclaimed allocated
bytes are reported separately from filesystem free-space changes and growth of
the shared evidence pair. The 90/100 GiB filesystem reserves remain unchanged.
