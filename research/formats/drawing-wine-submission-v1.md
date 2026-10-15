# Selected Wine submission metadata V1

`wine-submission.bin` is a bounded diagnostic side journal. It transports no host
pointers or source pixels. The 64-byte little-endian header contains magic
`MNMWIN01`, version1, row bytes192, game build0x40209ca7, maximum131072rows,
32thread slots and2048opaque identities; remaining words are zero.

Each192-byte row contains48u32words. Words0..9 are record ordinal, phase
(1entry/2return/4terminal), local call ID, method(1context blt/2swapchain present),
Win32 thread ID, enclosing observed surface API entry or0, API nesting depth,
GetTickCount stamp, original result and original LastError. Entry result/error
are zero; paired input fields must match exactly. Tick stamps do not establish
Linux-clock or EGL assignment.

Words10/11 are opaque destination/source texture IDs for blt, or swapchain ID/0
for present. Identity keys include object kind and retain addresses only inside
the process. No lifetime generation is implied; release/reuse tracking is open.
Words12/13 are destination/source subresource indices for blt, or interval/0 for
present. Word14 is flags. Word15 bits0/1 identify readable source/destination
rectangles; bits16/17 retain unreadable nonnull rectangle failures. Words16..19
and20..23 hold corresponding signed rectangle coordinates as rawu32. Word24 is
blt filter,25the nonnull effects flag,26the nonnull present window flag.
Words27..47 are reserved zero. Null rectangles preserve zero coordinates.

Terminal phase4 contains reason1detach/2explicit stop in word2,totalcalls in
word3,thread/tick in words4/7 and totalopaqueIDs in word10; other words zero.
All admitted entries require exactly one matching return and a closed terminal.
The strict decoder retains unbound submissions and keeps storage ownership,
submitted-source identity, EGL temporal assignment and complete journey false.

The opt-in hook replaces two validated import slots in the loaded pinned Wine
`ddraw.dll`, never the module file or immutable game input. Original cdecl calls
execute exactly once outside recorder locks. Binary file pins are checked before
launch; loaded import values and target export RVAs are checked before mutation.
