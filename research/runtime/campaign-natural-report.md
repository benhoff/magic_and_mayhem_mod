# Experimental natural campaign reports

The opt-in V20 campaign channel admits the original report singleton at
`006db967`, with outcome 0 (defeat) or 1 (victory), only in the recovered
Main / Realm / World / report stack, context 5 and layout 0. The callback,
button ID 21, owner, strings and parent identities are checked again before
calling original `004747a0`. The title and all 21 displayed strings are copied
verbatim; native code does not compute scores, rewards or campaign progress.
Earlier channel versions retain their Quit provenance and defeat-only bounds.

The Qt controller selects Victory or Defeat artwork when installed assets have
been loaded. The V20 acknowledgement must match the requested sequence and
engine thread. Original fade and World cleanup run before a fresh accepted
Realm state can restore native command ownership.

Headless validation is recorded as `UI62.campaign-natural-report`, in
`campaign-natural-report-20261012.json`: two unchanged original OK/fade cases,
13 guards, eight Qt cases, four companion regressions and PE32/Qt builds passed. The bounded private fixture retains original OK and fade
bodies, stubbing only audio cue and focus dependencies after checking expected
bytes. Its report/control data are authored fixtures. The Qt peer is synthetic;
its title/text and return tests do not validate installed artwork, scoring,
cleanup, a real win or saved progress. The reported drawing-lock timeout and
incomplete recovery snapshot remain a separate unresolved blocker for live
mission-end validation. No timeout increase or renderer fix is part of this
menu change.
