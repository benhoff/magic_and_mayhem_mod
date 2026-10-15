# Selected Wine submission generations V2

V2usesmagic`MNMWIN02`,version2,andtheexisting64-byteheader/192-byterows.
It retainsV1words0..26andaddsword27first-objectobservedgeneration,
word28second-objectgeneration,andword29exactlifetimejournalrecordprefix.
Words30..47remainzero.V1words27..47remainreservedzero;historicalV1records
retaintheiroriginalmeaningandrequire no lifecyclefile.

Capture these three fields under the same recorder lock as submission entry.
Preserve them through the paired original return. Decode V2 only alongside the
closed selected-lifetime journal; replay observed create/retire transitions to
word29and require exact generation agreement. Zero generation explicitly denotes
an object without an observed live creation at that prefix.

This is generation metadata at a submission point. It does not freeze texture
bytes,proveallinternalWinealiases/lifetimes,bindfullproducerinputsorassignan
asynchronousEGLphysicalframe. Allownershipandcompletejourneyflagsremainfalse.
