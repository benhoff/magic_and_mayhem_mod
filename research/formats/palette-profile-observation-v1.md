# Palette profile API observations v1

`MNM_PALETTE_OBSERVE=1` also observes actual `GetPrivateProfileStringA` calls
inside original palette builders4/5/7. Before installation the selected IAT slot
`005c507c` must equal the current resolved KERNEL32 export, obtained dynamically.
The local IAT pointer is changed only after this check; the original API executes
once with all six original arguments. Entry/return assembly preserves the actual
registers, flags, floating state and LastError. Calls outside the owning active
palette builder pass through without a transcript. The exact prior import is
restored at close; unexpected replacement causes permanent loss.

The separate `palette-profiles-0001.bin` uses a64-byte `MNMPPR01` header:
version1, size64, build40209ca7, maximum20,480 records, record bytes1,344,
maximum32MiB, then zeros. Each record has16little-endian DWORDs followed by five
256-byte, zero-padded NUL-terminated strings. Header fields are record bytes,
ordinal, parent builder-entry record ordinal from the main palette stream, mode,
owner thread, tick, World queue, preceding producer sequence, original API caller,
original EAX, requested buffer size256, original LastError, payload CRC32C and
three zeros. Strings are section, key, default, filename and actual returned
value. Inputs are copied before forwarding; output is copied only after return.
The observer does not choose defaults, parse values or feed expected colours.

The64-byte `MNMPPEND` terminal has version1, size64, records, preceding bytes,
global palette loss code, owner and zeros. Lost, unknown-parent, incomplete and
foreign records cannot supply a completed input transcript. Old v1/v2 main
palette captures without this file retain their absent platform-input boundary.

A native numerical replay may consume the returned text as a declared external
service input, reproducing the original missing-value and finite ASCII conversion
path. This proves palette construction for those actual inputs; it does not
recover or replace the Windows profile service, general malformed conversion,
exceptional arithmetic, caller ABI, visible output or cycling by itself.
