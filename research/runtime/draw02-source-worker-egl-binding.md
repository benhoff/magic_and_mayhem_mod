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
