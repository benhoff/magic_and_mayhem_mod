# COPY upstream caller companions, versions 1 and 2

`MNM_NATIVE_COPY_CALLERS=1` adds `lock-capture/copy-callers-v1.bin` when the
all-COPY V2 route observer is active. This scalar diagnostic is not a rendering
input. The file has a 128-byte little-endian header and 65,536 reserved 64-byte
rows, bounded to 4,194,432 bytes. Unused rows are zero. Publication uses the
existing serialized COPY lease and exactly the published V2 route ordinals.

| Header word | Meaning |
| --- | --- |
| 0–1 | `MNMCCL01` |
| 2–5 | version 1, header bytes 128, row bytes 64, capacity 65,536 |
| 6–9 | published rows, overflow, attempted rows, mismatch/source-miss flags |
| 10–15 | Counts of row states 1 through 6 |
| 16 | Terminally closed |
| 17–19 | Observed original module base, extent, final companion V2 row count |
| 20–31 | Reserved zero |

| Row word | Meaning |
| --- | --- |
| 0 | Published V2 route ordinal |
| 1–2 | Original API return PC and module-relative RVA |
| 3–4 | Selected original wrapper entry RVA and fixed return-slot distance |
| 5–6 | Read upstream return PC and RVA, when a frame could be read |
| 7–8 | Read destination wrapper identity and actual API target identity |
| 9 | Resolution state |
| 10–12 | Frozen V2 preparation route, keyed flag, GPU-source flag |
| 13–14 | Frozen canonical source and destination identities |
| 15 | Reserved zero |

States are 1 resolved; 2 unsupported API site; 3 code/method mismatch;
4 unreadable/overflowing slot; 5 invalid or mismatched destination wrapper;
6 upstream return is not a direct E8 call to the selected wrapper. Unknown
sites have no fabricated frame. State counts do not imply branch reachability.
PE32 addresses are observed numeric identities, not host pointers or stable
identities across launches.

The header binds one-to-one to the strict, closed V2 route census. Overflow,
lost rows, active source misses, wrong ordinals/module, unclosed captures,
invalid states and nonzero reserved/unused tails refuse proof. Retired callbacks
excluded by V2 remain outside this companion's interval. They are not assigned
branch metadata or caller frames.

For a resolved row, the decoder also requires a reviewed API/layout pair and
the appropriate Blt/BltFast method. The acquisition producer independently
checks complete staged wrapper-body SHA256 values and the upstream E8 bytes.
It joins original static direct-call candidates and retains undiscovered owners.

The hook locates its own PE32 return-address slot using its compiler-maintained
frame. It then uses a pinned original wrapper layout; it never follows an
original EBP chain or searches arbitrary stack words. The runtime caches a
once-per-layout FNV code guard and checks API instruction bytes, readable slots,
module boundaries, the direct-call target and `destination_wrapper+8` against
the actual API target. All checks preserve GetLastError. Failures affect only
diagnostic classification, never copy admission or execution.

## Version 2: existing Scene detours

The current producer writes `copy-callers-v2.bin`, magic `MNMCCL02`, version 2,
with 80-byte rows and a total extent of 5,243,008 bytes. Header meanings and
row words 0–15 are unchanged. The reader retains version 1 support for historical
captures and rejects mixed versions in one capture directory.

| Appended word | Meaning |
| --- | --- |
| 16 | Observed source wrapper for a verified Scene handoff |
| 17 | Frame mode: 0 unresolved, 1 direct fixed slot, 2 Scene handoff |
| 18 | Verified detour prefix length, or zero without a verified detour |
| 19 | Physical return marker for a verified Scene handoff |

`SceneCopyCallerV1(entry, original_return_slot, output, 32)` is a guest-only
stdcall query returning 32 scalar PE32 words. It is enabled only by the caller
diagnostic. Its header is magic `0x3148434d`, version 1, size 128. Words 3–8
are original entry, displaced-prefix length, saved trampoline, installed detour,
physical return marker and original RET argument pop. Word 9 indicates an exact
active-frame match; words 10–14 then contain saved original caller, destination
wrapper, source wrapper, after-return SP and current thread. Word 15 and words
19–31 are zero; bytes 64–75 contain the saved original prefix, zero padded.
No match requires words 10–14 to be zero.

The query verifies the current entry jump, Scene-owned target and NOP padding,
saved instructions and exact trampoline continuation. A matched frame must have
the selected wrapper tag, published after-return SP, compatible owner, physical
Scene return marker and matching destination argument. The render observer
also checks the shared contract, original prefix, thread/SP and actual source
and destination Surface fields. It substitutes original bytes and saved frame
values only in its private read buffer. It changes neither the guest stack nor
the executable, operands, leases, drawing commands or pixels. Complete-body FNV
validation includes the unchanged remainder after that prefix normalization.

The native cursor adapter owns five further detours and calls saved original
trampolines with protected wrapper views. The Scene query deliberately refuses
these entries. Version 2 does not fabricate their upstream caller or interpret
a native service return address as an original game caller. An independently
validated cursor-adapter handoff is a separate implementation gap.

Successful metadata decoding does not imply that every frame resolves or that
every output was compared. Positive GPU output links require a resolved upstream
caller and a complete independently checked emitted interval. Missing intervals,
CPU per-caller pixel links and strict whole-output closure remain separate.

The separately versioned [V3 cursor extension](native-copy-callers-v3.md) is now
the current producer. V1/V2 retain the historical meanings and refusals above;
their recorded payloads and source hashes are unchanged.
