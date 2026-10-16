# Native campaign arena integration

This is a bounded authored campaign over the existing native skirmish arena.
Two declared missions reuse its frozen navigation and ordinary-health backend;
they are not original campaign maps or recovered victory/reward rules. Both
retain existing arena damage, mana, AI and spell policies.

`apps/skirmish/campaign.cpp` owns the content/resource adapter. Its content
revision fingerprints canonical initial campaign data, ordered mission IDs,
the explicit linear-policy version, and the entire initial native arena
snapshot. That snapshot includes map/profile/navigation identity, arena rules
hash and health backend identity. The 64-bit FNV binding detects consistency
changes; it is not authentication. Semantic policy changes require a version
bump even if the data stays identical.

Fresh creation and restoration use the same application-owned navigation and
ordinary-health interfaces. Real skirmish constructors validate navigation,
saved combat/backend identity and all existing motion/combat/arena state.
The campaign coordinator also requires the reconstructed world to encode
exactly to the original bytes before adopting either owner.

The command-line entry point exposes create, inspect, start, semantic bolt and
summon orders, and bounded tick continuation. Orders use the actual wizard
handles internally. Save destinations must be new; input files remain intact.
JSON reports expose the persisted last result separately from the number of
new settlements emitted by this command. Reopening an idle result yields zero
new settlements. The [campaign window](native-campaign-window.md) is a separate
consumer of this adapter. Arbitrary manifest import and installed original assets
remain outside its scope.

The independent-process journey exercises a pending summon, launched bolts,
restored resource/backend bindings, byte-identical split continuation,
victory selecting the next region, defeat/reopen without advancement, fresh
retry identity and final completion. It independently walks version-1 and
version-2 campaign wire fields. Checksummed navigation/health/content/participant
and result-order/ownership counterexamples are rejected, as are duplicate
start, terminal restart, overwrite, malformed tick and corrupt snapshot inputs.
The ordinary native arena contract/process/window and full native game suites
remain regression checks; no original campaign equivalence is inferred.

Run `tools/test-native-campaign-session.py --arena` through the main checkout's
storage guard. It retains the journey's commands, stdout/stderr, checkpoints,
their hashes and report under the pinned run's `journey/` directory.
