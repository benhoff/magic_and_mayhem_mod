# COPY return order and driver command order

## Confirmed failure and fix

The retained 238-frame storage session originally fails ten presentations.
Nine failures disappear when replay follows independently observed driver COPY
order and binds each source read to that command, rather than to wrapper return
order. This is a trace interpretation fix; original journals and encoded assets
remain immutable. Original CPU/GPU/destination pixels never select the ordering.

In the captured loading interval, producer49745 belongs to API4129/driver1164
(whole canvas81←82), whereas producer49744 belongs to API4130/driver1166
(save background85←81). The earlier driver command's wrapper returns later on
another thread. Replaying wrapper returns saves stale background. Likewise,
producer49748/API4131/driver1167 precedes49746/API4132/driver1169.
These are confirmed identities in the retained API points, worker packet and
source-submission journals; they are not inferred from matching cursor shapes.

`tools/drawing_copy_order.py` matches a COPY uniquely by logical source and
destination identities, the API completion producer count and the next API entry
on that same thread. It additionally requires exact driver source/destination
boxes, supported opaque/keyed flags, zero effects/subresources and filter1.
CPU-only or unsupported copies remain barriers. Ambiguous API binding refuses.
Only runs of eligible COPYs can move; recorder-only checkpoint10 is ignored and
all other operations are barriers. Within those runs, reads use their exact
API/worker command and the prefix of driver copies completed before that read.

The replay-only wire replaces fixed 96-byte COPY headers and renumbers their
native sequence slots. Each moved record and rebound read retains its original
sequence in the normalization report, with input and normalized SHA-256 hashes.
Payload operations are never reordered. No global ordering, alias ownership or
unobserved driver route is established by this bounded normalization.

## Results and boundaries

The complete retained storage replay now compares all238frames and fails only
ordinal170, with126 differing pixels in each native-GPU and physical cohort.
The323frame Quick Battle regression remains passing. Six focused tests check
ordering, command-bound reads, missing read identities, unsupported barriers,
ambiguous bindings and independence from oracle pixels.

Fresh original source-GPU diagnostics isolate the remaining first-downsize
mismatch after source rendering: native output equals the original final source
texture across all228frames, but one physical frame still differs. Completion,
correct pre/post EGL dimensions and correct native X11 geometry do not eliminate
that difference. See [the presentation diagnostic](drawing-egl-swap-completion.md)
for exact scope and subsequent policy experiments. The historical ten-failure
reports remain unchanged; no failing frame is omitted or accepted by tolerance.
