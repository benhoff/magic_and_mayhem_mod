# Installed encounter support (NS67)

Pinned NoCD40209ca7 provides three bounded contracts needed for an installed
crocodile encounter. CREATURE_17 is width2,height2,SwimmingAbility3,Acceleration10,
Health360 and IsPhysicalTRUE. NS66's one-cell provider fixture did not instantiate
that configuration; real installed construction remains a separate milestone.

50e400 uses wrapped source-minus-target XY with the original signed half-map ties.
A2x2 source indexes three25-DWORD tables at5e51e4/5e5248/5e52ac. Target width selects
the small/large animation base; odd facing falls back to24 except explicit24.
Without a creature target, only a route-cell object of class26 admits the object
facing table. Its coordinates come from the route cell, not the object record.
The initial negative fixture used object coordinates with an unset route cell;
that error is retained and the corrected fixture supplies the actual query cell.
Camera turn and reverse flags transform facing before optional visual/attachment/
secondary callbacks and the final ANI start. Native providers support ordinary
creature targets; special object attachments remain explicit refused boundaries.

4f3260 samples NW,NE,SW,SE. It uses unsigned classification comparisons against
an initial16 ceiling and returns immediately for absent kind or a zero result.
Native geometry samples wrapped cells separately and calls this recovered helper;
this comparison covers the helper, not original out-of-map pointer behavior.
510480 returns true only for zero health and either corpse flag.

The fixture protects the three actual bodies and all75 table DWORDs, compares
full declared actor changes and ordered controller/visual endpoints, checks
configuration/object inputs remain unchanged, and guards the original text and
tables after every case. Control/World lifetime, installed encounter composition
and live replacement stay separate from the primitive comparison.
