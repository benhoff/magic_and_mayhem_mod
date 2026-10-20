# Strict source ownership admission journal V1

`wine-source-ownership.bin` is a pointer-free little-endian wire contract. The
64-byte header contains magic `MNMSOW01`, version 1, record bytes 64, build
`0x40209ca7`, required capability mask 1023 and slot bound 2048; the remaining
nine DWORDs are zero. This journal attests admission, not mutation safety alone.

Each 16-DWORD record contains ordinal, phase, thread, capability mask, COPY
count, handoff count and ten reserved zero words. Phase 1 follows successful
installation; counts are zero. Phase 2 closes after all owned bindings restore;
COPY and handoff counts must match. Phase 3 refuses. The analyzer requires
exactly ready/closed, nonzero matching counts and joins the existing source
handoff, map, reservation, packet, worker and current-context fence journals.
Truncation, extra sessions, refusal, missing capability, forged counts or
missing physical proof refuse. No host pointers or C++ layouts cross the wire.

Mask bits 0..9 require submission; bounded map; source versions; GPU completion;
reservations; physical ownership; private handoff; packet plus worker; completed
physical hook installation; and all six public mutator entry patches.
