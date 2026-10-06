# NS19: static initial display for native creatures

The explicit native spawn initializer chooses the first opcode-0 bitmap record
in direction zero of the bound eight-sequence movement ANI profile. It stores
only the sequence/record pair in the existing owned display pose. It does not
start or advance an ANI player, execute controls/events, queue movement, or tick
the world. This is an intentional native static presentation policy; it does not
recover the original idle/action sequence choice or idle animation timing.

`MovementSession::spawn` accepts `initialDisplay=true` for an uncleaned creature
with a bound ANI and a continuous sample driver. The navigation interface selects
and validates the pose behind the application adapter boundary; native simulation
contains no ANI decoding or build-specific reconstruction types. A resource with
no selected bitmap, an incompatible policy, an unbound resource or an invalid
entity refuses before committing any slot or state. Existing calls default to
no initializer, preserving the independent reconstructed movement fixtures.

The sandbox command `spawn-terrain-ani MAP ANI BASE OUTPUT X Y Z` creates a
zero-tick idle checkpoint with terrain-aware positioning and a static body.
Snapshot v8 already owns this pair. Resource restoration validates the bound
record using the NS18 contract; loading old checkpoints does not invent a pose.
The pose stays static during admitted idle ticks, is retained through suppressed
planning, and yields to the active movement controller after the first order.
Stop then retains the current display rather than reverting to the spawn pose.
Released slot reuse explicitly initializes a new direction-zero pose.

Evidence is recorded in `native-spawn-display.json`. Synthetic tests exercise
zero-tick ownership, independent initial v8 layout/checksum, fresh-process idle
continuation, policy/resource refusal and rollback, initial-to-moving priority,
Stop and generation reuse. Installed Forest/Redcap actual-window tests start with
this zero-tick idle checkpoint, run idle ticks, save through the real modal
window, restore in a fresh process, pick the initial body and issue its first
move. They retain the mid-motion and stopped restart checks and independent
complete-frame sprite rasterization in all four diagnostic views. Installed
artifacts are read only, with original-manifest verification before and after.

Historical NS18 evidence remains unchanged and can become source-stale. Original
idle/action selection, animated idle, caller-selected initial facing, initial
poses for stationary blockers or non-ANI resources, other installed profiles,
full-game spawn admission and live replacement remain pending. No gameplay
balance or recovered comparison/replacement status is promoted.
