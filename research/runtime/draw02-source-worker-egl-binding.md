# Mapped sources, selected workers and post-EGL physical witnesses

This diagnostic policy joins an owned source snapshot to its observed Wine
submission, exact emitted packet and completed selected opcode42 worker. A
separate post-EGL physical witness is selected by the stable active worker
context before and after that exact EGL call, using process/call identity.
Windows and Unix thread identifiers remain separate. There is no nearest-frame
or ordinal pairing and no conversion between clock domains.

Each source also retains the exact enclosing original API caller, method,
HRESULT and last-error result from the raw-validated completed API journal.
These are caller and outcome observations, not inferred semantic input effects.

The join checks source and destination generations, subresources, boxes, flags
and observed lifetime prefixes. Unsupported filters, effects and copy geometry,
missing or changed context, unretained sources and every pixel mismatch remain
explicit. RGB565 copy regions use the existing independently tested expansion
and one-level RGB tolerance; alpha is outside the comparison scope.

An owned snapshot is immutable after acquisition. The original texture can
still be changed by an unobserved writer before consumption. Exact command
metadata and measured pixel agreement therefore do not prove original-source
atomicity, exclusive backing storage, every alias writer, GPU completion,
scanout, all presentation routes, semantic action outcomes or native parity.
Original rasterization remains active and these bytes are diagnostic inputs.


Whole-source image comparisons are recorded separately from the actual copy
region. When source and physical extents agree, all RGB pixels are compared;
agreement outside the command rectangle corroborates that image visually but
does not assign those pixels to that partial copy.

## Original Quick Battle and first campaign results — 2026-10-13

[Combined Quick Battle acquisition](draw02-source-map-worker-original-01.json)
retains 2,906 mapped sources (1,271,523,040 bytes), 2,906 exact emitted packets
and completed selected workers, 217 consecutive advancing complete updates,
457 API witnesses and 456 post-EGL witnesses. [Current full-image analysis](draw02-source-worker-egl-analysis-04.json)
binds every source to a completed worker and every retained EGL physical
witness to that source/command/worker/context. All 456 copy regions and all
456 whole 800×600 source images match (218,880,000 whole-image RGB pixels).
One final stable context has no physical witness after the explicit capture
close; it is retained as an unwitnessed context, not counted as a frame.

The original API mismatch is retained in [API analysis](draw02-source-worker-api-comparison-01.json).
Its first black witness finishes at Linux monotonic 310728827403545 ns.
Exact worker5/command5/submission5 is active for EGL call168, whose entry is
310728832188273 ns and return is310728833879990 ns. The later physical image
matches the same source SHA-256. Both timestamps use Linux CLOCK_MONOTONIC;
no Windows/Unix clock conversion or ordinal association is used. Confidence
is high for this observed earlier API capture/later delivery distinction.
The source remains immutable only in the owned snapshot; original aliases
and unobserved writers are outside this finding.

[Source/update interval analysis](draw02-source-worker-update-intervals-01.json)
retains 2,247 contained reads, one crossing and 658 outside readings out of
2,906. A crossing is not assigned to one complete update. [Matching timing
control](draw02-source-worker-cadence-comparison-01.json) measures 120 common
queued updates: median2.269684714× and p953.197318159× the counter-only control.
This combined observation is intrusive; neither identical randomized work nor
unobtrusive capture is established.

[First campaign acquisition](draw02-source-map-worker-campaign-original-01.json)
retains 1,177 owned sources (529,915,552 bytes), 34 complete advancing updates,
163 API witnesses and 162 post-EGL witnesses through actual original Mini,
Preferences, resume, abort confirmation and return to Main. [Its full-image
analysis](draw02-source-worker-egl-campaign-analysis-02.json) matches all162copy
regions and162whole images (77,760,000 RGB pixels), with no source or witness
refusals. The same startup API/EGL boundary distinction occurs. Its selected
pause/resume analysis initially refused an unobserved Quit acknowledgement;
[negative01](draw02-source-worker-campaign-pause-negative-01.json) preserves that
failure. The corrected analyzer requires exact request/screen/action/normal
exit/code0 evidence, while leaving the missing acknowledgement explicit.
[Negative02](draw02-source-worker-campaign-pause-negative-02.json) then correctly
refuses a982ms second Mini dwell against the unchanged1000ms threshold. The
capture now explicitly waits1500ms after observing the second Mini before
aborting; a fresh result is required to validate that change.

[Owned fixture04](draw02-source-worker-egl-fixture-04.json) passes ten independent
identity, generation, subresource, lifetime, whole-image/region, same-clock,
missing/ambiguous and mismatch/hash guards. Earlier fixture/analysis versions
keep their recorded fingerprints and do not validate the final model.

Step2 remains open for every original alias/writer and source atomicity,
semantic action-to-source outcomes, alternate display/movie routes and
unobtrusive reference acquisition. These are observed bounded diagnostic
associations and agreements. Steps3–7 and full47journey/21operation admission
remain separate outstanding work. Original rasterization is active.


## Source/worker/full-image acquisition and campaign timing — 2026-10-13

[Source/worker research](draw02-source-worker-egl-binding.md) records exact
mapped-source/submission/emitted-packet/completed-worker bindings and stable
active context around each retained EGL call, with original API callers/results.
[Quick Battle analysis04](draw02-source-worker-egl-analysis-04.json) matches456
copy regions and456whole800×600source images over217complete advancing updates.
The retained black startup API witness precedes the matching exact-command
EGL entry in the same Linux monotonic clock; the original mismatch is preserved.

[Final campaign original03](draw02-source-map-worker-campaign-original-03.json)
retains1424owned sources/654457216bytes,55complete advancing updates and194API/
193EGLwitnesses through Mini/Preferences/resume/abort/normalQuit. [Campaign
analysis04](draw02-source-worker-egl-campaign-analysis-04.json) binds every observed
source/worker and retained physical witness, matching193copy regions and193
whole images/92640000RGBpixels. Final Quick Battle and campaign references
therefore match649whole images/311520000RGBpixels. Whole-image agreement outside
a partial copy region is visual corroboration, not attribution of those pixels
to that command. One stable final EGL context per run has no physical witness
after explicit capture close and remains recorded separately.

[Pause/resume reanalysis04](draw02-source-worker-campaign-pause-analysis-04.json)
requires actual selected-field stability and advancement:Mini2514ms/3323ms,
Preferences2625ms,24resumed World samples with advancing counters. Exact normal
Quit exits0before its acknowledgement can be sampled; the acknowledgement
remains false. Owned guards reject failed/missing/mismatched normal-exit evidence.
The earlier unobserved-ack,982msMini and absent-counter-advancement refusals stay
in immutable negative01–03records. The capture now explicitly waits4000msafter
Preferences close and1500msat the observed secondMini; thresholds stay intact.

Ten source/worker/whole-image/delivery guards, four pause/resume/outcome guards
and eight existing source/physical guards pass. Changed capturer and offline
analyzer sources leave older coupled evidence historical; new records retain
fresh fingerprints without changing old hashes. Engine stages remain separate,
with no allpause/camera/native/replacement promotion. History4b772131..72a357b0
contains24exact reviewed source versions and zero missing receipts.

Step2continues:all original backing aliases/writers and source atomicity,
semantic input-to-source outcomes, remaining movie/display routes and unobtrusive
acquisition remain unproved. Existing timing control measures median2.27×/
p953.20×costfor120common queued updates; this is diagnostic acquisition.
Steps3–7and full47journey/21operation admission remain outstanding.


## Step 2 current combined source/delivery and lifetime reassessment (2026-10-14)

GPU recorder teardown is committed as3f4614f8 after the actual staged gate and
normal hook passed. The6453a4c9..3f4614f8 first-parent range has15 exact file
receipts and zero gaps; no retrospective execution claim.

[Current source/worker/EGL analysis01](draw02-integrated-source-egl-01.json)
revalidates raw original32pilot03 API/results, submission, source-map, packet,
worker/context and physical-peer records. All2,108 observed source submissions
bind to workers. All355 eligible post-EGL copy regions and355 whole source images
match, with170,400,000 whole-image RGB pixels and114,905,600 copied-region pixels.
There are zero source/witness/boundary refusals, pixel mismatches or API mismatches
before a matching EGL witness. Only355 eligible EGL images are compared; the
capture's356 API planes are not relabeled as one global presented-frame catalog.
Whole-image agreement outside a copied rectangle is corroboration, not attribution
of every pixel to that command. No all-context/hardware/scanout or native proof.

[Current backing analysis01](draw02-integrated-backing-01.json) replays unchanged
protected writer/lifecycle records and passes4classifier tests. The2,179 unlocated
cache entries cover49canvas identities:2,169 lack any observed Lock return during
the window, and10precede later Lock returns. The analyzer's report scope now names
the supplied completed capture rather than hardcoding the older original02. This
label correction changes no classifier semantics or historical evidence. Absence
of a Lock observation leaves backing unknown; it is not a GPU-only classification.

[Current lifetime analysis01](draw02-integrated-lifetimes-01.json) passes16lifetime
and8interval tests. It observes70successful query links,68alias groups,1,504CPU/DC
leases,117retirements and zero lease gaps/remaining leases. Of356 physical witness
intervals,100expose globally open leases, none expose known source/destination
leases, and122contain competing mutators.28intervals have known shared competitors
and113have unresolved listed competitors;19have both. None has unlisted competitors.
Query knowledge is temporal; shared sources alone do not prove actual writes, and
zero known lease exposure does not establish exclusive backing ownership.

Reassessment: current bounded reference association is stronger; source allocation
and alternate views, unhooked writers, all presentation routes,120whole-update
source/action/frame identity and independently verified semantics remain open.
Step2 remains active, all47journey/21operation closures and steps3–7 remain open.
No native-policy status promotion and no historical fingerprint refresh.

The main checkout's non-destructive storage inventory is retained at
`working/retention/draw02-original32-storage-review-20261014-01.json`:1378.0236GiB
working and107.6993GiBfree at its measured timestamp, with0.8595GiBreview candidates.
Those are candidates, not deletion permission. Registered worktrees, pins, referenced
evidence and current work remain preserved; no cleanup or archive executed. The
worktree1392GiBcap/100GiBfree reserve and disabled automatic archive remain in force.


## Step 2 combined whole120 acquisition and counter-gap reassessment (2026-10-14)

Source/delivery/lifetime analysis is committed as7398fd20. Its13first-parent
file versions have exact receipts with zero gaps; the normal staged gate/hook
passed. Retrospective history does not grant new execution validation.

[Combined whole120 attempt01](draw02-integrated-original120-01.json) times out
without a completion marker or normal Quit. The retained partial decoder finds
72closed World queue pairs/212,779records. The4,096source-definition cache limit
follows the existing inline-input fallback:3,579later primitives retain inline
inputs. It is not a silent truncation or demonstrated budget-caused failure.
The passive input timing hypothesis is assessed by a fresh separate retry;
original manifests verify before/after both runs. The failed raw/report remains
immutable and is not admitted as120whole-source acquisition.

[Retry02](draw02-integrated-original120-02.json) inserts30seconds of passive
World dwell after camera/click actions and before the first Escape. It leaves
game rules/balance unchanged and requests physical close only after exact source
completion. The12GiB guard admits1370.4064GiBworking/112.3936GiBfree.
The original child exits0 after MainQuit dispatch. The whole-source marker closes
120queues,476,150records,2,176checkpoints and123,912,664bytes with zero producer
failures, at the exact next accepted boundary122.589API physical planes and519
post-EGL witnesses are retained.2,649current-context fences/version checks and
231stable protected source pairs/447commands complete. Unknown cache layouts
remain6,091; no allocation/alternate-view/all-writer proof.

[Fresh source/worker/EGL replay01](draw02-integrated-original120-egl-01.json)
binds all2,649source submissions to observed workers and matches519eligible copy
regions and519whole source images, comprising249,120,000whole-image RGBpixels.
Every refusal/mismatch remains accounted for; the result has none. RGB agreement
outside the copied rectangle remains corroboration rather than attribution.
No expected pixels become native application inputs; this is reference analysis.

[Fresh counter/span replay01](draw02-original120-window-span-01.json) distinguishes
source queues from accepted-update windows. The120queues span121advancing windows
(sequence1..121). Sequence96advances counter95→96 with maintenance/animation/
scheduler/pacer events and zero observed calls to the two selected World queue
sites. The longest consecutive queue-bound accepted-sequence run is95; the
separate engine counter run is130. Missing selected calls do not prove no rendering
or complete alternate-route coverage. Their branch condition and all intervening
presentation/source/action/lifetime ownership remain open; no denominator or
required120window acceptance is waived.

Reassessment: the bounded whole120source milestone now executes on the combined
code, while the complete presented acquisition milestone stays open. Step2 remains
active; all47journey/21operation closures and steps3–7 remain open. Historical
source hashes/status axes are preserved; there is no engine/native parity,
hardware/all-context, scanout or semantic status promotion.

The user separately approved proposal729b818e to cold-relocate exactly9,475
RGB565/RGBA payloads (7.1873GiB) from failed runsxzz9u077/uu724zzl. Validation
passes without mutation. Attempts01/02refuse the existing archive lock with zero
files moved; bounded waiting attempt03 remains pending. Preserve journals/logs/
configuration/installations/source and latest successful captures. The proposal's
generic pin label was inaccurate; its explicit inventories correctly show neither
failed run has a root pin. The approved file list is unchanged. Archival is storage
provenance, not new engine validation; restore exact paths before future analysis.
