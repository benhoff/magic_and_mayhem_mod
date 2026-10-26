# Owned cursor lifetime in native launches

This closes the supported native graphics path's cursor ownership and deferred
copy lifetime. Recovered original callers remain documented independently in
[cursor clients](cursor-ownership-deferred-lifetime-20261025.md) and
[resource/setup recovery](cursor-lifetime-completion-20261025.md). The original
DirectDraw driver's undiscovered aliases are outside admission; the diagnostic
source gate continues to make no claim about them.

## Ownership and admission

`MnmDraw` owns every admitted surface backing. Private monotonic pins are
independent of public Surface1/2/3/4/IUnknown counts. Original wrapper retirement
can drain every public reference without destroying accepted queued or immediate
copies. A valid outstanding Lock/DC/storage lease also retains backing. Collection
runs only at the outer provider gate, after public references, private pins and
writers have all retired. Public interface headers remain bounded tombstones;
retired public pixel address ranges are quarantined, preventing later native
allocations from turning stale addresses into another generation. Physical
backing is reclaimed. Bounds are512 simultaneous pins,32768 lifetime headers,
768MiB cumulative public address reservations and the existing256MiB active
pixel/512-surface limits. Exhaustion refuses; it never reuses a generation.

Provider admission precedes the bridge copy owner, then tracker/history gates.
Native physical retirement identities reach the bridge after the last accepted
copy/writer, so its DELETE cannot precede a retained COPY. Full x86 FP/SIMD state
and LastError survive added gate bookkeeping. The fault handler follows an
immutable generation registry and skips retired headers; valid writer leases
keep their storage alive through an actual write and Unlock/ReleaseDC.

## Original cursor services and queue

`cursor_owner.h` validates18 complete non-relative entry anchors and the kernel
mutex imports against the pinned NoCD build. The48-byte wrapper snapshot includes
both marker+40 and source-key/retry+44. The original queue record remains44 bytes.
Immediate foreground/timer/background calls and all five copy services use
entry-generation wrapper views and private backing pins. Original create/release
services share provider admission. A public wrapper can be rebound while retained
work continues to designate its original backing.

The replacement128-slot queue copies rectangle values and retains both operands.
It preserves priority/slot order, bounds a drain by its starting sequence, keeps
busy writers pending, cancels invalid geometry before unbounded original retry,
and cancels retained cursor generations on disposal or terminal stop. Full queues
and late publication refuse without taking pins. In-flight work retains its pins
until return, including reentrant cancellation. Those are explicit native
ownership/refusal policies, separate from the original borrowed-pointer queue.

Private duplicates retain the inner and outer kernel mutex objects. The actual
game publishes outer before graphics creation; startup admits that valid mutex
while inner and queue are still empty. Later Create/Close interception tracks
new generations. Clients wait for outer outside the provider gate, then recheck
its generation before entering. Terminal RenderStop closes publication and
quiesces clients before lifecycle/tracker gates and final worker joins.

The resident scene observer already hooks seven shared helpers. Its private
`SceneCursorHookV1` handoff validates its installed jump, padding, saved complete
instructions and continuation. Native create/release retains scene observation.
Native-owned copies use original helper bodies and the MnmRender COM copy path;
they bypass the scene observer's wrapper-address copy model, which cannot name a
retired generation's private wrapper. Unknown or modified hook chains refuse.
The existing screen-services background helper implementation is reused unchanged.

`native_cursor_launch.py` selects this owner for native menu and OpenGL sessions,
clears inherited diagnostic selection and admits a fresh bounded lifetime journal.
Legacy diagnostic launches retain their separately documented scope.

## Graphics recovery

The private OpenGL compositor now checkpoints every canvas, atlas and saved
underlay into owned CPU images. Suspension requires a current checkpoint and
actually destroys its backend/context. Recovery stages all replacement allocations
before publishing them, preserves generation/queue identities and supports retry
after allocation failure. Stale checkpoints refuse. Four visible/clipped cursor
cycles, retained deferred generations, background refresh and lost-state shutdown
are checked against independent complete-frame pixels. This is a controlled
native context destruction/recreation result, separate from installed game
process restart and unspecified external-driver loss.

## Execution and limits

`recovery02` passes778 assertions and203 independently rebuilt complete frames in
each optimized and ASan/UBSan configuration,2400 producer attempts and eight actual
owner-destruction races. Every accepted concurrent publication drains once.

The final provider controls use real PE32 interfaces and kernel resources in
plain/protected modes:197641 assertions,1616 complete13x9 planes,16 ordered alias
pairs,eight writer cases,512pin saturation and1600 concurrent copies per mode.
The original-client controls execute the production installer and original cursor
bodies in four profiles covering both startup mutex orders and both backing modes.
Each checks1843262 assertions and six complete640x480 planes,131 accepted jobs,
three completed jobs and128 cancelled jobs,including old wrapper generations,
a busy writer,full queue and late refusal. All18 changed-byte anchors refuse.
Position/window services are authored in that isolated fixture; it does not run
the game entrypoint or the complete original setup resource internals.

The installed native-only campaign checks actual setup,selection,invalidation,
foreground,timer,both background clients and disposal,physical Qt/XTest movement,
Mini Cancel,Quit No/resume and Quit Yes/report/Main/exit. The native campaign
configuration selects immediate copies:zero queued jobs is an explicit observed
denominator,not deferred coverage. Deferred execution is supplied by the separate
original-machine-code/provider controls above. Complete world/driver pixel
parity,unknown external aliases and invalid CPU use after returning a lease are
not claims of this ownership policy. Historical failed builds and refused live
attempts remain immutable; later successful runs receive new evidence IDs.

Final combined-tree records are `CUR27.recovery02`,`CUR27.provider06`,
`CUR27.owner07`,`CUR27.controls02`,`CUR27.live09` and `CUR27.live10` under
[cursor results](cursor-live-ownership-results-20261025/). Live09 completes278
balanced Worlds and3363 cursor client calls;live10 verifies terminal producer
failure followed by public Hard restart and a normally completed fresh session.
The scope-bound lifetime acceptance flag checks installed hooks,quiescence and
resource retirement;it is not a whole original-frame pixel-equivalence flag.
Static pins01 retained an exploratory58ac40 candidate that was subsequently
rejected as a mid-instruction/non-entry. It is absent from production pins.

Local storage maintenance shared only byte-identical SHA256-named immutable
compressed fixtures between reviewed inactive checkouts. Raw/decoded hashes,
Git blob identities,modes and every logical path remain intact;no captures,
unique inputs or retention pins were removed. File-level SQLite plans have
verified second catalogs. This storage provenance grants no engine validation.

The behavior register keeps original comparison and original semantic replacement
at `none` for this authored native policy. The owning adapter is active in native
launches and has scoped live integration evidence;those fields do not assert
equivalence to unknown legacy-driver alias semantics.
