# Bounded native render command channel v1

Toolkit-designed native policy, 2026-10-05; no recovered original address/layout.
Schema: `protocols/schemas/render_commands-v1.json`. Bindings are generated.
Supported transport is a shared regular-file mapping on little-endian PE32 x86
and native x86-64, with aligned lock-free 32-bit acquire/release operations.

| Offset | Bytes | Owner | Meaning |
| --- | --- | --- | --- |
| 0 | 8 | creator | `MNMRDC01` |
| 8 | 4 | creator | version 1 |
| 12 | 4 | creator | exact whole-file size 67,108,928 |
| 16 | 4 | creator | nonzero launch/session token |
| 20 | 4 | producer | published immutable command bytes, 0..67,108,864 |
| 24 | 4 | producer | Ready=0, Writing=1, Ended=2, Failed=3 |
| 28 | 4 | producer | terminal reason: None=0, Overflow=1, Gap=2, Cancelled=3, Interrupted=4, Invalid=5 |
| 32 | 4 | consumer | cancellation flag, 0 or 1 |
| 36 | 28 | creator | zero reserved bytes |
| 64 | 67,108,864 | producer | append-only `MNMCMD01` byte stream |

The Qt creator uses an exclusive new file, initializes identity and zeros, and
keeps the mapping/file available until the Wine producer exits. One reader is
allowed by policy. The writer validates exact identity/size, nonzero token,
zero published/reason/cancel/reserved fields, and atomically claims Ready to
Writing. A second writer or a reused channel is refused. No host pointer or
C++ structure crosses the channel. Older frame peers ignore the separate
opt-in environment variable; this is not a frame-v1 layout extension.

The guarded owned-session producer appends each complete command header,
fields and native payload, then release-publishes the new byte count. Already
published bytes never change or wrap. The reader acquires state before byte
count, verifies identity and monotonically bounded publication, copies at most
1 MiB per poll, and frames commands across arbitrary byte fragments. It does
not scan for magic after a failure or replay a tail without its CREATE state.
There are no acknowledgments, waits for the reader, or overwrite-on-full policy.
Capacity exhaustion fails the session; producer-owned session limits can fail
earlier. This is bounded observation, not an indefinite gameplay ring.

END is first published as a command after owned DESTROY records; Ended state
is release-published afterward. Seeing Ended does not imply the reader has
drained all bytes. Success requires complete framing, all queued commands,
command END and channel Ended. EOF/process exit without that boundary fails.
A tracker gap or file/snapshot failure publishes Failed instead of claiming
completion. Normal DLL detach tries existing guarded session completion, then
marks an outstanding writer Interrupted. Abrupt death is detected by the
launcher's process lifecycle; there is no idle heartbeat or autonomous death
oracle in this channel. A consumer cancel is release-published separately;
subsequent producer append refuses it. No original rendering work is bypassed.

The reader drops incomplete/failed native presentation, aborts owned GPU
resources and requires a new file/consumer for restart. Previously displayed
frames cannot establish equivalence after a later gap. File truncation/replacement
while mapped and hostile peers are outside this cooperative transport contract.
