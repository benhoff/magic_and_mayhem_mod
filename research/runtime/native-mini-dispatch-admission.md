# Native Mini dispatch admission

The retained [navigation failure](source-prefix-preferences-witness-20261015.json)
ends in original Preferences without a native Save request. The storage driver
had already advanced to its waiting-for-Save stage. The exact cause remains
unclassified. No native renderer failure or original serialization defect is
established by that attempt.

The fresh [diagnostic](mini-dispatch-coarse-dwell-refusal-20261015.json), driver
`run-ax1lqax7` / observer `run-h5ju651u`, completes Save, Load, both original
display rebuilds, Main Load and normal Quit. All four native Mini requests were
accepted. The screenshot-before-click intervals are 181–446 ms. It does not
reproduce the Preferences detour. The overall test correctly refuses its
59749 ms Load hold against the requested 60000 ms: a default coarse Qt single-shot
may fire early. Sources remained unchanged and the private Wine prefix stopped.
This failed test remains a failed record, despite its completed navigation.

The driver now uses a precise timer and checks the monotonic minimum before
Load. It dispatches each action before optional screenshot work and records
native Mini admission explicitly. Rejected ownership, expired leases or changed
wire readiness/generations fail immediately; they cannot silently wait for Save.
Original Preferences while awaiting Save/Load/Quit is an immediate independent
failure. Original callback traces and fresh World checkpoints remain required;
host acceptance alone is not an engine ACK. There is no retry or replay policy.

The viewport is already focused by the driver. Mini ingress therefore sends
Escape without the former generic center click, which was a game command and
could overlap the menu boundary. The helper releases only input it still holds;
its successful Escape-only path sends no extra mouse release. Initiate's
recognized tutorial mouse actions and original Quit confirmation clicks remain.
Apprentice also recognizes the existing Hermes title and admits its calibrated
dismissal. A click batch sends no Escape. The next helper requires an advancing
native World source and a newly captured title observation before Escape; it
records image/crop SHA-256 values that the completed journey verifies.
The validators reject unclassified Mini mouse actions, missing Escape edges,
missing/reordered/refused native requests and capture-before-submission timing.
Input-channel snapshots are diagnostic observations, not consumption ACKs.

`--mini-capture-delay-ms 0..1000` explicitly blocks the GUI after submission and
before capture. It tests a bounded presentation delay, not unbounded lease
extension or a native input replacement. Cold ordinary and delayed runs are
recorded separately below.

The [Escape-only control](mini-dispatch-escape-only-refusal-20261015.json), driver
`run-x0y9_qb4` / observer `run-7ab27akv`, accepts Save before a 691 ms capture
including the deliberate 500 ms stall. Fresh native World sources advance after
Save, but all eight subsequent physical Escape attempts fail to reopen Mini.
Input remains active with repeated down/up generations; the cursor stays 0,0
because no mouse motion was sent. Its displayed post-Save state was not captured,
so focus, cursor and dialogue causes remain distinct hypotheses. No generic
click is reinstated as a focus action. The next control retains visible-title
crops and permits the existing calibrated Hermes dismissal only on a recognized
Hermes title; unknown text emits no mouse action.

The [dialogue-aware pilot](mini-dispatch-dialogue-pilot-20261015.json), driver
`run-03_0bpda` / observer `run-yxcq2w84`, completes 552 balanced Worlds,
Save/Load, both display rebuilds, Main Load and normal Quit. Its exact Load hold
is 60000 ms; all four native Mini requests precede the deliberately delayed
captures. Retained input 1 shows the original Hermes introduction after Save;
recognized dismissals precede later navigation. This demonstrates that dialogue
is a real boundary in the journey, not merely a Qt focus assignment. It does not
prove the exact cause of the earlier Escape-only or Preferences failures. The
pilot predates the final separate mouse/Escape batches, fresh-source guard and
image hashes, and retains those earlier source fingerprints.

[The incoming-history review](coverage/mini-dispatch-incoming-history-20261015.json)
checks committed `443a6d97..3981e981`:38 exact file transitions and 211 behavior
transitions. One missing intermediate resident-frame fingerprint is covered by a
retrospective pending receipt. This asserts neither a past gate pass nor fresh
engine execution.

The [final-source synthetic controls](mini-dispatch-controls-final-20261015.json)
pass six real Qt session/wire admission cases and the storage, campaign
Preferences/controller and Results regressions. The 16 storage and 21 campaign
portable checks include a real helper execution against mocked XTest/OCR:
Hermes produces only its classified click; a later unknown-title observation
produces only Escape. Reused World sources, combined click/Escape batches and
changed title images fail the validator. These are synthetic host/input controls,
not original comparisons.

The [final-source delayed journey](mini-dispatch-delayed-live-20261015.json),
driver `run-xr4ps23t` / observer `run-kdlt4228`, passes cold Apprentice Save/Load,
both original display rebuilds, Main Load and normal Quit. All 375 Worlds complete,
import and return; canvas 2 changes to 90. The full 60000 ms Load hold drains 688128
source records in 42 chunks with maximum 16384/chunk. Four native Mini requests
are accepted before their explicit 500 ms GUI stalls. Every input batch has its
advancing source witness, verified title image/crop hashes and separated mouse/
Escape actions. Original callback/return ordering and unchanged saved bytes are
checked independently. Sources/inputs remain stable, originals verify before/
after and the private Wine prefix stops. The declared 1 FPS functional floor is
not a performance claim. This journey exercises no marked NS_RECT copy-refusal
record and adds no fresh credit for that route.

The final-code [Initiate extension fails](mini-dispatch-initiate-queue-overflow-20261015.json),
driver `run-eiqqs1tn` / observer `run-58nugc3r`. It completes the four opening
commands, same-session Save/Load and first Quit, then refuses during Main Load
after both display rebuilds. The [raw queue witness](mini-dispatch-initiate-queue-witness-20261015.json)
confirms OVERFLOW reason 1, a full 1,048,576-byte mapped ring and a private queue
peak 33,542,596 of 33,554,432 bytes, with 469 FULL observations.707 Worlds have
balanced completion/import/return before failure; no changed World canvas has
yet completed. Source hashes remain stable, original manifests verify and the
private prefix stops. This is a distinct confirmed native command transport
failure, not a passing navigation result. The exact refused append packet and
producer/consumer scheduling cause are not captured. The harness fails before
its final input-stability check, so that check is not claimed for this negative.
No capacity change or
backpressure repair is included here. Complete Initiate rebuilt-map restore
therefore remains a known failure; the successful Apprentice journey cannot
substitute for it. This negative is kept out of positive milestone support.

The [ordinary final-source repeat](mini-dispatch-ordinary-live-20261015.json),
driver `run-914k_p2c` / observer `run-_h6xxfce`, also passes the
complete Apprentice Save/Load/display-rebuild/Main Load/normal-Quit journey.
Its 484 Worlds complete/import/return with no pending ownership,
renderer error or recovery. Native source generations and all input/image hashes
pass; four native Mini requests precede capture without an injected stall. The
5 FPS functional floor is met at 11.818 aggregate engine FPS;
this is software rendering, not hardware/performance qualification. Sources and
inputs remain stable, original manifests verify and the private prefix stops.

Current evidence therefore covers two final-code cold Apprentice journeys,
including the delayed 60-second boundary. The original Preferences detour's exact
cause remains unclassified, and the final-code Initiate rebuilt-map Load has the
explicit queue-overflow failure above. Other maps, arbitrary GUI stalls, complete
restored simulation and full-session original/native pixels remain separate.
Comparison and replacement statuses stay none.
