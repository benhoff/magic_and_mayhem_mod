# Owned terrain lighting resources

This is an intentional native policy. `TerrainLightField` owns five buffers and
18 kernel slots through vectors. Construction releases earlier allocations if a
later allocation fails. Replay restoration validates all buffer lengths and both
subcycle state domains before committing a candidate. Stepping also commits a
candidate, preserving the previous state if copying or validation fails.

`terrain-light-lifetime-test.cpp` injects failure at each allocation position in
construction, restore and step for three dimensions. It checks outstanding
allocations and the exact previous buffer/state fingerprint, then executes the
first successful position. It separately refuses each malformed buffer extent
and out-of-domain creature/static phase and cursor. Native and ASan/UBSan runs
exercise the same cases. This does not change the separately recovered original
initializer fault behavior and does not prove original allocator internals.
