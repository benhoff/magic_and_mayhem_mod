# DIB Engine NE segment evidence

The archived Windows 98 DIB Engine used for diagnostics has SHA-256
`b7c0b676cd8c9960da4c53deae8de846d1251e9534ef4e4b2f05c988306dd2bd`.
It is downloaded input, outside the immutable original-media manifest; archive
provenance and publisher-authentication limitations remain recorded separately.

The bounded parser follows the MZ DWORD at file `0x3c` to the NE header. Relative
NE WORD fields used are: entry IP/segment `0x14/0x16`, segment count `0x1c`, segment
table `0x22`, and sector alignment shift `0x32`. Each segment-table entry contains
four WORDs: file sector, byte length (zero represents 65,536), flags and minimum
allocation. File offset is sector shifted by the alignment. Zero-sector and
iterated segments are explicitly refused; this parser is not a complete NE
loader or relocation implementation.

In this file, initialization starts at segment 2 offset `0x27`. Segment 2 begins
at file `0x1740`, is 529 bytes long and has file flags `0x1d70`. Offsets `0xce`
and `0xd7` contain `cd 31` (`INT 31h`). The executed module trace independently
maps the faulting CS to this segment in that run. Its selectors are runtime
observations, never cross-launch identities. AX is `0x000b` and `0x000c` at the
two faults, respectively: DPMI [Get Descriptor](https://www.delorie.com/djgpp/doc/dpmi/api/31000b.html)
and [Set Descriptor](https://www.delorie.com/djgpp/doc/dpmi/api/31000c.html).
The intervening instruction ORs descriptor byte 6 with `0x40`.

REC11 corrects the earlier terminal-cause attribution: these are observed
first-chance interrupt contexts, not proof that the descriptor services fail.
The continued trace dispatches both services before a later 32-bit access
violation. A read-only observer records the matching later PC/access and memory
state while returning CONTINUE_SEARCH, without handling the exception or changing
protection. This does not validate a compatibility fix or establish legacy
display-driver behavior. See
[the continued diagnosis](../runtime/open-recovery-closeout-20261021.md). Original DLL instruction bytes remain
unchanged. `tools/inspect-legacy-dibeng-fault.py` reproduces the file/trace mapping;
its extent and register-record tests use an independently authored NE fixture.
