# Alternate canvas: live ownership search and borrowed composition

Chunk 4 now has an independently implemented, original-compared composition for
explicitly borrowed backing. Chunk 3 remains partial: the new real-game watches
find no alternate publication, and enabling the configuration setting produces
an original DirectDraw rectangle error before World dispatch. These results do
not identify an allocator, initial clear, source/base relationship or retirement.
No native allocator or live replacement is inferred from padding or configuration.

Addresses use the No-CD image SHA-256
`40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Original-consuming executions verify the immutable manifest before and after.
Findings have high confidence within the explicit execution boundaries below.

## 3. Real-game publication observation

`runtime/diagnostics/alternate_canvas_debug.c` starts only its owned child under
the Win32 debugger API. It installs DWORD write watchpoints on `6e1f68/6c` on
the initial thread and every subsequently reported created thread, before that
thread resumes. Two execution watches count `4fd438` dispatch and `58acf0`
shutdown. The debugger reads child memory; it never links WriteProcessMemory,
changes a game instruction, authors an alternate pointer or edits a CPU family.
SetThreadContext changes debug registers and the resume flag, retaining the
original general/control values. Idle samples briefly suspend/resume the main
thread to read its PC/stack; cadence and performance are not acceptance results.
All runs use bounded termination, not normal Quit or semantic cleanup evidence.

An authored PE32 positive control performs nonzero and zero stores to both slots
on the main and a created worker thread. Both threads' transient stores must be
observed: **four hits per slot, eight total**, plus two dispatch and two shutdown
execution hits. This establishes observation, not original game publication.

The real game uses the pinned immutable executable, with only the established
menu-navigation import added to its staged copy. Every original stored section
is checked unchanged before launch; menu callback/table instrumentation is an
explicit intervention. The working installation supplies assets, not its already
adapted executable. Initial installation files and provider DLLs are fingerprinted
and checked unchanged. Both plain and higher-precedence encrypted configuration
copies are edited and checked in the isolated stage. No shared game copy or
original artifact is modified. The serial-owned DirectPlay prefix is exclusively
locked, stopped and reused, including retirement between cohorts.

| Original setting | 640×480 | 800×600 |
| --- | --- | --- |
| `UseMallocedBackBuffer=FALSE` | Main → setup → selected map → Quick Battle; at least 40 dispatch calls; ordinary pitch 1,280 bytes | Same journey and dispatch bound; ordinary pitch 1,600 bytes |
| `UseMallocedBackBuffer=TRUE` | Original child-owned DirectDraw error dialog: “Rectangle provided was invalid.” | Same original error before World dispatch |

The enabled error observations read the actual configuration flag and `96/0`
padding, video choice and offscreen dimensions. The disabled observations also
verify the actual video/configuration state, not just authored files. Every
created thread is watched in each cohort. Both alternate pointers remain zero;
there are **zero write events** and no observed alternate World dispatch.
The actual `4fd438` comparand is checked zero during real dispatch. The debugger
reads modal text without dismissing the original error or replacing its result.
A stopped enabled case is a recorded negative; it is not a successful enabled
rendering session or proof of all possible failures.

This supports only the recorded startup/Quick Battle prefixes. It does not prove
that the alternate route is globally unreachable, that no hidden allocation
exists, or that another caller/build/external writer could not publish it. The
previous 54,067 unresolved computed writers and 2,948 indirect edges stay retained.
A compiled-out debug allocator is a possible explanation, not a confirmed finding.

## 4. Native composition with explicit borrowed storage

`reconstruction/rendering/alternate_canvas_dispatch.hpp` implements the selected
unchanged original `4fd438..4fd5af` window with separate storage binding and clip
state. The caller supplies independent ordinary, alternate-base and copy-source
spans. Their aliasing is explicit; the API supplies no allocation, initial clear,
generation publication, last-writer tracking or retirement policy.

The recovered ordering is consequential:

* Ordinary path: Lock; on success bind ordinary pixels/word pitch/height; World
  drawing sees the incoming clip; then set ordinary clip, draw overlays in order
  and Unlock. A failed Lock performs no World/overlay/Unlock work.
* Alternate path: bind the padded base and expand right/bottom clip; World draws
  before ordinary Lock. A failed Lock therefore retains the alternate World
  effects. On success rebind ordinary pixels and clip, execute the real copy
  schedule, draw overlays and Unlock. Unlock's HRESULT is ignored in this window.
* The copy snapshots four WORDs before each group of stores, preserving forward
  alias effects rather than substituting memmove. Copy-source stride is ordinary
  word pitch plus twice horizontal padding. Binding height, word stride and clip
  remain independent.

The new fixture executes the unchanged selected window, complete `57dc20/60`
setters and complete `594508` copy. Only named external Lock, World, overlay and
Unlock endpoints and the following continuation have checked private-image
adapters. World/overlay writes are independent authored controls, not the whole
original World renderer. The new header independently composes those endpoints;
no original pixel output is a native input.

**1,152 cases** cover both video dimensions, horizontal padding `0/4/96`, vertical
padding `0/3`, ordinary/alternate branches, successful/failed Lock, `0/1/3`
overlays, four explicit backing relationships and successful/failed Unlock.
The authored ordinary lease has a32-WORD pitch and9 rows, deliberately independent
of the video-derived alternate binding. Large backing canaries cover the padded
binding and untouched storage; these are not whole-game frames. They compare
every word in all three owned planes, complete ordered callback
observations and final binding/clip state: **1,170,874,368 owned WORDs** per full
execution. Source=base, offset source, independent source and source/destination
overlap are separate cases. Original window/setter/copy bodies remain unchanged.

Optimized original/native comparison and ASan/UBSan comparison pass in i386.
An independent native x86-64 ASan/UBSan run consumes no original image and matches
the case/state/pixel checksum. Eight invalid owned requests refuse before any
callback or state/pixel mutation. Those guards belong to the separate
`NP.alternate-canvas-dispatch-admission` policy, not original malformed-input
behavior. Callbacks must be nonthrowing with stable supplied inputs; arbitrary
reentrancy, callback mutation and asynchronous ownership are outside the domain.

## Evidence, reproduction and remaining dependency

Final SQLite-backed records:

* [Live observation](alternate-canvas-composition-results-20261024/live10.json).
* [Composition and sanitizers](alternate-canvas-composition-results-20261024/composition04.json).

All earlier reports/raw attempts remain retained. Live01 failed compilation;
live02 rejected an undeclared menu compiler input; live03 refused the adapted
installation executable. Live04–06 exposed the enabled-setting modal as a menu
timeout while the observer was developed. Live07 completed the four cohorts
before stronger asset/provider/video fingerprints. Live08 was interrupted by an
X11 window-destruction race in the optional focus helper; its owned clients were
retired and immutable inputs reverified, with an explicit interrupted record.
Live09 completed with the final observer before duplicate dependency declarations
were reconciled. Live10 renews the final prospective contract. The focus helper
tolerates disappearing windows on its private display.
Composition01 failed compilation; composition02 caught an unaligned fixture
store under UBSan, fixed with memcpy; composition03 passed before final policy
claims. No failed attempt supplies current validation credit.

Both runners require fresh output paths under the owned checkout's `working/`
and use `coverage_evidence.write_report` for immutable SQLite results. They pin
prospective claims and exact source/compiler inputs. The live runner requires
`--source-game`, `--prefix-owner-root` and `--wine-prefix`, plus `--output` and
`--report`; `--enabled-first` checks enabled cases before successful defaults.
Use the main retention guard with approximately 1 GiB reserved for the live
installation stage, or 0.08 GiB for the composition comparison. The private X
server uses authenticated TCP, Mesa software rendering and a Wine virtual desktop.

The borrowed composition is scoped/compared, with application integration and
live replacement `none`. Provenance remains partial, comparison/integration/
replacement `none`; live observations do not promote an unknown owner. Existing
canvas/retirement aggregates keep their broader boundaries.

To close the original ownership part of chunks 3–4 still requires a natural
nonzero publication or a defensible broader reachability analysis, followed by
allocator/clear/source relationship/writers/retirement recovery. The enabled
configuration failure also needs its own original call/rectangle attribution
before any intentional corrected native policy is proposed. Neither missing
contract is replaced by a fabricated allocator or a silently dismissed error.
