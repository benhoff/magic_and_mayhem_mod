# Bounded native panel domain and atomic admission

The No-CD `58ddf0` bevel, `58d8c0` highlight and `58e2c0` whitewash are
destination-dependent writers. Existing closed producer replay reproduced them
in the observed startup journey, with isolated fixtures covering one rectangle.
This extension compares small/degenerate rectangles, asymmetric overlapping
corners, edge rectangles, percentage extremes, both boolean branches, and padded
storage. It keeps the recovered pixel rules distinct from native admission.

Whitewash uses a half-open rectangle and can reach `right=width,bottom=height`;
highlight owns the extra asymmetric edge corners at `right,bottom`. Bevel visits
two rows and columns on each side, including `right+1,bottom+1`. Repeated visits
must apply the transform repeatedly in the original order.

Before this change native panel replay could mutate early pixels before finding
an undefined late destination. Its complete ordered footprint is now validated
before mutation. Late-undefined tests repair the missing source pixel only after
refusal and verify every earlier pixel remains unchanged. This is an intentional
native admission rule; the original routine assumes its locked pixels exist.

RGB565 pixels are compared against unchanged original routines in a private PE
mapping with explicit external Lock/Unlock success adapters. Full physical
planes and guards are retained; native logical storage projects through the
captured stride and preserves padding. This does not establish panel caller/FX
state, live suppression, clipping with nonzero wrapper origins, RGB555 panels,
allocation/restore failures or whole-renderer replacement. Those remain separate
from the independently installed native fade splice.

The [current comparison](panel-domain-comparison-20261010.json) passes 7,490
cases / 2,160,320 full physical WORDs: 6,720 bevel, 336 highlight and 434 whitewash
cases, with padded rows, corner overlap and all 65,536 initial whitewash values.
Nineteen native refusals include late undefined destinations, bounds, mode and
percentage guards, with unchanged earlier pixels and sanitizer checks.
