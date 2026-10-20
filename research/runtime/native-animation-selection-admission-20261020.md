# Native owned animation selection admission — 2026-10-20

This is intentional native safety and ownership policy. It is separate from
original controller/descriptor equivalence and from gameplay/effect admission.

Forward start and dispatch reject a selected negative SPR index before exposing
that record through the owned display snapshot. Sprite lookup already rejected
negative values; the earlier start/tick path could nevertheless publish them
through displayedRecord. The new regression failed against the earlier code.
A raw player dispatch exception can retain intermediate control state; callers
must stop/restart or discard that failed continuation, as already documented.

The body descriptor implements the recovered absent-display sequence288+facing
and inclusive SPR-bound fallback to frame0. Its source callback returns owned
records, and candidate initialization/descriptor validation precede replacement
of the previous player. Unknown facing, missing source, source error, empty
visible result and negative selected record refuse without replacing the player.
An existing display uses neither the facing nor the source callback.

ModeOneAttachment owns the selected mode0/mode1 state and a copied controller.
Same-mode requests return false before examining unused source parameters.
Entry resolves the configured asset/index/facing selection and initializes an
owned candidate before publication. Leaving stops the controller but retains
its binding/sequence. Reentry resolves and starts the configured selection again.
Health gates visibility, not the selected child tick; ended controllers do not
automatically restart. Other numeric modes explicitly refuse. Source failures
preserve the previous accepted state. No host pointers or QWidget services enter
these contracts; external callbacks must provide the selected owned source and
are not an arbitrary reentrancy protocol.

The next-twenty animation runner declares source/scope identities before native,
sanitized and original fixtures. Negative-index, missing-source and absent-result refusal are native-only controls;
no original-equivalence claim is made for those domains. Safe original descriptor, binding,
controller-state and phase-switch comparisons remain independent from actual
world drawing, complete configuration provenance, object lifetime and replacement.
