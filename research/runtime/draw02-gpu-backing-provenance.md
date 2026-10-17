# DRAW-02 backing provenance and current-stream completion

This increment continues step 2's diagnostic source lifetime work. It does not
complete DRAW-02 or replace the original drawing body.

## Backing provenance

`tools/analyze-drawing-backing-provenance.py` replays retained original02 inputs,
requiring their exact historical producer/transition fingerprints and the entire
recomputed writer-trace observation to match. The original manifest is verified
before and after; historical source fingerprints are preserved.

There are 2,154 unlocated cache entries across 49 canvas identities. Of these,
2,144 entries across 46 identities have **no observed Lock return anywhere in the
captured window**. The other ten entries involve canvases 2, 21 and 53 whose first
observed Lock returns occur at later producer prefixes. Their eventual returned
address identities are 16, 116 and 115 respectively. Those are process-local
numerical address identities, not allocation owners. The wrapper interface
identities provide lifecycle context, not pixel pointers.

Confidence is high for this captured-event classification; physical backing
provenance remains unknown. An absent Lock observation does not mean a canvas has
no backing or is GPU-only. Equal producer prefixes cannot order a trace snapshot
against a transition. All-call instrumentation, alternate views, alias lifetime,
and direct engine storage allocation paths remain separate work.

Four classifier tests distinguish absent, earlier, equal-prefix and later Lock
returns. A CLI path normalization failure is retained separately; its original
inputs and manifests were valid and no engine execution occurred.

## Current-stream GL completion

The opt-in `MNM_DRAW_WINE_GPU_COMPLETION=1` diagnostic runs after the original
Wine opcode42 callback and before the worker return journal, portable context
retirement and selected lease release. The existing callback runs exactly once.
GL calls execute outside the journal critical section. Original callback floating
point state and LastError are restored by the enclosing worker wrapper.

The installed Wine11.16 PE32 `opengl32.dll` is pinned to SHA-256
`ea5c57f46e2bfe10e8ffdcbd1fba317c689b88b1bc25c3c7bb720b28ca7649d6`.
Checked public exports are `wglGetCurrentContext` at RVA `0xda210` and
`wglGetProcAddress` at `0xda900`, each with the first 16 bytes checked before use.
No private GL context layout is read or patched. Public sync entry points are
resolved afresh on the current context for every selected worker completion.
Null/sentinel/non-executable entry points refuse.

A null current context is explicitly **unwitnessed**, not a successful fence or
proof of no GPU work. Otherwise `glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE,0)`
is followed by `glClientWaitSync(sync,GL_SYNC_FLUSH_COMMANDS_BIT,1000000000)`.
Only `GL_ALREADY_SIGNALED` and `GL_CONDITION_SATISFIED` admit a witnessed result.
A created sync is deleted once. Missing functions, null fences, timeout, failed or
unknown wait results, and context changes retain a negative row and stop with
exit 99 before worker return publication. The one-second value bounds the wait
request; driver scheduling and failures are not a hard process watchdog.

A signaled fence means preceding commands in that current GL stream completed,
according to the [Khronos glFenceSync contract](https://raw.githubusercontent.com/KhronosGroup/OpenGL-Refpages/main/gl4/glFenceSync.xml).
The result classification follows
[glClientWaitSync](https://raw.githubusercontent.com/KhronosGroup/OpenGL-Refpages/main/gl4/glClientWaitSync.xml).
It does not identify which pixel backing the driver consumed, prove completion
on other contexts, show scanout, or establish hardware/native renderer parity.
The diagnostic adds synchronization and cannot establish unobtrusive timing.

## Wire format

`wine-gpu-completion.bin` uses `MNMWGC01`, a 64-byte header and 128-byte rows,
all little-endian u32. Header words 2..6 are version1, row128, build0x40209ca7,
record limit131072, wait timeout1000000000ns; words7..15 are zero.

Observation row words: 0 ordinal, 1 phase1, 2 worker, 3 command, 4 submission,
5 Windows thread, 6/7 start/end tick, 8 status, 9 process-local context identity,
10 wait result, 11 elapsed tick difference, 12 callable-function mask
(fence1/wait2/delete4), 13 fence created, 14 deleted, 15 same current context,
16/17 timeout low/high. Words18..31 are zero. No pointers cross the wire.
Status1 is no current context, 2 signaled, 3 unavailable procedures, 4 null fence,
5 timeout, 6 failed wait, 7 changed context, 8 unknown wait result.

Terminal phase4: word2 close reason1/2, word3 calls, words8/9/10
failed/unwitnessed/signaled counts. Other payload words are zero. Failed runs have
no normal terminal; the strict decoder refuses their admission. Completed cohorts
must bind exactly to retained worker/packet identities without pending commands.

## Executed results

[Owned fixture01](draw02-gpu-completion-fixture-01.json) passes 11 PE32 cases
and six decoder methods. Its GL results are injected, with the actual worker
wrapper preserving stack, x87 control and LastError and forwarding once. Six
negative cases stop before worker-return publication. The freestanding linker
failure is retained separately.

[Original01](draw02-gpu-completion-original-01.json) records **1,918 signaled
current-context fences, zero unwitnessed contexts**.
[The static identity review](draw02-gpu-completion-lease-review-01.json) joins
all 266 protected commands on 140 leases to exact worker/thread/command fence
witnesses. All 140 held source pairs are byte-identical. The run has 115 complete
advancing updates, normal original Quit, unchanged original sections and both
manifest checks. The schedule targets 120 updates; this capture does not reach
that target. It forces llvmpipe software rendering, so it proves observed GL
stream completion, not hardware GPU execution. The timed diagnostic rows total
208ms with a maximum32ms, including metadata overhead; this is not a controlled
performance comparison.

[Backing analysis02](draw02-backing-provenance-analysis-02.json) preserves the
historical original02 inputs and fingerprints every replay dependency. Analysis01
retains its earlier dependency contract; analysis02 supersedes it.

History 9a5f14b6..f56822f6 was reviewed against 24 committed file versions,
with exact before/after receipts and no gaps. Coverage census and current exact
review accounting accompany this increment.

## Remaining boundary

Original backing allocation and alternate mappings, all GPU contexts, source
identity inside driver operations, native replacement and complete DRAW-02 remain
open. Retained execution reports below supply concrete counts when available;
no historical evidence is refreshed to claim current validation.


## Step 2 GPU recorder teardown regression (2026-10-14)

The merged original-active pilot first selected the X11-only delivery library,
so the EGL interval could not close. Correcting that selection produced closed
API/EGL/producer, source-version, worker-packet and GPU journals, followed by
original exit99. Both failed acquisitions remain retained; closed journals
do not override the actual process exit. The second pilot recorded2,065 signaled
current-stream fences and142 restored source leases, without a recorded source
fault. These counts are diagnostic observations from a failed acquisition.

The owned postclose regression failed with exit216 because the GPU recorder
remained enabled after its file closed. The fix disables recording before
terminal emission/close and rechecks admission under the journal lock. GL calls
remain outside that lock. A callback finishing after a forced close cannot write
the closed file or fabricate a completed result; its started count makes strict
terminal decoding reject the incomplete cohort. Subsequent workers forward to
the original callback without additional GL calls, preserving x87 and LastError.
This fixes recorder retirement, not global worker or context quiescence.

[Fresh owned regression02](draw02-gpu-postclose-fixed-02.json) passes12 PE32 cases
and6 decoder methods. Six existing fence failures still stop before worker-return
publication. The postclose and forced-midwait cases use injected GL responses,
not hardware. [Acquisition/development negatives01](draw02-gpu-teardown-acquisition-negative-01.json)
preserve raw logs, failed reports and pre-fix source hashes. Causal attribution
of original exit99 remains pending a fresh original-active rerun.



The fresh [combined original32 pilot03](draw02-integrated-original32-03.json)
passes after the GPU teardown fix. The observed original child exits0 after
MainQuit dispatch; both2,927-file manifest checks and unchanged original sections
pass. The cohort has356 primary planes,2,302 consumed input calls,133 contiguous
advancing updates,2,108 signaled current-context fences and matching observed
source-version checks, and144 stable protected source pairs across272 completed
commands. The bounded source map retains964,557,152bytes and makes8,432 additional
public calls; diagnostic interference remains explicit. This fresh rerun supports
the teardown correction, without rewriting or excluding the earlier exit99.

Pilot32 does not request whole-update capture or producer/update binding. Its133
advancing counter windows therefore do not establish the required120 complete
source/frame/route cohort. Current-stream completion is software GL; alternate
mappings, direct unhooked writes, all contexts and source backing ownership remain
open. Semantic selection/tooltip/pause and native parity are unverified.
No status promotion or47/21closure. The8GiB reservation admitted1374.5187GiBworking
and108.3209GiBfree; the100GiB guard remained active. Further large capture sizing
requires a new storage preflight.


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
