# COPY caller companion V3 and native cursor handoff

V3 extends the [V1/V2 companion](native-copy-callers-v1.md) with native cursor
frame mode 3. Magic is `MNMCCL03`, version 3, filename
`lock-capture/copy-callers-v3.bin`. The 128-byte header, 80-byte rows,
65,536-row capacity and 5,243,008-byte extent retain V2 field meanings.
V1/V2 decoders remain supported; V2 rejects mode 3 and mixed captures refuse.

Mode 3 is valid only for original wrapper RVAs `0x18c140`, `0x18c360`,
`0x18c4a0`, `0x18c8a0`, `0x18ca90`. Row words 16–19 retain source wrapper,
frame mode, verified normalized-prefix length and exact physical return marker.
Unknown frames remain unknown; mode 3 alone does not imply a direct game caller.

`NativeDrawCopyCallerV1(entry, original_return_slot, output, 32)` is a guest PE32
stdcall query using the existing 32-word handoff V1 scalar response. It is
enabled only by `MNM_NATIVE_CURSOR_COPY_CALLERS=1` at cursor installation; the
public launcher requires the separate COPY diagnostic route. It preserves
LastError, changes no rendering decision and acquires no additional lock.

The code guard checks the exact installed NativeDraw entry jump, replacement
target, original trampoline instructions and continuation. The two ordinary
rectangle wrappers have five-byte native pins but six-byte Scene pins. Both
exact forms are verified: Scene padding must be NOP, while an undisplaced
original sixth byte must match its pinned instruction. Only the reader's private
buffer receives the full normalized prefix. Executable memory remains untouched
by this query; complete-body FNV and staged original SHA256 checks still apply.

An active match requires the current provider owner/thread and nonzero recursive
depth, the top diagnostic copy scope, selected wrapper index, exact physical
return slot and marker, destination argument and protected source/destination
views. Both COM aliases in each view must resolve to the same retained provider
state; its immutable `lifetime_identity` must match the entry snapshot. This
checks backing lifetime, not mutable channel/history generation. The existing
provider guard and pins protect these reads. No public wrapper is reread after
the original service starts, and no lease is added, dropped or substituted.

The opt-in thunks call the same original thiscall service with the same operands
and RET cleanup. They publish the outgoing return slot and use unique assembly
return labels. The scope is cleared/restored after the original result state is
saved. Nested calls use distinct top scopes; an outer slot cannot match an inner
scope. Actual native-service or indirect callers retain their real entry return
PC and fail direct-original-E8 resolution. No earlier queue enqueuer is invented.

The render observer independently binds actual API operands and thread/SP to the
response before substituting the saved caller in its private read buffer. The
companion still requires an exact closed V2 ordinal/counter/module join, no active
misses/overflow, and zero unused tails. These are observed PE32 numeric identities,
not stable host pointers or addresses promised across launches.

The detailed positive-output list still admits only resolved original GPU callers
with complete independently compared command intervals. The existing strict
whole-COPY validator is reported separately. Neither success implies original
driver/ABI equivalence, global branch reachability or complete renderer acceptance.
