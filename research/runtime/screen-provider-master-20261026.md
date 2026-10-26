# Screen provider recovery: local master integration

This integration brings `4061ce9a` into the current local master while preserving
independent coverage entries, exact historical receipts and pending main-checkout
work. It introduces no live replacement or gameplay changes.

The provider engine sources match the recovery commit. Master's shared evidence
reader/writer changed, so the original provider and caller fingerprints remain
historical. Fresh `SP26.master-original-20261026`, `SP26.master-native-20261026`
and `RSA26.master-regression-20261026` results record execution with the current
helper rather than editing the old evidence. The provider run passes 2,551
schedules in each optimized original32, sanitized32 and native64 configuration,
325 constructor-prefix schedules and 16 populated-resource screen journeys with
zero allocations left at session exit. Native owner and resource-manager/
preparation regressions pass. The caller regression passes 768 schedules and
192 controlled lifecycle compositions per configuration. Original manifests
verify before and after. Scope and native safety policies remain as documented
in [the provider recovery](screen-providers-and-failures-20261026.md).

The combined dashboard checks exposed one inherited planning-review omission:
`NP.native-only-launch-contract` already declared the combined Qt-lighting
runner, but its planning review omitted that runner's source fingerprint. The
runner was reviewed and added to that static source list. Existing review hashes,
execution evidence and status credit are retained. The fixed expected-unrated
list also includes the new provider contracts and already committed alternate
canvas contracts; no importance weight is assigned. Failed planning runs remain
preserved alongside the corrected result.

Generated records are immutable SQLite references under
`screen-provider-master-results-20261026/`. Review and staging use bounded private
RAM. Verified local SQLite backups and the configured independent snapshot
stores remain authoritative. The normal 90 GiB main-drive and 100 GiB archive
reserves apply. No publication or private remote transfer accompanies this merge.

The next original-behavior deliverable is real screen lifecycle recovery:
World construction and Main/Realm/World/Report transition routing. It should
produce a route-driven round trip using populated resources, replacing the
fixture's manual entry/exit ordering. Windows SEH delivery, physical drivers/
codec DLLs and installed widget/GL acceptance remain separate validations.

The exact accounted-history review covers 22 intervening/incoming commits and
2,610 code/control/research Markdown+JSON transitions, with no missing receipts
(`history02.json`). The earlier broad diagnostic (`history01.json`) also listed
routine raw logs and binary/archive attachments outside the receipt gate and
three historical raw attachments unavailable through that resolver. Its failed
result is preserved; the focused review does not claim complete repository-wide
payload availability or a historical gate pass.

Final reconciliation retains validated candidate `75f441d2` and its normal staged pass/verified full shared-store backup. Independently committed master advances were reconciled after their reservations ended. The new reservation preserves all eight pending paths with verified SQLite snapshots and a pinned stash. All three fresh screen proof source/contract bindings remain exact across that reconciliation. The final merge reuses that execution without asserting new original or live work. A verified read-only RAM cache supplies existing metadata; an invocation-local writer synchronizes every new payload into the single persistent shared store before staging and receives ordinary verified backups through the configured dual snapshot stores.
