# Continuous guarded original source delivery

This increment replaces the accumulating diagnostic journal only in an explicit
`MNM_WORLD_CONTINUOUS` session. Legacy finite captures retain their protocol.
The pinned original traversal/caller checks and 800x600 RGB565 destination guard
remain mandatory. Native CPU history and GPU resources persist across queues;
original destination pixels never seed native rendering.

The fixed little-endian V1 channel has a128-byte header and two slots, each with
128-byte metadata,16MiB source capacity,197184-byte batch descriptor capacity and
960064-byte reply capacity. Total35869312 bytes. The source block is an owned
local V3 packet whose queue ordinal starts at1; the independent session generation
is64 bits. Local ordinal adaptation occurs only after checking that generation.

Slot ownership is `FREE -> WRITING -> REQUEST -> PROCESSING -> REPLY -> WRITING
-> RETURNED -> FREE`. The second WRITING state belongs exclusively to the original
producer while it appends actual closing records. Publication uses aligned
lock-free32-bit acquire/release operations;64-bit generation words are read only
under the state lease. Native role and original role are claimed once; nonzero
nonce, version, bounds, reserved fields, exact descriptor/source hashes, caller
entry/AX, destination extent and actual original return are checked. A slot cannot
be reused until native checks the unchanged prefix and actual checkpoint/return.
No supersession, dropped queue or implicit resume is permitted. Cancellation or
timeout is terminal; recovery requires a new file/session.

Original source definitions, observer counters and checkpoint ordinals reset per
block after the real original return, while surface identities and native history
persist. This resets observation bookkeeping, not original simulation state.
Source/record/descriptor overflow refuses the session. Diagnostics do not read or
write original destination oracle images, checkpoint image files, or disk replies.
The native final CPU/GPU comparison and required GPU readback for original
caller writeback remain active. Qt presents the retained GPU canvas directly.

`--continuous-queues N` explicitly bounds automated private-Xvfb experiments;
the channel target0 permits streaming until cancellation, without an accumulating
report (last1024 timing rows only). A bounded4096-queue original run and separate
native/original peer-death runs are the intended validation domain. They are
pending before execution. Correct final native CPU/GPU words do not establish
independent original raster equivalence beyond the separately compared32 prefix.
Menus, HUD writers outside the selected World body, uncaptured branches, combat,
hardware desktop/input/physical scanout and unlimited-duration reliability remain
separate boundaries. Normal full-path pacing must be measured, not inferred from
this transport implementation.

The initial64-queue probe refused at the first native request: concurrent startup
producers interleaved record emission (508 before507, then509), while the owned
source checksum and extent remained valid. Continuous mode now serializes
source-definition/reference emission and sequence/byte allocation as one bounded
critical section. This is an observation transport ordering fix; original
simulation/drawing state and legacy finite mode are unchanged. The failed
source-bound execution is preserved separately from fresh validation.
