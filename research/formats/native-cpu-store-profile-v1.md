# Accepted protected CPU store profile v1

Optional native instrumentation records only already accepted mode3 scalar stores handled by the protected-page exception handler. Direct native write-service operations are separate.
`MNM_NATIVE_CPU_STORE_PROFILE` names a new launch-owned output file; initialization
requires mode3. Original instruction bytes, destination validation, private-alias
writes, dirty-byte accounting and restored PE32 context remain independent.

Counters occupy a fixed64-site table in the native facade. Handler updates are
memory-only and use a nonwaiting guard. At most10,000,000 stores are retained.
After the exception handler retires, a bounded file is written and closed.
Early process termination can leave an empty file and is not a complete census.
All integers are little-endian unsigned32-bit words. No pointers are retained as
live references by a reader; instruction addresses describe this launch only.

The40-byte header is: eight magic bytes `MNMSTP01`, version1, header bytes40,
record bytes40, site capacity64, used sites, incomplete flags, counted stores,
and reserved zero. The exact file size is40 plus40 times used sites.

Each40-byte record contains instruction address, counted stores, counted bytes,
store width1/2/4, instruction length1..15, sixteen instruction bytes padded with
zeros, and a reserved zero word. Addresses are unique; instruction/width changes
at a recorded address mark the observation incomplete. Counted bytes equal
width times counted stores. Record counts sum to the header count.

Incomplete flags are1 for concurrent recording,2 for table exhaustion,4 for
invalid/inconsistent instruction metadata and8 for the store-count limit.
Nonzero flags describe a partial census and cannot establish a complete total.
The profiler neither changes an accepted instruction nor drops its engine write.
Its overhead is part of any instrumented timing. Original module identity and
address/RVA classification require the matching launch's module evidence.
