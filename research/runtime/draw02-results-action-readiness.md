# Results action readiness

The prior App04 failure was status **2 (MNM_MENU_STALE)**, not unavailable
(status 3). Its immutable raw report and log remain unchanged. Results rows and
readiness advance the generation; a displayed ready state can change before the
original engine consumes its action. Confidence is high for this observed race
and the reviewed rejection guards, without asserting all possible transitions.

The production session retains one semantic Continue/Quit intent. Before wire
publication it reads current readiness and generation. Duplicates coalesce,
conflicting actions are refused, and native controls stay disabled while waiting.
An exact same-thread acknowledgement of the outstanding ID with STALE or
UNAVAILABLE proves no original Results callback ran: channel.h rejects stale or
unready requests before dispatch, and results.h's unavailable return precedes
0x475860. Only that explicit rejection may cause a new, increasing request ID
for the same observed Results screen/context/depth/offered action. Every wire ID
remains consumed once. An accepted callback is never replayed. Other menus retain
their existing rejection behavior and no-retry policy; the wire format is unchanged.

Observed departure or changed context/depth/thread/action, mismatched ACK,
retirement, unsupported/unknown rejection and timeout fail closed. One intent has
a ten-second total deadline, independently of request IDs; the existing two-second
engine lease still applies. The shared wire does not identify invisible leave-and-
reenter transitions between host observations, so this is an observed-visit scope,
not a universal engine ownership proof.

[Synthetic02](draw02-results-readiness-synthetic-02.json) passes thirteen
actual Qt/controller/session mapped-peer cases, including explicit stale/unavailable
rejections and fail-closed identity/deadline/inactive-session cases. The focused Result controller,
bridge, resident Quit and normal/HiDPI input checks pass 5/5. The initial sandbox
attempt could not start Xvfb for the two input checks; their ordinary escalated
rerun passes, without source changes.

[Live02](draw02-qt-results-readiness-02.json) passes Main → setup/map2 → Spells →
World → Results Continue → World → Results Quit → Quick → Main → normal Quit.
The test clicks immediately (zero settle delay), posts three rapid duplicate
button clicks per Results action and holds Escape 600ms on each World visit.
It observes two actual stale pre-callback rejections. Each original Continue/Quit
callback runs exactly once; request IDs increase to final ACK12 while the original
callback journal contains exactly ten actions on one thread. There are 402 native
command frames, 326 actual paints, no native fallback/recovery/error, normal
launcher status zero and retired menu/input channels. Source/binary hashes stay
stable, and all 2927 originals verify before/after. Raw run-7a9sxslm and staged
run-gi4gi566 remain retained. The earlier Live01 also passes with two real stale
rejections and once-only callbacks; it remains historical after the inactive-session
guard. Synthetic01 remains historical after adding the thirteenth case. This is one bounded two-visit software-GL journey,
not stress-duration/hardware or invisible-transition equivalence.

The 38e6539a commit range was reviewed against seventeen exact changed-file
receipts with no unresolved hash/history gaps. Old App04 is retained as a failure;
App05's 750ms settled harness pass remains historical after changed session/test
sources. The new policy and scenario provide fresh bounded operational evidence.
 Original CPU raster remains active;
this work does not promote full rendering takeover or pixel equivalence.

The 600ms interval names the test's scheduled Escape KeyRelease. Switching to
native Results can release mapped keys earlier through normal focus-loss handling.
Held-key auto-repeat is checked by the synthetic Qt case; this does not assert
600ms of uninterrupted original key-down or physical-keyboard timing.
