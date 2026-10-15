# Diagnostic source lease through worker completion

`MNM_DRAW_SOURCE_WORKER_LEASE=1` extends an accepted read-only API source lease
until every opcode-42 packet emitted under that exact observed API entry has a
checked worker entry and return. It requires source-byte observation, read-only
source leases and exact packet binding. Other API entries do not contribute to
its completion count. The original API and worker bodies still execute.

The recorder arms one lease under its journal mutex before the original API
body. It retains up to 2048 command IDs, including commands that return before
the API returns. Emission binds the current observed API entry; worker entry
binds its worker call ID; worker return must match that ID exactly once. The
original owner waits for equality of emitted and completed counts, releasing
the mutex between checks. Zero emissions, unknown packets while the lease is
active, inconsistent IDs, duplicate returns and a five-second timeout refuse
with exit 99. The Scene lease separately refuses source writes and checks page
protection and identity before restoring it. This is an intentional diagnostic
cadence change, with no hardware-completion or scanout claim.

`api-source-worker-lease.bin` uses `MNMSWL01`, a 64-byte header and 64-byte rows.
Header words 2–7 are version 1, row size 64, build tag `0x40209ca7`, maximum
16384 rows, maximum 2048 commands per lease and timeout 5000 ms; words 8–15
are zero. Row words 0–10 are ordinal, phase, API entry, lease ID, Win32 thread,
GetTickCount, command ID, worker ID, emitted count, completed count and unknown
count; words 11–15 are zero. Phases are begin=1, emission=2, return=3,
release-ready=4 and terminal=5. Terminal word 2 is the stop reason; words 3–15
are zero. Local pointers never enter this journal.

The PE32 fixture passes delayed completion with a worker taking the same mutex,
early completion, unrelated API exclusion, missing emission, unknown packet
and bounded timeout. Its synthetic getter does not establish original source
page ownership. The original experiment and external native comparison are
separate evidence. All complete journey, operation and 120 advancing-window
requirements remain open; checked worker return does not cover other command
routes or asynchronous GPU completion.

Original run 01 closes 118 accepted leases and 222 command returns, with 658
refused lease attempts and no observed protected write. Exact crosschecks bind
all 222 commands to the same lease/API entry and corresponding Wine submission
and worker; 104 also have an observed active EGL swap. The independent native
replay matches 104 selected physical witnesses (233 excluded) and 208 CPU
entry/return planes, 99,840,000 pixels, with zero differences. CPU copies occur
outside the protected interval and therefore do not identify an immutable
source version within it. Capturing a snapshot under protection is the next
acquisition increment. These observations keep hardware completion, cadence
controls and the full semantic requirements pending.
